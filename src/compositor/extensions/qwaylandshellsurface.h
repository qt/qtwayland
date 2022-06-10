// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

#include <QtWaylandCompositor/qtwaylandqmlinclude.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>

QT_BEGIN_NAMESPACE

class QWaylandQuickShellIntegration;
class QWaylandQuickShellSurfaceItem;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShellSurface : public QWaylandCompositorExtension
{
    Q_OBJECT
    Q_PROPERTY(Qt::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    QML_NAMED_ELEMENT(ShellSurface)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
#if QT_CONFIG(wayland_compositor_quick)
    virtual QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) = 0;
#endif
    QWaylandShellSurface(QWaylandObject *waylandObject) : QWaylandCompositorExtension(waylandObject) {}
    virtual Qt::WindowType windowType() const { return Qt::WindowType::Window; }

protected:
    QWaylandShellSurface(QWaylandCompositorExtensionPrivate &dd) : QWaylandCompositorExtension(dd){}
    QWaylandShellSurface(QWaylandObject *container, QWaylandCompositorExtensionPrivate &dd) : QWaylandCompositorExtension(container, dd) {}

Q_SIGNALS:
    void windowTypeChanged();
};

template <typename T>
class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShellSurfaceTemplate : public QWaylandShellSurface
{
public:
    QWaylandShellSurfaceTemplate(QWaylandObject *container = nullptr)
        : QWaylandShellSurface(container)
    { }

    const struct wl_interface *extensionInterface() const override
    {
        return T::interface();
    }

    static T *findIn(QWaylandObject *container)
    {
        if (!container) return nullptr;
        return qobject_cast<T *>(container->extension(T::interfaceName()));
    }

protected:
    QWaylandShellSurfaceTemplate(QWaylandCompositorExtensionPrivate &dd)
        : QWaylandShellSurface(dd)
    { }

    QWaylandShellSurfaceTemplate(QWaylandObject *container, QWaylandCompositorExtensionPrivate &dd)
        : QWaylandShellSurface(container,dd)
    { }
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELLSURFACE_H
