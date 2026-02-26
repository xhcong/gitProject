# UDP消息发送调试指南

## 问题: 程序启动发不出UDP消息

### ✅ 问题原因与解决

**根本原因:**
- `sendData()` 是槽函数，需要在正确的线程上下文中执行
- 直接调用槽函数不会在工作线程中执行

**已修复:**
1. 在 UDPWorker 中添加了 `requestSendData` 信号
2. 线程启动时自动连接信号到槽
3. 发送时使用信号而不是直接调用

## 修改内容

### 1. udp_worker.h
```cpp
signals:
    void requestSendData(const QHostAddress& address, quint16 port, const QByteArray& data);
```

### 2. udp_worker.cpp - run()函数
```cpp
// 在run()中添加这一行:
connect(this, &UDPWorker::requestSendData, this, &UDPWorker::sendData,
        Qt::QueuedConnection);
```

### 3. udp_interface.cpp
```cpp
// sendBytes() 和 sendBytesByPort() 现在使用:
emit worker->requestSendData(address, port, data);
// 而不是:
// worker->sendData(address, port, data);
```

### 4. meta_manage.cpp
```cpp
// 改进了日志输出，便于调试
Logger::instance().debug(QString("Sending to NEC (%1:%2): %3")
                        .arg(necAddress.toString())
                        .arg(config.network.nec_port)
                        .arg(message));
```

## 验证修复

### 检查日志输出

编译后运行程序，应该看到：

```
UDP Interface initialized
UDP socket bound to 127.0.0.1:6001 (NEC)
UDP socket bound to 127.0.0.1:7000 (Interface)
Sending to NEC (127.0.0.1:9000): NENetRunSuccess
Received NEC data from 127.0.0.1:xxxxx: NECRunSuccess
NEC connection established
```

### 使用nc工具验证接收

1. 启动程序（应该发送心跳）
2. 监听NEC端口查看是否收到心跳：

```bash
# Linux/Mac
nc -u -l 127.0.0.1 9000

# Windows (需要 Netcat 或 socat)
nc -u -l -p 9000
```

3. 应该会收到：
```
NENetRunSuccess
```

## 信号槽调用流程

```
MetaManage::sendMessageToNEC()
    ↓
UDPInterface::sendBytesByPort()
    ↓
emit UDPWorker::requestSendData(address, port, data)  ← 发出信号
    ↓ (跨线程转移到UDPWorker线程)
UDPWorker::sendData() ← 在UDPWorker线程中执行
    ↓
QUdpSocket::writeDatagram()
    ↓
消息发送到网络
```

这个流程确保 sendData() 在正确的线程中执行！

## 如果仍然发送失败

### 检查清单

- [ ] 查看日志中是否显示 "UDP socket bound to 127.0.0.1:6001"
- [ ] 查看日志中是否显示 "Sending to NEC"
- [ ] 查看是否有 "UDP worker on port %1 is not bound" 的警告
- [ ] 确认配置中的IP和端口是否正确
- [ ] 使用 `netstat -an | grep 6001` 检查端口是否正确绑定

### 调试步骤

1. **添加更多日志** (可选)

   在 udp_worker.cpp 的 sendData() 中添加：
   ```cpp
   void UDPWorker::sendData(const QHostAddress& address, quint16 port, const QByteArray& data)
   {
       if (!m_socket) {
           qWarning() << "Socket not ready!";
           return;
       }

       qint64 sentBytes = m_socket->writeDatagram(data, address, port);
       qWarning() << "Sent" << sentBytes << "bytes to" << address.toString() << port;

       if (sentBytes == -1) {
           qWarning() << "Send error:" << m_socket->errorString();
       }
   }
   ```

2. **验证UDPWorker是否启动**

   在 udp_interface.cpp 的 bindToPort() 中添加：
   ```cpp
   worker->start();
   Logger::instance().info(QString("UDP worker thread started for port %1").arg(port));
   ```

3. **监控信号槽连接**

   使用Qt的信号/槽调试：
   ```bash
   QT_DEBUG_COMPONENTS=1 ./NENET_Qt5
   ```

## 常见问题

### Q: 看到 "UDP worker on port %1 is not bound"

**A: 可能原因:**
1. UDPWorker线程还没启动
2. socket绑定失败
3. 端口已被占用

**解决:**
- 增加延迟: 在发送心跳前等待100ms
- 检查防火墙是否阻止了该端口
- 确认没有其他程序占用该端口

### Q: 看到 "No bound UDP socket available"

**A: 可能原因:**
1. UDPInterface没有初始化
2. 没有成功绑定任何端口

**解决:**
- 检查日志中是否有绑定成功的消息
- 确认配置正确加载

### Q: 能接收消息但发送失败

**A: 可能原因:**
1. 接收线程正常但发送有问题
2. 目标IP/端口配置错误

**解决:**
- 检查目标地址和端口是否正确
- 使用 `nc` 工具验证网络连通性

## 重新编译

修改后需要重新编译：

```bash
cd NENET_Qt5
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build . --parallel 4
```

---

**修复完成后应该能正常发送UDP消息！**
