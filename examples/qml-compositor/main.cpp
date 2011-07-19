/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "waylandcompositor.h"
#include "waylandsurface.h"
#include "waylandsurfaceitem.h"

#include <QApplication>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>

#include <QDeclarativeContext>
#include <QtQuick1/QDeclarativeView>

#include <QSGItem>
#include <QSGView>

class QmlCompositor : public QSGView, public WaylandCompositor
{
    Q_OBJECT
public:
    QmlCompositor() : WaylandCompositor(this, const_cast<QGLContext *>(context())) {
        setMouseTracking(true);
        setSource(QUrl(QLatin1String("qrc:qml/QmlCompositor/main.qml")));
        setResizeMode(QSGView::SizeRootObjectToView);
        winId();
    }

signals:
    void windowAdded(QVariant window);
    void windowDestroyed(QVariant window);
    void windowResized(QVariant window);

public slots:
    void destroyWindow(QVariant window) {
        qvariant_cast<QObject *>(window)->deleteLater();
    }

private slots:
    void surfaceMapped(const QSize &size) {
        WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
        surface->setGeometry(QRect(surface->geometry().topLeft(),size));

        if (m_windowMap.contains(surface)) {
            WaylandSurfaceItem *item = m_windowMap.value(surface);
            item->setWidth(size.width());
            item->setHeight(size.height());
            emit windowResized(QVariant::fromValue(static_cast<QSGItem *>(item)));
        } else {
            WaylandSurfaceItem *item = new WaylandSurfaceItem(surface, rootObject());
            item->setTouchEventsEnabled(true);
            connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
            emit windowAdded(QVariant::fromValue(static_cast<QSGItem *>(item)));
            m_windowMap[surface] = item;

            item->takeFocus();
        }
    }

    void surfaceDestroyed(QObject *object) {
        WaylandSurfaceItem *item = m_windowMap.take(object);
        emit windowDestroyed(QVariant::fromValue(static_cast<QSGItem *>(item)));
    }

protected:
    void surfaceCreated(WaylandSurface *surface) {
        connect(surface, SIGNAL(mapped(const QSize &)), this, SLOT(surfaceMapped(const QSize &)));
    }

    void paintEvent(QPaintEvent *event) {
        QSGView::paintEvent(event);
        frameFinished();
        glFinish();
    }

private:
    QMap<QObject *, WaylandSurfaceItem *> m_windowMap;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QmlCompositor compositor;
    compositor.setWindowTitle(QLatin1String("QML Compositor"));
    compositor.show();

    compositor.rootContext()->setContextProperty("compositor", &compositor);

    QObject::connect(&compositor, SIGNAL(windowAdded(QVariant)), compositor.rootObject(), SLOT(windowAdded(QVariant)));
    QObject::connect(&compositor, SIGNAL(windowDestroyed(QVariant)), compositor.rootObject(), SLOT(windowDestroyed(QVariant)));
    QObject::connect(&compositor, SIGNAL(windowResized(QVariant)), compositor.rootObject(), SLOT(windowResized(QVariant)));

    return app.exec();
}

#include "main.moc"
