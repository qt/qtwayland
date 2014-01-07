/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandcompositor.h"
#include "qwaylandsurface.h"
#include "qwaylandsurfaceitem.h"

#include <QGuiApplication>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QOpenGLContext>

#include <QQmlContext>

#include <QQuickItem>
#include <QQuickView>

#include "qwayland-server-share-buffer.h"
#include <QtCompositor/private/qwlcompositor_p.h>
#include <QtCompositor/qwaylandserverbufferintegration.h>

#include "serverbufferitem.h"

#include <QtGui/private/qdistancefield_p.h>

class QmlCompositor
    : public QQuickView
    , public QWaylandCompositor
    , public QtWaylandServer::qt_share_buffer
{
    Q_OBJECT

public:
    QmlCompositor()
        : QWaylandCompositor(this, 0, DefaultExtensions | SubSurfaceExtension)
        , QtWaylandServer::qt_share_buffer(QWaylandCompositor::handle()->wl_display())
        , m_server_buffer_32_bit(0)
        , m_server_buffer_item_32_bit(0)
        , m_server_buffer_8_bit(0)
        , m_server_buffer_item_8_bit(0)
    {
        setSource(QUrl("qrc:/qml/main.qml"));
        setResizeMode(QQuickView::SizeRootObjectToView);
        setColor(Qt::black);
        create();
        grabWindow();

        connect(this, SIGNAL(frameSwapped()), this, SLOT(frameSwappedSlot()));

        connect(this, SIGNAL(sceneGraphInitialized()), this, SLOT(initiateServerBuffer()),Qt::DirectConnection);
        connect(this, SIGNAL(serverBuffersCreated()), this, SLOT(createServerBufferItems()));
    }

signals:
    void windowAdded(QVariant window);
    void windowDestroyed(QVariant window);
    void windowResized(QVariant window);
    void serverBufferItemCreated(QVariant);
    void serverBuffersCreated();

public slots:
    void destroyWindow(QVariant window)
    {
        qvariant_cast<QObject *>(window)->deleteLater();
    }

    void destroyClientForWindow(QVariant window)
    {
        QWaylandSurface *surface = qobject_cast<QWaylandSurfaceItem *>(qvariant_cast<QObject *>(window))->surface();
        destroyClientForSurface(surface);
    }

private slots:
    void surfaceMapped() {
        QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());

        QWaylandSurfaceItem *item = surface->surfaceItem();
        if (!item) {
            item = new QWaylandSurfaceItem(surface, rootObject());
            item->setUseTextureAlpha(true);
        }

        item->setTouchEventsEnabled(true);
        emit windowAdded(QVariant::fromValue(static_cast<QQuickItem *>(item)));
    }

    void surfaceUnmapped() {
        QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
        QQuickItem *item = surface->surfaceItem();
        emit windowDestroyed(QVariant::fromValue(item));
    }

    void surfaceDestroyed(QObject *object) {
        QWaylandSurface *surface = static_cast<QWaylandSurface *>(object);
        QQuickItem *item = surface->surfaceItem();
        emit windowDestroyed(QVariant::fromValue(item));
    }

    void frameSwappedSlot() {
        frameFinished();
    }

    void initiateServerBuffer()
    {
        if (!QWaylandCompositor::handle()->serverBufferIntegration())
            return;

        openglContext()->makeCurrent(this);

        QWaylandServerBufferIntegration *sbi = QWaylandCompositor::handle()->serverBufferIntegration();
        if (!sbi) {
            qWarning("Could not find a Server Buffer Integration");
            return;
        }
        if (sbi->supportsFormat(QWaylandServerBuffer::RGBA32)) {
                QImage image(100,100,QImage::Format_ARGB32_Premultiplied);
                image.fill(QColor(0x55,0x0,0x55,0x01));
                {
                QPainter p(&image);
                QPen pen = p.pen();
                pen.setWidthF(3);
                pen.setColor(Qt::red);
                p.setPen(pen);
                p.drawLine(0,0,100,100);
                pen.setColor(Qt::green);
                p.setPen(pen);
                p.drawLine(100,0,0,100);
                pen.setColor(Qt::blue);
                p.setPen(pen);
                p.drawLine(25,15,75,15);
                }
                image = image.convertToFormat(QImage::Format_RGBA8888);

                m_server_buffer_32_bit = sbi->createServerBuffer(image.size(),QWaylandServerBuffer::RGBA32);

                GLuint texture_32_bit;
                glGenTextures(1, &texture_32_bit);
                glBindTexture(GL_TEXTURE_2D, texture_32_bit);
                m_server_buffer_32_bit->bindTextureToBuffer();
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width(), image.height(), GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
                glBindTexture(GL_TEXTURE_2D, 0);

                glBindTexture(GL_TEXTURE_2D, 0);
                glDeleteTextures(1, &texture_32_bit);

        }

        if (sbi->supportsFormat(QWaylandServerBuffer::A8)) {
            QRawFont defaultRaw = QRawFont::fromFont(QFont(), QFontDatabase::Latin);
            QVector<quint32> index = defaultRaw.glyphIndexesForString(QStringLiteral("A"));
            QDistanceField distanceField(defaultRaw, index.front(), true);
            QImage img = distanceField.toImage(QImage::Format_Indexed8);

            m_server_buffer_8_bit = sbi->createServerBuffer(img.size(), QWaylandServerBuffer::A8);
            GLuint texture_8_bit;
            glGenTextures(1, &texture_8_bit);
            glBindTexture(GL_TEXTURE_2D, texture_8_bit);
            m_server_buffer_8_bit->bindTextureToBuffer();

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, img.width(), img.height(), GL_ALPHA, GL_UNSIGNED_BYTE, img.constBits());
        }
        emit serverBuffersCreated();
    }

    void createServerBufferItems()
    {
        if (m_server_buffer_32_bit) {
            m_server_buffer_item_32_bit = new ServerBufferItem(m_server_buffer_32_bit);
            m_server_buffer_item_32_bit->setUseTextureAlpha(true);
            emit serverBufferItemCreated(QVariant::fromValue(m_server_buffer_item_32_bit));
        }
        if (m_server_buffer_8_bit) {
            m_server_buffer_item_8_bit = new ServerBufferItem(m_server_buffer_8_bit);
            m_server_buffer_item_8_bit->setUseTextureAlpha(true);
            emit serverBufferItemCreated(QVariant::fromValue(m_server_buffer_item_8_bit));
        }
    }
protected:
    void resizeEvent(QResizeEvent *event)
    {
        QQuickView::resizeEvent(event);
        QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
    }

    void surfaceCreated(QWaylandSurface *surface) {
        connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
        connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
        connect(surface,SIGNAL(unmapped()), this,SLOT(surfaceUnmapped()));
    }

    void share_buffer_bind_resource(Resource *resource) Q_DECL_OVERRIDE
    {
        if (m_server_buffer_32_bit) {
            struct ::wl_client *client = resource->handle->client;
            struct ::wl_resource *buffer = m_server_buffer_32_bit->resourceForClient(client);
            send_cross_buffer(resource->handle, buffer);

        }
        if (m_server_buffer_8_bit) {
            struct ::wl_client *client = resource->handle->client;
            struct ::wl_resource *buffer = m_server_buffer_8_bit->resourceForClient(client);
            send_cross_buffer(resource->handle, buffer);

        }
    }

private:
    QWaylandServerBuffer *m_server_buffer_32_bit;
    ServerBufferItem *m_server_buffer_item_32_bit;
    QWaylandServerBuffer *m_server_buffer_8_bit;
    ServerBufferItem *m_server_buffer_item_8_bit;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    if (!app.arguments().contains(QStringLiteral("--invert"))) {
        qDebug() << "iverting";
        qputenv("QT_COMPOSITOR_NEGATE_INVERTED_Y", "1");
    }

    qmlRegisterType<ServerBufferItem>();

    QmlCompositor compositor;
    compositor.setTitle(QLatin1String("QML Compositor"));
    compositor.setGeometry(0, 0, 1024, 768);
    compositor.show();

    compositor.rootContext()->setContextProperty("compositor", &compositor);

    QObject::connect(&compositor, SIGNAL(windowAdded(QVariant)), compositor.rootObject(), SLOT(windowAdded(QVariant)));
    QObject::connect(&compositor, SIGNAL(windowDestroyed(QVariant)), compositor.rootObject(), SLOT(windowDestroyed(QVariant)));
    QObject::connect(&compositor, SIGNAL(windowResized(QVariant)), compositor.rootObject(), SLOT(windowResized(QVariant)));
    QObject::connect(&compositor, SIGNAL(serverBufferItemCreated(QVariant)), compositor.rootObject(), SLOT(serverBufferItemCreated(QVariant)));

    app.exec();
    qDebug() << "ending" << glGetError();
}

#include "main.moc"
