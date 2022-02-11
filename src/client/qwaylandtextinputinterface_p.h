/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDTEXTINPUTINTERFACE_P_H
#define QWAYLANDTEXTINPUTINTERFACE_P_H

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

#include <QtCore/qlocale.h>
#include <QtCore/qrect.h>
#include <QtCore/private/qglobal_p.h>

struct wl_surface;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandTextInputInterface
{
public:
    virtual ~QWaylandTextInputInterface() {}
    virtual void reset() = 0;
    virtual void commit() = 0;
    virtual void disableSurface(::wl_surface *surface) = 0;
    virtual void enableSurface(::wl_surface *surface) = 0;
    virtual void updateState(Qt::InputMethodQueries queries, uint32_t flags) = 0;
    virtual void showInputPanel() {}
    virtual void hideInputPanel() {}
    virtual bool isInputPanelVisible() const = 0;
    virtual QRectF keyboardRect() const = 0;
    virtual QLocale locale() const = 0;
    virtual Qt::LayoutDirection inputDirection() const = 0;
    virtual void setCursorInsidePreedit(int cursor) = 0;

    // This enum should be compatible with update_state of text-input-unstable-v2.
    // Higher versions of text-input-* protocol may not use it directly
    // but QtWaylandClient can determine clients' states based on the values
    enum TextInputState {
        update_state_change = 0, // updated state because it changed
        update_state_full = 1, // full state after enter or input_method_changed event
        update_state_reset = 2, // full state after reset
        update_state_enter = 3, // full state after switching focus to a different widget on client side
    };
};

}

QT_END_NAMESPACE

#endif // QWAYLANDTEXTINPUTINTERFACE_P_H

