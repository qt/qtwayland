// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

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

    qmlRegisterType<CustomSharingExtensionQuickExtension>("io.qt.tests.customsharingextension", 1, 0, "CustomSharingExtension");
    appEngine.addImageProvider("wlshared", new QWaylandSharedTextureProvider);

    appEngine.load(QUrl("qrc:///qml/main.qml"));

    return app.exec();
}

#include "main.moc"
