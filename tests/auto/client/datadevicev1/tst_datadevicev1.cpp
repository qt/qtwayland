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
#include <QtGui/QClipboard>
#include <QtGui/QDrag>

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
    DataDevice *dataDevice() { return get<DataDeviceManager>()->deviceFor(get<Seat>()); }
};

class tst_datadevicev1 : public QObject, private DataDeviceCompositor
{
    Q_OBJECT
private slots:
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void initTestCase();
    void pasteAscii();
    void pasteUtf8();
    void destroysPreviousSelection();
    void destroysSelectionWithSurface();
    void destroysSelectionOnLeave();
    void dragWithoutFocus();
};

void tst_datadevicev1::initTestCase()
{
    QCOMPOSITOR_TRY_VERIFY(pointer());
    QCOMPOSITOR_TRY_VERIFY(!pointer()->resourceMap().empty());
    QCOMPOSITOR_TRY_COMPARE(pointer()->resourceMap().first()->version(), 5);

    QCOMPOSITOR_TRY_VERIFY(keyboard());

    QCOMPOSITOR_TRY_VERIFY(dataDevice());
    QCOMPOSITOR_TRY_VERIFY(dataDevice()->resourceMap().contains(client()));
    QCOMPOSITOR_TRY_COMPARE(dataDevice()->resourceMap().value(client())->version(), dataDeviceVersion);
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
        auto *client = xdgSurface()->resource()->client();
        auto *offer = dataDevice()->sendDataOffer(client, {"text/plain"});
        connect(offer, &DataOffer::receive, [](QString mimeType, int fd) {
            QFile file;
            file.open(fd, QIODevice::WriteOnly, QFile::FileHandleFlag::AutoCloseHandle);
            QCOMPARE(mimeType, "text/plain");
            file.write(QByteArray("normal ascii"));
            file.close();
        });
        dataDevice()->sendSelection(offer);

        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol

        pointer()->sendEnter(surface, {32, 32});
        pointer()->sendFrame(client);
        pointer()->sendButton(client, BTN_LEFT, 1);
        pointer()->sendFrame(client);
        pointer()->sendButton(client, BTN_LEFT, 0);
        pointer()->sendFrame(client);
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
        auto *client = xdgSurface()->resource()->client();
        auto *offer = dataDevice()->sendDataOffer(client, {"text/plain", "text/plain;charset=utf-8"});
        connect(offer, &DataOffer::receive, [](QString mimeType, int fd) {
            QFile file;
            file.open(fd, QIODevice::WriteOnly, QFile::FileHandleFlag::AutoCloseHandle);
            QCOMPARE(mimeType, "text/plain;charset=utf-8");
            file.write(QByteArray("face with tears of joy: 😂"));
            file.close();
        });
        dataDevice()->sendSelection(offer);

        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol

        pointer()->sendEnter(surface, {32, 32});
        pointer()->sendFrame(client);
        pointer()->sendButton(client, BTN_LEFT, 1);
        pointer()->sendFrame(client);
        pointer()->sendButton(client, BTN_LEFT, 0);
        pointer()->sendFrame(client);
    });
    QTRY_COMPARE(window.m_text, "face with tears of joy: 😂");
}

void tst_datadevicev1::destroysPreviousSelection()
{
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    // When the client receives a selection event, it is required to destroy the previous offer
    exec([&] {
        QCOMPARE(dataDevice()->m_sentSelectionOffers.size(), 0);
        auto *offer = dataDevice()->sendDataOffer(client(), {"text/plain"});
        dataDevice()->sendSelection(offer);
        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol
        QCOMPARE(dataDevice()->m_sentSelectionOffers.size(), 1);
    });

    exec([&] {
        auto *offer = dataDevice()->sendDataOffer(client(), {"text/plain"});
        dataDevice()->sendSelection(offer);
        QCOMPARE(dataDevice()->m_sentSelectionOffers.size(), 2);
    });

    // Verify the first offer gets destroyed
    QCOMPOSITOR_TRY_COMPARE(dataDevice()->m_sentSelectionOffers.size(), 1);

    exec([&] {
        auto *offer = dataDevice()->sendDataOffer(client(), {"text/plain"});
        dataDevice()->sendSelection(offer);
        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendLeave(surface);
    });

    // Clients are required to destroy their offer when losing keyboard focus
    QCOMPOSITOR_TRY_COMPARE(dataDevice()->m_sentSelectionOffers.size(), 0);
}

void tst_datadevicev1::destroysSelectionWithSurface()
{
    auto *window = new QRasterWindow;
    window->resize(64, 64);
    window->show();

    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    // When the client receives a selection event, it is required to destroy the previous offer
    exec([&] {
        QCOMPARE(dataDevice()->m_sentSelectionOffers.size(), 0);
        auto *offer = dataDevice()->sendDataOffer(client(), {"text/plain"});
        dataDevice()->sendSelection(offer);
        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol
        QCOMPARE(dataDevice()->m_sentSelectionOffers.size(), 1);
    });

    // Ping to make sure we receive the wl_keyboard enter and leave events, before destroying the
    // surface. Otherwise, the client will receive enter and leave events with a destroyed (null)
    // surface, which is not what we are trying to test for here.
    xdgPingAndWaitForPong();
    window->destroy();

    QCOMPOSITOR_TRY_COMPARE(dataDevice()->m_sentSelectionOffers.size(), 0);
}

void tst_datadevicev1::destroysSelectionOnLeave()
{
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    exec([&] {
        auto *offer = dataDevice()->sendDataOffer(client(), {"text/plain"});
        dataDevice()->sendSelection(offer);

        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendEnter(surface); // Need to set keyboard focus according to protocol
    });

    QTRY_VERIFY(QGuiApplication::clipboard()->mimeData(QClipboard::Clipboard));
    QTRY_VERIFY(QGuiApplication::clipboard()->mimeData(QClipboard::Clipboard)->hasText());

    QSignalSpy dataChangedSpy(QGuiApplication::clipboard(), &QClipboard::dataChanged);

    exec([&] {
        auto *surface = xdgSurface()->m_surface;
        keyboard()->sendLeave(surface);
    });

    QTRY_COMPARE(dataChangedSpy.count(), 1);
    QVERIFY(!QGuiApplication::clipboard()->mimeData(QClipboard::Clipboard)->hasText());
}

// The application should not crash if it attempts to start a drag operation
// when it doesn't have input focus (QTBUG-76368)
void tst_datadevicev1::dragWithoutFocus()
{
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);

    auto *mimeData = new QMimeData;
    const QByteArray data("testData");
    mimeData->setData("text/plain", data);
    QDrag drag(&window);
    drag.setMimeData(mimeData);
    drag.exec();
}

QCOMPOSITOR_TEST_MAIN(tst_datadevicev1)
#include "tst_datadevicev1.moc"
