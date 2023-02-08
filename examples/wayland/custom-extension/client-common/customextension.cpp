// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "customextension.h"
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>
#include <QtGui/QPlatformSurfaceEvent>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QDebug>

QT_BEGIN_NAMESPACE

CustomExtension::CustomExtension()
    : QWaylandClientExtensionTemplate(/* Supported protocol version */ 1 )
{
    connect(this, &CustomExtension::activeChanged, this, &CustomExtension::handleExtensionActive);
}


static inline struct ::wl_surface *getWlSurface(QWindow *window)
{
    void *surf = QGuiApplication::platformNativeInterface()->nativeResourceForWindow("surface", window);
    return static_cast<struct ::wl_surface *>(surf);
}

QWindow *CustomExtension::windowForSurface(struct ::wl_surface *surface)
{
    for (QWindow *w : std::as_const(m_windows)) {
        if (getWlSurface(w) == surface)
            return w;
    }
    return nullptr;
}

bool CustomExtension::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        QWindow *window = qobject_cast<QWindow*>(object);
        Q_ASSERT(window);
        if (window->isExposed()) {
            window->removeEventFilter(this);
            QtWayland::qt_example_extension::register_surface(getWlSurface(window));
        }
    }
    return false;
}

void CustomExtension::sendWindowRegistration(QWindow *window)
{
    if (window->handle())
        QtWayland::qt_example_extension::register_surface(getWlSurface(window));
    else
        window->installEventFilter(this); // register when created
}

void CustomExtension::registerWindow(QWindow *window)
{
    m_windows << window;
    if (isActive()) {
        m_activated = true;
        sendWindowRegistration(window);
    }
}

CustomExtensionObject *CustomExtension::createCustomObject(const QString &color, const QString &text)
{
    auto *obj = create_local_object(color, text);
    return new CustomExtensionObject(obj, text);
}

//! [sendBounce]
void CustomExtension::sendBounce(QWindow *window, uint ms)
{
    QtWayland::qt_example_extension::bounce(getWlSurface(window), ms);
}
//! [sendBounce]

void CustomExtension::sendSpin(QWindow *window, uint ms)
{
    QtWayland::qt_example_extension::spin(getWlSurface(window), ms);
}

void CustomExtension::handleExtensionActive()
{
    if (isActive() && !m_activated) {
        m_activated = true;
        for (QWindow *w : std::as_const(m_windows))
             sendWindowRegistration(w);
    }
}

void CustomExtension::example_extension_close(wl_surface *surface)
{
    QWindow *w = windowForSurface(surface);
    if (w)
        w->close();
}

void CustomExtension::example_extension_set_font_size(wl_surface *surface, uint32_t pixel_size)
{
    emit fontSize(windowForSurface(surface), pixel_size);
}

void CustomExtension::example_extension_set_window_decoration(uint32_t state)
{
    bool shown = state;
    for (QWindow *w : std::as_const(m_windows)) {
        Qt::WindowFlags f = w->flags();
        if (shown)
            f &= ~Qt::FramelessWindowHint;
        else
            f |= Qt::FramelessWindowHint;
        w->setFlags(f);
    }
}

CustomExtensionObject::CustomExtensionObject(struct ::qt_example_local_object *wl_object, const QString &text)
    : QWaylandClientExtensionTemplate<CustomExtensionObject>(1)
    , QtWayland::qt_example_local_object(wl_object)
    , m_text(text)
{

}

void CustomExtensionObject::example_local_object_clicked()
{
    qDebug() << "Object clicked:" << m_text;
    emit clicked();
}

void CustomExtensionObject::setText(const QString &text)
{
    m_text = text;
    set_text(text);
}

QT_END_NAMESPACE
