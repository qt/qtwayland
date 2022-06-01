/****************************************************************************
**
** Copyright (C) 2016 Robin Burchell <robin.burchell@viroteck.net>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#include "qwaylanddecorationplugin_p.h"

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

QWaylandDecorationPlugin::QWaylandDecorationPlugin(QObject *parent)
    : QObject(parent)
{
}
QWaylandDecorationPlugin::~QWaylandDecorationPlugin()
{
}

}

QT_END_NAMESPACE

#include "moc_qwaylanddecorationplugin_p.cpp"
