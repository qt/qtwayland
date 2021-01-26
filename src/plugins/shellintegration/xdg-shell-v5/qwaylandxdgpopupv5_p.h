/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the config.tests of the Qt Toolkit.
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
**
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

#ifndef QWAYLANDXDGPOPUPV5_P_H
#define QWAYLANDXDGPOPUPV5_P_H

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

#include "qwayland-xdg-shell-unstable-v5_p.h"

#include <QtWaylandClient/qtwaylandclientglobal.h>
#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

QT_BEGIN_NAMESPACE

class QWindow;

namespace QtWaylandClient {

class QWaylandWindow;
class QWaylandExtendedSurface;

class Q_WAYLAND_CLIENT_EXPORT QWaylandXdgPopupV5 : public QWaylandShellSurface
        , public QtWayland::xdg_popup_v5
{
    Q_OBJECT
public:
    QWaylandXdgPopupV5(struct ::xdg_popup_v5 *popup, QWaylandWindow *window);
    ~QWaylandXdgPopupV5() override;

protected:
    void xdg_popup_popup_done() override;

private:
    QWaylandExtendedSurface *m_extendedWindow = nullptr;
    QWaylandWindow *m_window = nullptr;
};

QT_END_NAMESPACE

}

#endif // QWAYLANDXDGPOPUPV5_P_H
