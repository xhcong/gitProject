#ifndef QJ_CUSTOM_H
#define QJ_CUSTOM_H

#include <QString>

class QJCustom
{
public:
    static QJCustom& instance();

    bool initialize();
    void cleanup();

private:
    QJCustom();
    ~QJCustom();

    QJCustom(const QJCustom&) = delete;
    QJCustom& operator=(const QJCustom&) = delete;

    bool m_initialized = false;
};

#endif // QJ_CUSTOM_H
