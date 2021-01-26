/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
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

#include "qwaylanddatadevicemanager_p.h"

#include "qwaylandinputdevice_p.h"
#include "qwaylanddatadevice_p.h"
#include "qwaylanddataoffer_p.h"
#include "qwaylanddisplay_p.h"

#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandDataDeviceManager::QWaylandDataDeviceManager(QWaylandDisplay *display, uint32_t id)
    : wl_data_device_manager(display->wl_registry(), id, 1)
    , m_display(display)
{
    // Create transfer devices for all input devices.
    // ### This only works if we get the global before all devices and is surely wrong when hotplugging.
    QList<QWaylandInputDevice *> inputDevices = m_display->inputDevices();
    for (int i = 0; i < inputDevices.size();i++) {
        inputDevices.at(i)->setDataDevice(getDataDevice(inputDevices.at(i)));
    }
}

QWaylandDataDeviceManager::~QWaylandDataDeviceManager()
{
    wl_data_device_manager_destroy(object());
}

QWaylandDataDevice *QWaylandDataDeviceManager::getDataDevice(QWaylandInputDevice *inputDevice)
{
    return new QWaylandDataDevice(this, inputDevice);
}

QWaylandDisplay *QWaylandDataDeviceManager::display() const
{
    return m_display;
}

}

QT_END_NAMESPACE
