#ifndef QWAYLANDEVENTTHREAD_H
#define QWAYLANDEVENTTHREAD_H

#include <QObject>
#include <QMutex>
#include <wayland-client.h>

#include <QtWaylandClient/qwaylandclientexport.h>

QT_BEGIN_NAMESPACE

class QSocketNotifier;

class Q_WAYLAND_CLIENT_EXPORT QWaylandEventThread : public QObject
{
    Q_OBJECT
public:
    explicit QWaylandEventThread(QObject *parent = 0);
    ~QWaylandEventThread();

    void displayConnect();

    wl_display *display() const;

private slots:
    void readWaylandEvents();

    void waylandDisplayConnect();

signals:
    void newEventsRead();

private:

    struct wl_display *m_display;
    int m_fileDescriptor;

    QSocketNotifier *m_readNotifier;

    QMutex *m_displayLock;

};

QT_END_NAMESPACE

#endif // QWAYLANDEVENTTHREAD_H
