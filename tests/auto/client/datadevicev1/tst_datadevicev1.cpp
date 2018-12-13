/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL-EXCEPT$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mockcompositor.h"

#include <QtGui/QRasterWindow>
#include <QtGui/QOpenGLWindow>

//TODO: move?
#include <QtGui/QClipboard>

using namespace MockCompositor;

constexpr int dataDeviceVersion = 1;

class DataDeviceCompositor : public DefaultCompositor {
public:
    explicit DataDeviceCompositor()
    {
        exec([this] {
            m_config.autoConfigure = true;
            add<DataDeviceManager>(dataDeviceVersion);
        });
    }
    DataDevice *dataDevice() { return get<Seat>()->dataDevice(); }
};

class tst_datadevicev1 : public QObject, private DataDeviceCompositor
{
    Q_OBJECT
private slots:
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void initTestCase();
    void pasteAscii();
    void pasteUtf8();
};

void tst_datadevicev1::initTestCase()
{
    QCOMPOSITOR_TRY_VERIFY(pointer());
    QCOMPOSITOR_TRY_VERIFY(!pointer()->resourceMap().empty());
    QCOMPOSITOR_TRY_COMPARE(pointer()->resourceMap().first()->version(), 4);

    QCOMPOSITOR_TRY_VERIFY(keyboard());

    QCOMPOSITOR_TRY_VERIFY(dataDevice());
    QCOMPOSITOR_TRY_COMPARE(dataDevice()->resource()->version(), dataDeviceVersion);
}

void tst_datadevicev1::pasteAscii()
{
    class Window : public QRasterWindow {
    public:
        void mousePressEvent(QMouseEvent *) override { m_text = QGuiApplication::clipboard()->text(); }
        QString m_text;
    };

    Window window;
    window.resize(64, 64);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);
    exec([&] {
        auto *offer = new DataOffer(client(), dataDeviceVersion); // Cleaned up by destroy_resource
        connect(offer, &DataOffer::receive, [](QString mimeType, int fd) {
            QFile file;
            file.open(fd, QIODevice::WriteOnly, QFile::FileHandleFlag::AutoCloseHandle);
            QCOMPARE(mimeType, "text/plain");
            file.write(QByteArray("normal ascii"));
            file.close();
        });
        dataDevice()->sendDataOffer(offer);
        offer->send_offer("text/plain");
        dataDevice()->sendSelection(offer);

        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol

        pointer()->sendEnter(surface, {32, 32});
        pointer()->sendButton(client(), BTN_LEFT, 1);
        pointer()->sendButton(client(), BTN_LEFT, 0);
    });
    QTRY_COMPARE(window.m_text, "normal ascii");
}

void tst_datadevicev1::pasteUtf8()
{
    class Window : public QRasterWindow {
    public:
        void mousePressEvent(QMouseEvent *) override { m_text = QGuiApplication::clipboard()->text(); }
        QString m_text;
    };

    Window window;
    window.resize(64, 64);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);
    exec([&] {
        auto *offer = new DataOffer(client(), dataDeviceVersion); // Cleaned up by destroy_resource
        connect(offer, &DataOffer::receive, [](QString mimeType, int fd) {
            QFile file;
            file.open(fd, QIODevice::WriteOnly, QFile::FileHandleFlag::AutoCloseHandle);
            QCOMPARE(mimeType, "text/plain;charset=utf-8");
            file.write(QByteArray("face with tears of joy: 😂"));
            file.close();
        });
        dataDevice()->sendDataOffer(offer);
        offer->send_offer("text/plain");
        offer->send_offer("text/plain;charset=utf-8");
        dataDevice()->sendSelection(offer);

        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol

        pointer()->sendEnter(surface, {32, 32});
        pointer()->sendButton(client(), BTN_LEFT, 1);
        pointer()->sendButton(client(), BTN_LEFT, 0);
    });
    QTRY_COMPARE(window.m_text, "face with tears of joy: 😂");
}

QCOMPOSITOR_TEST_MAIN(tst_datadevicev1)
#include "tst_datadevicev1.moc"
