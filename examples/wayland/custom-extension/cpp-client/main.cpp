// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include <QGuiApplication>
#include <QRasterWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QPlatformSurfaceEvent>
#include <QDebug>
#include <QTimer>

#include "../client-common/customextension.h"

class TestWindow : public QRasterWindow
{
    Q_OBJECT
public:
    TestWindow(CustomExtension *customExtension)
        : m_extension(customExtension)
        , rect1(50, 50, 100, 100)
        , rect2(50, 200, 100, 100)
        , rect3(50, 350, 100, 100)
        , rect4(200,350, 100, 100)
    {
        setTitle(tr("C++ Client"));
        m_extension->registerWindow(this);

//! [connection]
        connect(m_extension, &CustomExtension::fontSize, this, &TestWindow::handleSetFontSize);
//! [connection]
    }

public slots:
    void doSpin()
    {
        if (!m_extension->isActive()) {
            qWarning() << "Extension is not active";
            return;
        }
        qDebug() << "sending spin...";
        m_extension->sendSpin(this, 500);
    }

    void doBounce()
    {
        if (!m_extension->isActive()) {
            qWarning() << "Extension is not active";
            return;
        }
        qDebug() << "sending bounce...";
        m_extension->sendBounce(this, 500);
    }

    void newWindow()
    {
        auto w = new TestWindow(m_extension);
        w->show();
    }

    CustomExtensionObject *newObject()
    {
        m_objectCount++;
        QColor col = QColor::fromHsv(0, 511 / (m_objectCount + 1), 255);

        return m_extension->createCustomObject(col.name(), QString::number(m_objectCount));
    }

    void handleSetFontSize(QWindow *w, uint pixelSize)
    {
        if (w == this) {
            m_font.setPixelSize(pixelSize);
            update();
        }
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setFont(m_font);
        p.fillRect(QRect(0, 0, width(), height()), Qt::gray);
        p.fillRect(rect1, QColor::fromString("#C0FFEE"));
        p.drawText(rect1, Qt::TextWordWrap, "Press here to send spin request.");
        p.fillRect(rect2, QColor::fromString("#decaff"));
        p.drawText(rect2, Qt::TextWordWrap, "Press here to send bounce request.");
        p.fillRect(rect3, QColor::fromString("#7EA"));
        p.drawText(rect3, Qt::TextWordWrap, "Create new window.");
        p.fillRect(rect4, QColor::fromString("#7EABA6"));
        p.drawText(rect4, Qt::TextWordWrap, "Create custom object.");

    }

//! [mousePressEvent]
    void mousePressEvent(QMouseEvent *ev) override
    {
        if (rect1.contains(ev->position()))
            doSpin();
        else if (rect2.contains(ev->position()))
            doBounce();
        else if (rect3.contains(ev->position()))
            newWindow();
        else if (rect4.contains(ev->position()))
            newObject();
    }
//! [mousePressEvent]

private:
    CustomExtension *m_extension = nullptr;
    QRectF rect1;
    QRectF rect2;
    QRectF rect3;
    QRectF rect4;
    QFont m_font;
    static int m_objectCount;
    static int m_hue;
};

int TestWindow::m_objectCount = 0;
int TestWindow::m_hue;

int main (int argc, char **argv)
{
    QGuiApplication app(argc, argv);

    CustomExtension customExtension;

    TestWindow window(&customExtension);
    window.show();

    return app.exec();
}

#include "main.moc"
