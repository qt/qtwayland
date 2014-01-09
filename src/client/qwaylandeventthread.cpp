#include "qwaylandeventthread_p.h"
#include <QtCore/QSocketNotifier>
#include <QCoreApplication>

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

QT_BEGIN_NAMESPACE

QWaylandEventThread::QWaylandEventThread(QObject *parent)
    : QObject(parent)
    , m_display(0)
    , m_fileDescriptor(-1)
    , m_readNotifier(0)
    , m_displayLock(new QMutex)
{
}

QWaylandEventThread::~QWaylandEventThread()
{
    delete m_displayLock;
    wl_display_disconnect(m_display);
}

void QWaylandEventThread::displayConnect()
{
    m_displayLock->lock();
    QMetaObject::invokeMethod(this, "waylandDisplayConnect", Qt::QueuedConnection);
}

void QWaylandEventThread::readWaylandEvents()
{
    if (wl_display_dispatch(m_display) == -1 && errno == EPIPE) {
        qWarning("The Wayland connection broke. Did the Wayland compositor die?");
        ::exit(1);
    }
    emit newEventsRead();
}

void QWaylandEventThread::waylandDisplayConnect()
{
    m_display = wl_display_connect(NULL);
    if (m_display == NULL) {
        qErrnoWarning(errno, "Failed to create display");
        ::exit(1);
    }
    m_displayLock->unlock();

    m_fileDescriptor = wl_display_get_fd(m_display);

    m_readNotifier = new QSocketNotifier(m_fileDescriptor, QSocketNotifier::Read, this);
    connect(m_readNotifier, SIGNAL(activated(int)), this, SLOT(readWaylandEvents()));
}

wl_display *QWaylandEventThread::display() const
{
    QMutexLocker displayLock(m_displayLock);
    return m_display;
}

QT_END_NAMESPACE
