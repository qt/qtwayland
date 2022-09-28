// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDINPUTMETHODCONTROL_H
#define QWAYLANDINPUTMETHODCONTROL_H

#include <QtGui/qtguiglobal.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandInputMethodControlPrivate;
class QWaylandSurface;
class QInputMethodEvent;
class QWaylandTextInput;

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
    void updateTextInput();

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
