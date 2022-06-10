// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDKEYMAP_P_H
#define QWAYLANDKEYMAP_P_H

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

#include <QtWaylandCompositor/qwaylandkeymap.h>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandKeymapPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QWaylandKeymap)
public:
    QWaylandKeymapPrivate(const QString &layout, const QString &variant, const QString &options,
                          const QString &model, const QString &rules);

    QString m_layout;
    QString m_variant;
    QString m_options;
    QString m_rules;
    QString m_model;
};

QT_END_NAMESPACE

#endif // QWAYLANDKEYMAP_P_H
