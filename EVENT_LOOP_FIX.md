# UDP信号槽未触发 - 事件循环修复

## 🔴 问题
emit信号后，`sendData()`不被触发

## 🔍 根本原因
**手动QEventLoop::processEvents()无法正确处理Qt::QueuedConnection信号**

旧代码问题：
```cpp
QEventLoop eventLoop;
while (m_running) {
    eventLoop.processEvents();  // ❌ 不够用
}
```

这不是一个真正的事件循环，它只是处理一些待处理事件，无法正确处理跨线程的Queued连接。

## ✅ 解决方案

### 核心改动：改用 `exec()`

```cpp
// ❌ 修改前
QEventLoop eventLoop;
while (m_running) {
    eventLoop.processEvents();
}

// ✅ 修改后
this->exec();  // 启动真正的事件循环
```

---

## 📝 修改清单（6个改动）

### 1️⃣ `src/network/udp_worker.cpp` - run()函数

**核心改动：**
```cpp
// ❌ 旧方式
QEventLoop eventLoop;
while (m_running) {
    eventLoop.processEvents();
}

// ✅ 新方式
this->exec();  // 这是QThread的标准用法
```

**信号槽连接改动：**
```cpp
// 改为DirectConnection，确保立即执行
connect(this, &UDPWorker::requestSendData, this, &UDPWorker::sendData,
        Qt::DirectConnection);  // ← 改这里

// 检查连接是否成功
bool connected = connect(...);
if (!connected) {
    qWarning() << "Failed to connect signal!";
}
```

**为什么？**
- `exec()` 是QThread推荐的事件循环启动方式
- 它创建一个真正的事件循环，能处理所有信号和事件
- Qt::DirectConnection确保信号立即在线程中执行（不需要排队）

### 2️⃣ `src/network/udp_worker.cpp` - stop()函数

```cpp
// ✅ 改为调用quit()
void UDPWorker::stop()
{
    m_running = false;
    quit();  // 让exec()退出
}
```

### 3️⃣ `src/network/udp_worker.cpp` - sendData()函数

**添加调试输出：**
```cpp
void UDPWorker::sendData(const QHostAddress& address, quint16 port, const QByteArray& data)
{
    qDebug() << "=== sendData() called ===";  // ← 验证槽被调用
    qDebug() << "Sending" << data.size() << "bytes to" << address.toString() << ":" << port;

    if (!m_socket) {
        qWarning() << "ERROR: Socket not initialized";
        return;
    }

    qint64 sentBytes = m_socket->writeDatagram(data, address, port);

    if (sentBytes == -1) {
        qWarning() << "ERROR: Send failed:" << m_socket->errorString();
    } else {
        qDebug() << "SUCCESS: Sent" << sentBytes << "bytes";
    }
}
```

### 4️⃣ `src/network/udp_interface.cpp` - bindToPort()

```cpp
bool UDPInterface::bindToPort(const QString& ip, quint16 port)
{
    qDebug() << "Creating and starting UDPWorker on port" << port;

    UDPWorker* worker = new UDPWorker(ip, port, this);
    // ... 连接信号 ...
    worker->start();  // 启动线程，这会调用run()

    m_workers[port] = worker;
    qDebug() << "Worker started. Total workers:" << m_workers.size();
    return true;
}
```

### 5️⃣ `src/network/udp_interface.cpp` - sendBytesByPort()

```cpp
void UDPInterface::sendBytesByPort(quint16 sourcePort, const QHostAddress& address, quint16 port, const QByteArray& data)
{
    qDebug() << "Sending to port" << sourcePort;

    if (!m_workers.contains(sourcePort)) {
        qDebug() << "ERROR: No worker on port" << sourcePort;
        qDebug() << "Available ports:" << m_workers.keys();
        return;
    }

    UDPWorker* worker = m_workers[sourcePort];
    qDebug() << "Emitting signal...";

    emit worker->requestSendData(address, port, data);  // ← 发出信号
}
```

### 6️⃣ `src/core/meta_manage.cpp` - 延迟调整

```cpp
// 给workers充足的时间启动
QThread::msleep(300);  // 增加到300ms
sendMessageToNEC("NENetRunSuccess");
```

---

## 🔄 工作流程

**修改前 (失败)：**
```
emit requestSendData()
    ↓
QEventLoop::processEvents()  ❌ 无法处理queued信号
    ↓
slot不执行
```

**修改后 (成功)：**
```
emit requestSendData()
    ↓
Qt::DirectConnection  ✅ 立即执行
    ↓
sendData()在工作线程立即执行
    ↓
writeDatagram()
    ↓
✅ 消息发送
```

---

## 🚀 重新编译

```bash
cd NENET_Qt5
rm -rf build && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel 4
```

---

## ✨ 验证方法

编译后运行，查看Debug输出：

```
✅ === bindToPort() called ===
✅ Created UDPWorker, pointer: 0x...
✅ Started UDPWorker thread
✅ === sendBytesByPort() called ===
✅ Emitting signal...
✅ === sendData() called ===  ← 这证明槽被调用了
✅ Sending XX bytes to 127.0.0.1:9000
✅ SUCCESS: Sent XX bytes
```

---

## 📊 改动摘要

| 方面 | 修改 |
|------|------|
| **事件循环** | processEvents() → exec() |
| **信号连接** | QueuedConnection → DirectConnection |
| **停止线程** | m_running=false → quit() |
| **调试输出** | ✅ 大量qDebug()语句 |
| **错误处理** | ✅ 检查connect()返回值 |
| **启动延迟** | 200ms → 300ms |

---

## 🎯 为什么这样修复

1. **exec()是标准做法** - 这是所有QThread子类的推荐用法
2. **DirectConnection立即执行** - 不需要事件队列，直接调用
3. **详细调试** - qDebug()语句帮助验证每一步

---

## 🔗 关键知识点

### Qt::DirectConnection vs Qt::QueuedConnection

| 类型 | 说明 | 适用场景 |
|------|------|---------|
| **DirectConnection** | 信号发出时立即调用槽 | 线程内调用 |
| **QueuedConnection** | 信号转为消息，放入事件队列 | 跨线程调用 |

对于工作线程内的槽，DirectConnection更高效！

---

**现在信号应该能正常触发了！** 🎉
