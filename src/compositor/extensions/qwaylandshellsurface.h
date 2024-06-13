// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSHELLSURFACE_H
#define QWAYLANDSHELLSURFACE_H

#include <QtWaylandCompositor/qtwaylandqmlinclude.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>

QT_BEGIN_NAMESPACE

class QWaylandQuickShellIntegration;
class QWaylandQuickShellSurfaceItem;
class QWaylandShellSurfacePrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShellSurface : public QWaylandCompositorExtension
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandShellSurface)

    Q_PROPERTY(Qt::WindowType windowType READ windowType NOTIFY windowTypeChanged)
    Q_PROPERTY(bool modal READ isModal NOTIFY modalChanged FINAL REVISION(6, 8))
    QML_NAMED_ELEMENT(ShellSurface)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
#if QT_CONFIG(wayland_compositor_quick)
    virtual QWaylandQuickShellIntegration *createIntegration(QWaylandQuickShellSurfaceItem *item) = 0;
#endif
    QWaylandShellSurface(QWaylandObject *waylandObject);
    virtual Qt::WindowType windowType() const { return Qt::WindowType::Window; }

    bool isModal() const;

protected:
    QWaylandShellSurface(QWaylandShellSurfacePrivate &dd);
    QWaylandShellSurface(QWaylandObject *container, QWaylandShellSurfacePrivate &dd);
    void setModal(bool newModal);

Q_SIGNALS:
    void windowTypeChanged();
    Q_REVISION(6, 8) void modalChanged();
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
    QWaylandShellSurfaceTemplate(QWaylandShellSurfacePrivate &dd)
        : QWaylandShellSurface(dd)
    { }

    QWaylandShellSurfaceTemplate(QWaylandObject *container, QWaylandShellSurfacePrivate &dd)
        : QWaylandShellSurface(container,dd)
    { }
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELLSURFACE_H
