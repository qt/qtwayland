/****************************************************************************
 **
 ** Copyright (C) 2015 The Qt Company Ltd.
 ** Contact: http://www.qt.io/licensing/
 **
 ** This file is part of the examples of the Qt Wayland module
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
#include <QRasterWindow>
#include <QPainter>
#include <QMouseEvent>

#include <QDebug>

static QObject *s_custom;

class TestWindow : public QRasterWindow
{
    Q_OBJECT

public:
    TestWindow()
    {
        if (s_custom)
            connect(s_custom, SIGNAL(eventReceived(const QString &, uint)),
                    this, SLOT(handleEvent(const QString &, uint)));
    }

public slots:
    void handleEvent(const QString &text, uint value)
    {
        qDebug() << "Client application received event" << text << value;
    }

protected:
    void paintEvent(QPaintEvent *)
    {
        QPainter p(this);
        p.fillRect(QRect(0,0,width(),height()),Qt::gray);
        p.fillRect(50,50,100,100, QColor("#C0FFEE"));
    }

    void mousePressEvent(QMouseEvent *ev) Q_DECL_OVERRIDE
    {
        Q_ASSERT(s_custom);
        bool insideRect = QRect(50,50,100,100).contains(ev->pos());

        QString text = insideRect ? "Click inside" : "Click outside";
        int value = ev->pos().x();

        qDebug() << "Client application sending request:" << text << value;

        QMetaObject::invokeMethod(s_custom, "sendRequest", Qt::DirectConnection,
                                  Q_ARG(QString, text),
                                  Q_ARG(int, value));
    }

private:

};

int main (int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    s_custom = app.findChild<QObject*>("qt_example_custom_extension");
    if (!s_custom) {
        qCritical() << "This example requires the Qt Custom Extension platform plugin,\n"
            "add -platform custom-wayland to the command line";
        return -1;
    }
    TestWindow window;
    window.show();

    return app.exec();
}

#include "main.moc"
