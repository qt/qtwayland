// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKHARDWARELAYER_P_H
#define QWAYLANDQUICKHARDWARELAYER_P_H

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

#include <QtWaylandCompositor/QWaylandQuickItem>
#include <QtCore/private/qglobal_p.h>

QT_BEGIN_NAMESPACE

class QWaylandQuickHardwareLayerPrivate;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandQuickHardwareLayer : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)
    Q_DECLARE_PRIVATE(QWaylandQuickHardwareLayer)
    Q_PROPERTY(int stackingLevel READ stackingLevel WRITE setStackingLevel NOTIFY stackingLevelChanged)
    QML_NAMED_ELEMENT(WaylandHardwareLayer)
    QML_ADDED_IN_VERSION(1, 2)
public:
    explicit QWaylandQuickHardwareLayer(QObject *parent = nullptr);
    ~QWaylandQuickHardwareLayer() override;

    int stackingLevel() const;
    void setStackingLevel(int level);

    QWaylandQuickItem *waylandItem() const;

    void classBegin() override;
    void componentComplete() override;

    void setSceneGraphPainting(bool);
    void initialize();

Q_SIGNALS:
    void stackingLevelChanged();
};

QT_END_NAMESPACE

#endif // QWAYLANDQUICKHARDWARELAYER_P_H
