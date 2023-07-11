// Copyright (C) 2017 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDINPUTMETHODCONTROL_P_H
#define QWAYLANDINPUTMETHODCONTROL_P_H

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

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandinputmethodcontrol.h>

#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class QWaylandCompositor;
class QWaylandSeat;
class QWaylandSurface;
class QWaylandTextInput;
class QWaylandTextInputV3;
class QWaylandQtTextInputMethod;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandInputMethodControlPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandInputMethodControl)

public:
    explicit QWaylandInputMethodControlPrivate(QWaylandSurface *surface);

    QWaylandTextInput *textInput() const;
    QWaylandTextInputV3 *textInputV3() const;
    QWaylandQtTextInputMethod *textInputMethod() const;

    QWaylandCompositor *compositor = nullptr;
    QWaylandSeat *seat = nullptr;
    QWaylandSurface *surface = nullptr;
    bool enabled = false;
};

QT_END_NAMESPACE

#endif // QWAYLANDINPUTMETHODCONTROL_P_H
