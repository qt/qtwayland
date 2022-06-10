// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDSHELL_P_H
#define QWAYLANDSHELL_P_H

#include <QtWaylandCompositor/private/qwaylandcompositorextension_p.h>
#include <QtWaylandCompositor/QWaylandShell>

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandShellPrivate : public QWaylandCompositorExtensionPrivate
{
    Q_DECLARE_PUBLIC(QWaylandShell)
public:
    QWaylandShellPrivate();

    QWaylandShell::FocusPolicy focusPolicy = QWaylandShell::AutomaticFocus;
};

QT_END_NAMESPACE

#endif // QWAYLANDSHELL_P_H
