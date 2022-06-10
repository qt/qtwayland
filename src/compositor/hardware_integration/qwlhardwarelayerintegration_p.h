// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDHARDWARELAYERINTEGRATION_H
#define QWAYLANDHARDWARELAYERINTEGRATION_H

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

#include <QObject>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QPoint;

class QWaylandQuickHardwareLayer;

namespace QtWayland {

class Q_WAYLANDCOMPOSITOR_EXPORT HardwareLayerIntegration : public QObject
{
    Q_OBJECT
public:
    HardwareLayerIntegration(QObject *parent = nullptr)
        : QObject(parent)
    {}
    ~HardwareLayerIntegration() override {}
    virtual void add(QWaylandQuickHardwareLayer *) {}
    virtual void remove(QWaylandQuickHardwareLayer *) {}
};

} // namespace QtWayland

QT_END_NAMESPACE

#endif // QWAYLANDHARDWARELAYERINTEGRATION_H
