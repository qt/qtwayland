/****************************************************************************
 **
 ** Copyright (C) 2019 The Qt Company Ltd.
 ** Contact: https://www.qt.io/licensing/
 **
 ** This file is part of the examples of the Qt Wayland module
 **
 ** $QT_BEGIN_LICENSE:BSD$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and The Qt Company. For licensing terms
 ** and conditions see https://www.qt.io/terms-conditions. For further
 ** information use the contact form at https://www.qt.io/contact-us.
 **
 ** BSD License Usage
 ** Alternatively, you may use this file under the terms of the BSD license
 ** as follows:
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
 **   * Neither the name of The Qt Company Ltd nor the names of its
 **     contributors may be used to endorse or promote products derived
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
#include "texturesharingextension.h"

#include <QDebug>
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QTimer>
#include <QMap>

class TestWindow : public QOpenGLWindow
{
    Q_OBJECT

public:
    TestWindow()
        : m_extension(nullptr)
    {
        m_extension = new TextureSharingExtension;
        connect(m_extension, SIGNAL(bufferReceived(QtWaylandClient::QWaylandServerBuffer*, const QString&)), this, SLOT(receiveBuffer(QtWaylandClient::QWaylandServerBuffer*, const QString&)));
        connect(m_extension, &TextureSharingExtension::activeChanged, this, &TestWindow::handleExtensionActive);
    }

public slots:
    void receiveBuffer(QtWaylandClient::QWaylandServerBuffer *buffer, const QString &key)
    {
        if (!buffer) {
            qWarning() << "Could not find image with key" << key;
            return;
        }
        m_buffers.insert(key, buffer);
        update();
    }


    void handleExtensionActive()
    {
        if (m_extension->isActive())
            getImage("qt_logo");
    }

protected:

    void mousePressEvent(QMouseEvent *ev) override {
        QRect rect(10, height() - 10 - 50, 50, 50);
        bool rectPressed = rect.contains(ev->pos());

        static int c;

        if (rectPressed && ev->button() == Qt::LeftButton)
            getImage(QString("unreasonably large image %1").arg(c++));
        else if (ev->button() == Qt::RightButton)
            getImage("guitar.jpg");
        else if (ev->button() == Qt::MiddleButton)
            unloadImageAt(ev->pos());
    }

    void initializeGL() override
    {
        m_blitter = new QOpenGLTextureBlitter;
        m_blitter->create();
    }

    void paintGL() override {
        glClearColor(.5, .45, .42, 1.);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw a "button" to click in
        glScissor(10,10,50,50);
        glEnable(GL_SCISSOR_TEST);
        glClearColor(0.4, 0.7, 0.9, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_SCISSOR_TEST);

        int x = 0;
        qDebug() << "*** paintGL ***";
        showBuffers();
        for (auto buffer: qAsConst(m_buffers)) {
            m_blitter->bind();
            QSize s(buffer->size());
            qDebug() << "painting" << buffer << s;
            if (s.width() > 1024) {
                qDebug() << "showing large buffer at reduced size";
                s = QSize(128,128);
            }
            QRectF targetRect(QPointF(x,0), s);
            QOpenGLTexture *texture = buffer->toOpenGlTexture();
            if (!texture) {
                qWarning("Null texture");
                continue;
            }
            auto surfaceOrigin = QOpenGLTextureBlitter::OriginTopLeft;
            QMatrix4x4 targetTransform = QOpenGLTextureBlitter::targetTransform(targetRect, QRect(QPoint(), size()));
            m_blitter->blit(texture->textureId(), targetTransform, surfaceOrigin);
            m_blitter->release();
            x += s.width() + 10;
        }
    }

private:
    void getImage(const QString &key) {
        if (!m_buffers.contains(key))
            m_extension->requestImage(key);
    }

    void showBuffers() const
    {
        auto end = m_buffers.cend();
        for (auto it = m_buffers.cbegin(); it != end; ++it) {
            qDebug() << "    " << it.key() << it.value();
        }
    }

    void unloadImageAt(const QPoint &pos) {
        int x = 0;
        QtWaylandClient::QWaylandServerBuffer *foundBuffer = nullptr;
        QString name;
        auto end = m_buffers.cend();
        for (auto it = m_buffers.cbegin(); it != end; ++it) {
            auto *buffer = it.value();
            QSize s(buffer->size());
            if (s.width() > 1024)
                s = QSize(128,128);
            QRectF targetRect(QPointF(x,0), s);
            //qDebug() << "    " << it.key() << it.value() << targetRect << pos;

            if (targetRect.contains(pos)) {
                foundBuffer = buffer;
                name = it.key();
                //qDebug() << "FOUND!!";
                break;
            }

            x += s.width() + 10;
        }
        if (foundBuffer) {
            qDebug() << "unloading image" << name << "found at" << pos;
            unloadImage(name);
        } else {
            qDebug() << "no image at" << pos;
        }
    }

    void unloadImage(const QString &key) {
        auto *buf = m_buffers.take(key);
        if (buf) {
            qDebug() << "unloadImage deleting" << buf;
            delete buf;
            m_extension->abandonImage(key);
        }
        update();
    }

    QOpenGLTextureBlitter *m_blitter = nullptr;
    TextureSharingExtension *m_extension = nullptr;
    QMap<QString, QtWaylandClient::QWaylandServerBuffer*> m_buffers;

};

int main (int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    TestWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
