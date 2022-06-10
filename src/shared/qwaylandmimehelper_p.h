// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QWAYLANDMIMEHELPER_H
#define QWAYLANDMIMEHELPER_H

#include <QString>
#include <QByteArray>
#include <QMimeData>
#include <private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QWaylandMimeHelper
{
public:
    static QByteArray getByteArray(QMimeData *mimeData, const QString &mimeType);
};

QT_END_NAMESPACE

#endif
