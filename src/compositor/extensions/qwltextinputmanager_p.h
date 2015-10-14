/****************************************************************************
**
** Copyright (C) 2013 Klarälvdalens Datakonsult AB (KDAB).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTWAYLAND_QWLTEXTINPUTMANAGER_P_H
#define QTWAYLAND_QWLTEXTINPUTMANAGER_P_H

#include <QtWaylandCompositor/QWaylandExtension>
#include <QtWaylandCompositor/private/qwayland-server-text.h>

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

class QWaylandCompositor;

namespace QtWayland {

class TextInputManager : public QWaylandExtensionTemplate<TextInputManager>, public QtWaylandServer::wl_text_input_manager
{
    Q_OBJECT
public:
    TextInputManager(QWaylandCompositor *compositor);
    ~TextInputManager();

protected:
    void text_input_manager_create_text_input(Resource *resource, uint32_t id) Q_DECL_OVERRIDE;

private:
    QWaylandCompositor *m_compositor;
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QTWAYLAND_QWLTEXTINPUTMANAGER_P_H
