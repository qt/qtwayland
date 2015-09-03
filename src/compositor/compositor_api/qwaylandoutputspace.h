/****************************************************************************
**
** Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDOUTPUTSPACE_H
#define QWAYLANDOUTPUTSPACE_H

#include <QtCore/QObject>

#include <QtCompositor/QWaylandOutput>

QT_BEGIN_NAMESPACE

class QWaylandOutputSpacePrivate;

class Q_COMPOSITOR_EXPORT QWaylandOutputSpace : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandOutputSpace)

    Q_ENUMS(GeometryConstraint)

    Q_PROPERTY(GeometryConstraint geometryConstraint READ geometryConstraint WRITE setGeometryConstraint NOTIFY geometryConstraintChanged)
    Q_PROPERTY(QRect geometry READ geometry WRITE setGeometry NOTIFY geometryChanged)
    Q_PROPERTY(QWaylandOutput *defaultOutput READ defaultOutput WRITE setDefaultOutput NOTIFY defaultOutputChanged)
    Q_PROPERTY(QList<QWaylandOutput *> outputs READ outputs NOTIFY outputsChanged)
public:
    enum GeometryConstraint {
        AutomaticBoundingRect,
        FixedSize
    };

    QWaylandOutputSpace(QWaylandCompositor *compositor);

    QWaylandCompositor *compositor() const;

    QRect geometry() const;
    void setGeometry(const QRect &geometry);

    void setGeometryConstraint(GeometryConstraint geometryConstraint);
    GeometryConstraint geometryConstraint() const;

    Q_INVOKABLE QWaylandOutput *output(QWindow *window) const;

    QWaylandOutput *defaultOutput() const;
    void setDefaultOutput(QWaylandOutput *output);

    Q_INVOKABLE QList<QWaylandOutput *>outputs() const;
    Q_INVOKABLE QList<QWaylandOutput *>outputs(const QPoint &point) const;

    Q_INVOKABLE QWaylandView *pickView(const QPointF &globalPosition) const;
    Q_INVOKABLE QPointF mapToView(QWaylandView *view, const QPointF &spacePoint) const;
    Q_INVOKABLE QPointF mapToSpace(QWaylandView *view, const QPointF &local) const;

Q_SIGNALS:
    void geometryConstraintChanged();
    void geometryChanged();

    void outputsChanged();
    void defaultOutputChanged();

private:
    Q_DISABLE_COPY(QWaylandOutputSpace)
};

QT_END_NAMESPACE

#endif  /*QWAYLANDOUTPUTSPACE_H*/
