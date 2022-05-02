/****************************************************************************
**
** Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
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
******************************************************************************/

#ifndef QWAYLANDINPUTMETHODCONTROL_H
#define QWAYLANDINPUTMETHODCONTROL_H

#include <QtGui/qtguiglobal.h>
#include <QObject>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandInputMethodControlPrivate;
class QWaylandSurface;
class QInputMethodEvent;

class QWaylandInputMethodControl : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandInputMethodControl)
    Q_DISABLE_COPY(QWaylandInputMethodControl)

    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
public:
    explicit QWaylandInputMethodControl(QWaylandSurface *surface);

    QVariant inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const;

    void inputMethodEvent(QInputMethodEvent *event);

    bool enabled() const;
    void setEnabled(bool enabled);

    void setSurface(QWaylandSurface *surface);

Q_SIGNALS:
    void enabledChanged(bool enabled);
    void updateInputMethod(Qt::InputMethodQueries queries);

private:
    void defaultSeatChanged();
    void surfaceEnabled(QWaylandSurface *surface);
    void surfaceDisabled(QWaylandSurface *surface);
};

QT_END_NAMESPACE

#endif // QWAYLANDINPUTMETHODCONTROL_H
