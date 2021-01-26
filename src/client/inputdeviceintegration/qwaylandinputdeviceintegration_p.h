/****************************************************************************
**
** Copyright (C) 2016 LG Electronics Ltd
** Contact: https://www.qt.io/licensing/
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

#ifndef QWAYLANDINPUTDEVICEINTEGRATION_H
#define QWAYLANDINPUTDEVICEINTEGRATION_H

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

#include <QtCore/qglobal.h>
#include <QtWaylandClient/qtwaylandclientglobal.h>

#include <stdint.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandInputDevice;

class Q_WAYLAND_CLIENT_EXPORT QWaylandInputDeviceIntegration
{
public:
    QWaylandInputDeviceIntegration() {}
    virtual ~QWaylandInputDeviceIntegration() {}

    virtual QWaylandInputDevice *createInputDevice(QWaylandDisplay *d, int version, uint32_t id) = 0;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDINPUTDEVICEINTEGRATION_H
