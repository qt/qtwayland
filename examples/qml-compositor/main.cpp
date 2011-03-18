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

#include "qtcompositor.h"

#include "waylandsurface.h"

#include <QApplication>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>

#include <QDeclarativeView>

#ifdef QT_COMPOSITOR_WAYLAND_GL
#include <QGLContext>
#include <QGLWidget>
#endif

#include <QtDeclarative/adaptationlayer.h>
#include <QSGContext>
#include <QSGItem>
#include <QSGTextureManager>
#include <QSGTextureProvider>
#include <QSGView>

#include <private/qsgitem_p.h>
#include <private/qsgrectangle_p.h>

class WaylandSurfaceTextureProvider : public QSGTextureProvider
{
    Q_OBJECT
public:
    WaylandSurfaceTextureProvider(WaylandSurface *surface);
    ~WaylandSurfaceTextureProvider();

    QSGTextureRef texture() {
        return m_textureRef;
    }

private slots:
    void surfaceDamaged(const QRect &rect);

private:
    WaylandSurface *m_surface;

    QSGTexture *m_texture;
    QSGTextureRef m_textureRef;
};

WaylandSurfaceTextureProvider::WaylandSurfaceTextureProvider(WaylandSurface *surface)
    : m_surface(surface)
    , m_texture(new QSGTexture)
    , m_textureRef(m_texture)
{
    connect(surface, SIGNAL(damaged(const QRect &)), this, SLOT(surfaceDamaged(const QRect &)));
}

WaylandSurfaceTextureProvider::~WaylandSurfaceTextureProvider()
{
}

void WaylandSurfaceTextureProvider::surfaceDamaged(const QRect &)
{
    if (m_surface->type() == WaylandSurface::Texture) {
        m_texture->setTextureId(m_surface->texture());
        m_texture->setAlphaChannel(false);
    } else {
        if (m_texture->status() == QSGTexture::Null) {
            GLuint textureId;
            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_2D, textureId);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            m_texture->setTextureId(textureId);
        }

        const QImage image = m_surface->image();

        glBindTexture(GL_TEXTURE_2D, m_texture->textureId());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_BGRA_EXT, image.width(), image.height(), 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, image.bits());

        m_texture->setAlphaChannel(image.hasAlphaChannel());
    }

    m_texture->setTextureSize(m_surface->geometry().size());
    m_texture->setOwnsTexture(true);
    m_texture->setHasMipmaps(false);
    m_texture->setStatus(QSGTexture::Ready);

    emit textureChanged();
}

class WindowItem : public QSGItem, public QSGTextureProviderInterface
{
    Q_OBJECT
    Q_INTERFACES(QSGTextureProviderInterface)
public:
    WindowItem(WaylandSurface *surface, QSGItem *parent = 0);
    ~WindowItem();

    QSGTextureProvider *textureProvider() const
    {
        return m_textureProvider;
    }

    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

public slots:
    void takeFocus();

private slots:
    void surfaceMapped(const QRect &rect);

protected:
    Node *updatePaintNode(Node *oldNode, UpdatePaintNodeData *);

private:
    QPoint toSurface(const QPointF &pos) const;

    WaylandSurface *m_surface;
    WaylandSurfaceTextureProvider *m_textureProvider;
};

WindowItem::WindowItem(WaylandSurface *surface, QSGItem *parent)
    : QSGItem(parent)
    , m_surface(surface)
    , m_textureProvider(new WaylandSurfaceTextureProvider(surface))
{
    setWidth(surface->geometry().width());
    setHeight(surface->geometry().height());

    setSmooth(true);
    setFlag(ItemHasContents);
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    connect(surface, SIGNAL(mapped(const QRect &)), this, SLOT(surfaceMapped(const QRect &)));
    connect(m_textureProvider, SIGNAL(textureChanged()), this, SLOT(update()));
}

WindowItem::~WindowItem()
{
    delete m_textureProvider;
}

void WindowItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus())
        m_surface->sendMousePressEvent(toSurface(event->pos()), event->button());
}

void WindowItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus())
        m_surface->sendMouseMoveEvent(toSurface(event->pos()));
}

void WindowItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus())
        m_surface->sendMouseReleaseEvent(toSurface(event->pos()), event->button());
}

void WindowItem::keyPressEvent(QKeyEvent *event)
{
    if (hasFocus())
        m_surface->sendKeyPressEvent(event->nativeScanCode());
}

void WindowItem::keyReleaseEvent(QKeyEvent *event)
{
    if (hasFocus())
        m_surface->sendKeyReleaseEvent(event->nativeScanCode());
}

void WindowItem::takeFocus()
{
    setFocus(true);
    m_surface->setInputFocus();
}

QPoint WindowItem::toSurface(const QPointF &pos) const
{
    return pos.toPoint();
}

void WindowItem::surfaceMapped(const QRect &rect)
{
    setWidth(rect.width());
    setHeight(rect.height());

    update();
}

Node *WindowItem::updatePaintNode(Node *oldNode, UpdatePaintNodeData *)
{
    TextureNodeInterface *node = static_cast<TextureNodeInterface *>(oldNode);
    if (!node) {
        node = QSGContext::current->createTextureNode();
        node->setFlag(Node::UsePreprocess, false);
        node->setTexture(m_textureProvider);
    }

    m_textureProvider->setHorizontalWrapMode(QSGTextureProvider::ClampToEdge);
    m_textureProvider->setVerticalWrapMode(QSGTextureProvider::ClampToEdge);
    m_textureProvider->setFiltering(QSGItemPrivate::get(this)->smooth
                                    ? QSGTextureProvider::Linear : QSGTextureProvider::Nearest);

    node->setTargetRect(QRectF(0, 0, width(), height()));
    node->setSourceRect(QRectF(0, 0, 1, 1));
    node->update();

    return node;
}


class QmlCompositor : public QSGView, public WaylandCompositor
{
    Q_OBJECT
public:
    QmlCompositor() : WaylandCompositor(this) {
        setMouseTracking(true);
        setSource(QUrl(QLatin1String("qml/QmlCompositor/main.qml")));
        winId();
        if (platformWindow()) {
            platformWindow()->glContext();
        }
    }

signals:
    void windowAdded(QVariant window);
    void windowDestroyed(QVariant window);

public slots:
    void destroyWindow(QVariant window) {
        qvariant_cast<QObject *>(window)->deleteLater();
    }

private slots:
    void surfaceMapped(const QRect &rect) {
        WaylandSurface *surface = qobject_cast<WaylandSurface *>(sender());
        surface->setGeometry(rect);

        if (!m_windowMap.contains(surface)) {
            WindowItem *item = new WindowItem(surface, rootObject());
            connect(surface, SIGNAL(destroyed(QObject *)), this, SLOT(surfaceDestroyed(QObject *)));
            emit windowAdded(QVariant::fromValue(static_cast<QSGItem *>(item)));
            m_windowMap[surface] = item;

            item->takeFocus();
        }
    }

    void surfaceDestroyed(QObject *object) {
        WindowItem *item = m_windowMap.take(object);
        emit windowDestroyed(QVariant::fromValue(static_cast<QSGItem *>(item)));
        setInputFocus(0);
    }

protected:
    void surfaceCreated(WaylandSurface *surface) {
        connect(surface, SIGNAL(mapped(const QRect &)), this, SLOT(surfaceMapped(const QRect &)));
    }

    void paintEvent(QPaintEvent *event) {
        QSGView::paintEvent(event);
        frameFinished();
        glFinish();
    }

private:
    QMap<QObject *, WindowItem *> m_windowMap;
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

    return app.exec();

}

#include "main.moc"
