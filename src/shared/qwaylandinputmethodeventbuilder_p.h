/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
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

#ifndef QWAYLANDINPUTMETHODEVENTBUILDER_H
#define QWAYLANDINPUTMETHODEVENTBUILDER_H

#include <QInputMethodEvent>

QT_BEGIN_NAMESPACE

class QWaylandInputMethodEventBuilder
{
public:
    QWaylandInputMethodEventBuilder() = default;
    ~QWaylandInputMethodEventBuilder();

    void reset();

    void setCursorPosition(int32_t index, int32_t anchor);
    void setDeleteSurroundingText(uint32_t beforeLength, uint32_t afterLength);

    void addPreeditStyling(uint32_t index, uint32_t length, uint32_t style);
    void setPreeditCursor(int32_t index);

    QInputMethodEvent buildCommit(const QString &text);
    QInputMethodEvent buildPreedit(const QString &text);

    static int indexFromWayland(const QString &text, int length, int base = 0);
    static int indexToWayland(const QString &text, int length, int base = 0);
private:
    QPair<int, int> replacementForDeleteSurrounding();

    int32_t m_anchor = 0;
    int32_t m_cursor = 0;
    uint32_t m_deleteBefore = 0;
    uint32_t m_deleteAfter = 0;

    int32_t m_preeditCursor = 0;
    QList<QInputMethodEvent::Attribute> m_preeditStyles;
};

struct QWaylandInputMethodContentType {
    uint32_t hint;
    uint32_t purpose;

    static QWaylandInputMethodContentType convert(Qt::InputMethodHints hints);
};


QT_END_NAMESPACE

#endif // QWAYLANDINPUTMETHODEVENTBUILDER_H
