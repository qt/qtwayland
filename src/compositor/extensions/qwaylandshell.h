// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSHELL_H
#define QWAYLANDSHELL_H

#include <QtWaylandCompositor/qtwaylandqmlinclude.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>

QT_BEGIN_NAMESPACE

class QWaylandShellPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShell : public QWaylandCompositorExtension
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandShell)
    Q_PROPERTY(FocusPolicy focusPolicy READ focusPolicy WRITE setFocusPolicy NOTIFY focusPolicyChanged)

    QML_NAMED_ELEMENT(Shell)
    QML_UNCREATABLE("")
    QML_ADDED_IN_VERSION(1, 0)
public:
    enum FocusPolicy {
        AutomaticFocus,
        ManualFocus
    };
    Q_ENUM(FocusPolicy)

    QWaylandShell();
    QWaylandShell(QWaylandObject *waylandObject);

    FocusPolicy focusPolicy() const;
    void setFocusPolicy(FocusPolicy focusPolicy);

Q_SIGNALS:
    void focusPolicyChanged();

protected:
    explicit QWaylandShell(QWaylandShellPrivate &dd);
    explicit QWaylandShell(QWaylandObject *container, QWaylandShellPrivate &dd);
};

template <typename T>
class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShellTemplate : public QWaylandShell
{
public:
    QWaylandShellTemplate()
        : QWaylandShell()
    { }

    QWaylandShellTemplate(QWaylandObject *container)
        : QWaylandShell(container)
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
    QWaylandShellTemplate(QWaylandShellPrivate &dd)
        : QWaylandShell(dd)
    { }

    QWaylandShellTemplate(QWaylandObject *container, QWaylandShellPrivate &dd)
        : QWaylandShell(container,dd)
    { }
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELL_H
