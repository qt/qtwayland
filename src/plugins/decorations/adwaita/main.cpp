// Copyright (C) 2023 Jan Grulich <jgrulich@redhat.com>
// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#include <QtWaylandClient/private/qwaylanddecorationplugin_p.h>

#include "qwaylandadwaitadecoration_p.h"

QT_BEGIN_NAMESPACE

using namespace Qt::StringLiterals;

namespace QtWaylandClient {

class QWaylandAdwaitaDecorationPlugin : public QWaylandDecorationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandDecorationFactoryInterface_iid FILE "adwaita.json")
public:
    QWaylandAbstractDecoration *create(const QString &key, const QStringList &params) override;
};

QWaylandAbstractDecoration *QWaylandAdwaitaDecorationPlugin::create(const QString &key, const QStringList &params)
{
    Q_UNUSED(params);
    if (!key.compare("adwaita"_L1, Qt::CaseInsensitive) ||
        !key.compare("gnome"_L1, Qt::CaseInsensitive))
        return new QWaylandAdwaitaDecoration();
    return nullptr;
}

}

QT_END_NAMESPACE

#include "main.moc"
