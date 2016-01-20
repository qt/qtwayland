/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandClient module of the Qt Toolkit.
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


#ifndef QWAYLANDINPUTCONTEXT_H
#define QWAYLANDINPUTCONTEXT_H

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

#include <qpa/qplatforminputcontext.h>

#include <QtWaylandClient/private/qwayland-text.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;

class QWaylandTextInput : public QtWayland::wl_text_input
{
public:
    QWaylandTextInput(struct ::wl_text_input *text_input);

    QString commitString() const;

    void reset();
    void updateState();

protected:
    void text_input_preedit_string(uint32_t serial, const QString &text, const QString &commit) Q_DECL_OVERRIDE;
    void text_input_commit_string(uint32_t serial, const QString &text) Q_DECL_OVERRIDE;
    void text_input_enter(wl_surface *surface) Q_DECL_OVERRIDE;
    void text_input_leave() Q_DECL_OVERRIDE;
    void text_input_keysym(uint32_t serial, uint32_t time, uint32_t sym, uint32_t state, uint32_t modifiers);

private:
    QString m_commit;

    uint32_t m_serial;
    uint32_t m_resetSerial;
};

class QWaylandInputContext : public QPlatformInputContext
{
    Q_OBJECT
public:
    explicit QWaylandInputContext(QWaylandDisplay *display);

    bool isValid() const Q_DECL_OVERRIDE;

    void reset() Q_DECL_OVERRIDE;
    void commit() Q_DECL_OVERRIDE;
    void update(Qt::InputMethodQueries) Q_DECL_OVERRIDE;
    void invokeAction(QInputMethod::Action, int cursorPosition) Q_DECL_OVERRIDE;

    void showInputPanel() Q_DECL_OVERRIDE;
    void hideInputPanel() Q_DECL_OVERRIDE;
    bool isInputPanelVisible() const Q_DECL_OVERRIDE;

    void setFocusObject(QObject *object) Q_DECL_OVERRIDE;

private:
    bool ensureTextInput();

    QWaylandDisplay *mDisplay;
    QScopedPointer<QWaylandTextInput> mTextInput;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDINPUTCONTEXT_H
