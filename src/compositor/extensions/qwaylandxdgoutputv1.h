/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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
****************************************************************************/

#ifndef QWAYLANDXDGOUTPUTV1_H
#define QWAYLANDXDGOUTPUTV1_H

#include <QRect>
#include <QtWaylandCompositor/QWaylandCompositorExtension>
#include <QtWaylandCompositor/qwaylandquickchildren.h>

QT_BEGIN_NAMESPACE

class QWaylandOutput;

class QWaylandXdgOutputManagerV1Private;
class QWaylandXdgOutputV1Private;

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgOutputManagerV1
        : public QWaylandCompositorExtensionTemplate<QWaylandXdgOutputManagerV1>
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgOutputManagerV1)
public:
    explicit QWaylandXdgOutputManagerV1();
    QWaylandXdgOutputManagerV1(QWaylandCompositor *compositor);

    void initialize() override;

    static const wl_interface *interface();
};

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandXdgOutputV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgOutputV1)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandXdgOutputV1)
    Q_PROPERTY(QWaylandXdgOutputManagerV1 *manager READ manager NOTIFY managerChanged)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QPoint logicalPosition READ logicalPosition WRITE setLogicalPosition NOTIFY logicalPositionChanged)
    Q_PROPERTY(QSize logicalSize READ logicalSize WRITE setLogicalSize NOTIFY logicalSizeChanged)
    Q_PROPERTY(QRect logicalGeometry READ logicalGeometry NOTIFY logicalGeometryChanged)
public:
    QWaylandXdgOutputV1();
    QWaylandXdgOutputV1(QWaylandOutput *output, QWaylandXdgOutputManagerV1 *manager);
    ~QWaylandXdgOutputV1() override;

    QWaylandXdgOutputManagerV1 *manager() const;
    QWaylandOutput *output() const;

    QString name() const;
    void setName(const QString &name);

    QString description() const;
    void setDescription(const QString &name);

    QPoint logicalPosition() const;
    void setLogicalPosition(const QPoint &position);

    QSize logicalSize() const;
    void setLogicalSize(const QSize &size);

    QRect logicalGeometry() const;

Q_SIGNALS:
    void managerChanged();
    void outputChanged();
    void logicalPositionChanged();
    void logicalSizeChanged();
    void logicalGeometryChanged();
    void nameChanged();
    void descriptionChanged();
};

QT_END_NAMESPACE

#endif // QWAYLANDXDGOUTPUTV1_H
