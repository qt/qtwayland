/****************************************************************************
**
** Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
****************************************************************************/

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
 * \qmlproperty enumeration QtWaylandCompositor::Shell::focusPolicy
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

QWaylandShell::QWaylandShell(QWaylandCompositorExtensionPrivate &dd)
    : QWaylandShell(static_cast<QWaylandShellPrivate &>(dd))
{
}

QWaylandShell::QWaylandShell(QWaylandObject *container, QWaylandCompositorExtensionPrivate &dd)
    : QWaylandShell(container, static_cast<QWaylandShellPrivate &>(dd))
{
}

QT_END_NAMESPACE

#include "moc_qwaylandshell.cpp"
