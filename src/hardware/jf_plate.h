#ifndef JF_PLATE_H
#define JF_PLATE_H

#include <QString>
#include <QObject>

/**
 * @brief Jiangfei acquisition board interface
 */
class JFPlate : public QObject
{
    Q_OBJECT

public:
    JFPlate(QObject* parent = nullptr);
    ~JFPlate() override;

    bool initialize();
    void cleanup();

private:
    bool m_initialized = false;
};

#endif // JF_PLATE_H
