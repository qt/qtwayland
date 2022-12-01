// Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDXDGOUTPUTV1_H
#define QWAYLANDXDGOUTPUTV1_H

#include <QtCore/QRect>
#include <QtWaylandCompositor/QWaylandCompositorExtension>
#if QT_CONFIG(wayland_compositor_quick)
#include <QtWaylandCompositor/qwaylandquickchildren.h>
#endif

QT_BEGIN_NAMESPACE

class QWaylandOutput;

class QWaylandXdgOutputManagerV1Private;
class QWaylandXdgOutputV1Private;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgOutputManagerV1
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

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandXdgOutputV1 : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandXdgOutputV1)
#if QT_CONFIG(wayland_compositor_quick)
    Q_WAYLAND_COMPOSITOR_DECLARE_QUICK_CHILDREN(QWaylandXdgOutputV1)
#endif

    Q_PROPERTY(QWaylandXdgOutputManagerV1 *manager READ manager NOTIFY managerChanged)
    Q_PROPERTY(QWaylandOutput *output READ output NOTIFY outputChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QPoint logicalPosition READ logicalPosition WRITE setLogicalPosition NOTIFY logicalPositionChanged)
    Q_PROPERTY(QSize logicalSize READ logicalSize WRITE setLogicalSize NOTIFY logicalSizeChanged)
    Q_PROPERTY(QRect logicalGeometry READ logicalGeometry NOTIFY logicalGeometryChanged)
    Q_MOC_INCLUDE("qwaylandoutput.h")
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
