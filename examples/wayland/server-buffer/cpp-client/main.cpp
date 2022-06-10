// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QtGui/private/qguiapplication_p.h>
#include <QOpenGLWindow>
#include <QOpenGLTexture>
#include <QOpenGLTextureBlitter>
#include <QPainter>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>

#include <QtWaylandClient/private/qwaylanddisplay_p.h>
#include <QtWaylandClient/private/qwaylandintegration_p.h>
#include <QtWaylandClient/private/qwaylandserverbufferintegration_p.h>
#include "sharebufferextension.h"

#include <QDebug>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QTimer>
#include <QMap>

class TestWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    TestWindow()
    {
        m_extension = new ShareBufferExtension;
        connect(m_extension, SIGNAL(bufferReceived(QtWaylandClient::QWaylandServerBuffer*)), this, SLOT(receiveBuffer(QtWaylandClient::QWaylandServerBuffer*)));
    }

public slots:
    void receiveBuffer(QtWaylandClient::QWaylandServerBuffer *buffer)
    {
        m_buffers.append(buffer);
        update();
    }

protected:

    void initializeGL() override
    {
        m_blitter = new QOpenGLTextureBlitter;
        m_blitter->create();
    }

    void paintGL() override {
        glClearColor(.5, .45, .42, 1.);
        glClear(GL_COLOR_BUFFER_BIT);
        int x = 0;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        for (auto buffer: m_buffers) {
            m_blitter->bind();
            QPointF pos(x,0);
            QSize s(buffer->size());
            QRectF targetRect(pos, s);
            QOpenGLTexture *texture = buffer->toOpenGlTexture();
            auto surfaceOrigin = QOpenGLTextureBlitter::OriginTopLeft;
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(targetRect, QRect(QPoint(), size()));
            m_blitter->blit(texture->textureId(), targetTransform, surfaceOrigin);
            m_blitter->release();
            x += s.width() + 10;
        }
    }

private:

    QOpenGLTextureBlitter *m_blitter = nullptr;
    ShareBufferExtension *m_extension = nullptr;
    QList<QtWaylandClient::QWaylandServerBuffer*> m_buffers;

};

int main (int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    TestWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
