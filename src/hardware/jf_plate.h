#pragma once

#include <QObject>

class JFPlate : public QObject
{
    Q_OBJECT

public:
    explicit JFPlate(QObject* parent = nullptr);
    ~JFPlate() override;

    bool initialize();
    void cleanup();

private:
    bool m_initialized{false};
};
