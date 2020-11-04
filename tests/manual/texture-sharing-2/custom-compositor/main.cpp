/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
** Contact: http://www.qt-project.org/legal
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

#include <QtCore/QUrl>
#include <QtCore/QDebug>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>

#include <QtQml/qqml.h>
#include <QtQml/QQmlEngine>

#include <QtGui/QPainter>
#include <QtGui/QImage>

#include <QtCore/QDateTime>

#include "QtWaylandCompositor/private/qwltexturesharingextension_p.h"

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

class CustomSharingExtension : public QWaylandTextureSharingExtension
{
    Q_OBJECT
public:
    CustomSharingExtension() {qDebug("Instantiating custom texture sharing extension.");}
protected:
    bool customPixelData(const QString &key, QByteArray *data, QSize *size, uint *glInternalFormat) override
    {
        qDebug() << "CustomSharingExtension looking for local texture data for" << key;
        if (key.startsWith("unreasonably large ")) {
            int w = 10000;
            int h = 10000;
            int numBytes = w * h * 4;
            *data = QByteArray(numBytes, 0);
            quint32 *pixels = reinterpret_cast<quint32*>(data->data());
            for (int i = 0; i < w*h; ++i)
                pixels[i] = 0xff7f1fff;
            *glInternalFormat = GL_RGBA8;
            *size = QSize(w,h);
            return true;
        }

        QImage img;

        if (key == QLatin1String("test pattern 1")) {
            img = QImage(128,128,QImage::Format_ARGB32_Premultiplied);
            img.fill(QColor(0x55,0x0,0x55,0x01));
            {
                QPainter p(&img);
                QPen pen = p.pen();
                pen.setWidthF(3);
                pen.setColor(Qt::red);
                p.setPen(pen);
                p.drawLine(0,0,128,128);
                pen.setColor(Qt::green);
                p.setPen(pen);
                p.drawLine(128,0,0,128);
                pen.setColor(Qt::blue);
                p.setPen(pen);
                p.drawLine(32,16,96,16);
                pen.setColor(Qt::black);
                p.setPen(pen);
                p.translate(64, 64);
                p.rotate(45);
                p.drawText(QRect(-48, -32, 96, 64),
                           QDateTime::currentDateTime().toString(),
                           QTextOption(Qt::AlignHCenter));
            }
        }

        if (!img.isNull()) {
            img = img.convertToFormat(QImage::Format_RGBA8888);
            *data = QByteArray(reinterpret_cast<const char*>(img.constBits()), img.sizeInBytes());
            *size = img.size();
            *glInternalFormat = GL_RGBA8;
            return true;
        }
        return false;
    }
};

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(CustomSharingExtension);

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine appEngine;

    qmlRegisterType<CustomSharingExtensionQuickExtension>("com.theqtcompany.customsharingextension", 1, 0, "CustomSharingExtension");
    appEngine.addImageProvider("wlshared", new QWaylandSharedTextureProvider);

    appEngine.load(QUrl("qrc:///qml/main.qml"));

    return app.exec();
}

#include "main.moc"
