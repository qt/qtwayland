/****************************************************************************
**
** Copyright (C) 2014 Jolla Ltd, author: <giulio.camuffo@jollamobile.com>
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

#ifndef QWAYLANDQUICKCOMPOSITOR_H
#define QWAYLANDQUICKCOMPOSITOR_H

#include <QtCompositor/qwaylandcompositor.h>
#include <QtQml/QQmlParserStatus>

QT_BEGIN_NAMESPACE

class QQuickWindow;
class QWaylandQuickCompositorPrivate;
class QWaylandView;

class Q_COMPOSITOR_EXPORT QWaylandQuickCompositor : public QWaylandCompositor, public QQmlParserStatus
{
    Q_INTERFACES(QQmlParserStatus)
    Q_OBJECT
    Q_PROPERTY(bool initializeLegazyQmlNames READ initializeLegazyQmlNames WRITE setInitializeLegazyQmlNames)
public:
    QWaylandQuickCompositor(QObject *parent = 0);
    void create() Q_DECL_OVERRIDE;

    static void registerLegacyQmlNames();
    bool initializeLegazyQmlNames() const;
    void setInitializeLegazyQmlNames(bool init);

    QWaylandOutput *createOutput(QWaylandOutputSpace *outputSpace,
                                 QWindow *window) Q_DECL_OVERRIDE;
    QWaylandSurface *createSurface(QWaylandClient *client, quint32 id, int version) Q_DECL_OVERRIDE;

protected:
    void classBegin() Q_DECL_OVERRIDE;
    void componentComplete() Q_DECL_OVERRIDE;

private:
    bool m_initializeLegazyQmlNames;
};

QT_END_NAMESPACE

#endif
