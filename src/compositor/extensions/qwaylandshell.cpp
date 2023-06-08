// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandshell.h"
#include "qwaylandshell_p.h"

QT_BEGIN_NAMESPACE

QWaylandShellPrivate::QWaylandShellPrivate()
{
}

QWaylandShell::QWaylandShell()
{
}

QWaylandShell::QWaylandShell(QWaylandObject *waylandObject)
    : QWaylandCompositorExtension(waylandObject, *new QWaylandShellPrivate())
{
}

/*!
 * \enum QWaylandShell::FocusPolicy
 *
 * This enum type is used to specify the focus policy for shell surfaces.
 *
 * \value AutomaticFocus Shell surfaces will automatically get keyboard focus when they are created.
 * \value ManualFocus The compositor will decide whether shell surfaces should get keyboard focus or not.
 */

/*!
 * \qmlproperty enumeration Shell::focusPolicy
 *
 * This property holds the focus policy of the Shell.
 */

/*!
 * \property QWaylandShell::focusPolicy
 *
 * This property holds the focus policy of the QWaylandShell.
 */
QWaylandShell::FocusPolicy QWaylandShell::focusPolicy() const
{
    Q_D(const QWaylandShell);
    return d->focusPolicy;
}

void QWaylandShell::setFocusPolicy(QWaylandShell::FocusPolicy focusPolicy)
{
    Q_D(QWaylandShell);

    if (d->focusPolicy == focusPolicy)
        return;

    d->focusPolicy = focusPolicy;
    emit focusPolicyChanged();
}

QWaylandShell::QWaylandShell(QWaylandShellPrivate &dd)
    : QWaylandCompositorExtension(dd)
{
}

QWaylandShell::QWaylandShell(QWaylandObject *container, QWaylandShellPrivate &dd)
    : QWaylandCompositorExtension(container, dd)
{
}

QT_END_NAMESPACE

#include "moc_qwaylandshell.cpp"
