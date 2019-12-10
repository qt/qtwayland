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
#include <QtGui/qpa/qplatformnativeinterface.h>
#include <QtWaylandClient/private/wayland-wayland-client-protocol.h>

using namespace MockCompositor;

class tst_xdgshell : public QObject, private DefaultCompositor
{
    Q_OBJECT
private slots:
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void showMinimized();
    void basicConfigure();
    void configureSize();
    void configureStates();
    void popup();
    void tooltipOnPopup();
    void switchPopups();
    void hidePopupParent();
    void pongs();
    void minMaxSize();
    void windowGeometry();
    void foreignSurface();
};

void tst_xdgshell::showMinimized()
{
    QSKIP("TODO: This currently fails, needs a fix");
    // On xdg-shell there's really no way for the compositor to tell the window if it's minimized
    // There are wl_surface.enter events and so on, but there's really no way to differentiate
    // between a window preview and an unminimized window.
    QWindow window;
    window.showMinimized();
    QCOMPARE(window.windowStates(), Qt::WindowMinimized);   // should return minimized until
    QTRY_COMPARE(window.windowStates(), Qt::WindowNoState); // rejected by handleWindowStateChanged

    // Make sure the window on the compositor side is/was created here, and not after the test
    // finishes, as that may mess up for later tests.
    QCOMPOSITOR_TRY_VERIFY(xdgSurface());
    QVERIFY(!window.isExposed());
}

void tst_xdgshell::basicConfigure()
{
    QRasterWindow window;
    window.resize(64, 48);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());

    QSignalSpy configureSpy(exec([=] { return xdgSurface(); }), &XdgSurface::configureCommitted);

    QTRY_VERIFY(window.isVisible());
    // The window should not be exposed before the first xdg_surface configure event
    QTRY_VERIFY(!window.isExposed());

    exec([=] {
        xdgToplevel()->sendConfigure({0, 0}, {}); // Let the window decide the size
    });

    // Nothing should happen before the *xdg_surface* configure
    QTRY_VERIFY(!window.isExposed()); //Window should not be exposed before the first configure event
    QVERIFY(configureSpy.isEmpty());

    const uint serial = exec([=] { return nextSerial(); });

    exec([=] {
        xdgSurface()->sendConfigure(serial);
    });

    // Finally, we're exposed
    QTRY_VERIFY(window.isExposed());

    // The client is now going to ack the configure
    QTRY_COMPARE(configureSpy.count(), 1);
    QCOMPARE(configureSpy.takeFirst().at(0).toUInt(), serial);

    // And attach a buffer
    exec([&] {
        Buffer *buffer = xdgToplevel()->surface()->m_committed.buffer;
        QVERIFY(buffer);
        QCOMPARE(buffer->size(), window.frameGeometry().size());
    });
}

void tst_xdgshell::configureSize()
{
    QRasterWindow window;
    window.resize(64, 48);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());

    QSignalSpy configureSpy(exec([=] { return xdgSurface(); }), &XdgSurface::configureCommitted);

    const QSize configureSize(60, 40);

    exec([=] {
        xdgToplevel()->sendCompleteConfigure(configureSize);
    });

    QTRY_COMPARE(configureSpy.count(), 1);

    exec([=] {
        Buffer *buffer = xdgToplevel()->surface()->m_committed.buffer;
        QVERIFY(buffer);
        QCOMPARE(buffer->size(), configureSize);
    });
}

void tst_xdgshell::configureStates()
{
    QRasterWindow window;
    window.resize(64, 48);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());

    const QSize windowedSize(320, 240);
    const uint windowedSerial = exec([=] {
        return xdgToplevel()->sendCompleteConfigure(windowedSize, { XdgToplevel::state_activated });
    });
    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committedConfigureSerial, windowedSerial);
    QCOMPARE(window.visibility(), QWindow::Windowed);
    QCOMPARE(window.windowStates(), Qt::WindowNoState);
    QCOMPARE(window.frameGeometry().size(), windowedSize);
    // Toplevel windows don't know their position on xdg-shell
//    QCOMPARE(window.frameGeometry().topLeft(), QPoint()); // TODO: this doesn't currently work when window decorations are enabled

//    QEXPECT_FAIL("", "configure has already been acked, we shouldn't have to wait for isActive", Continue);
//    QVERIFY(window.isActive());
    QTRY_VERIFY(window.isActive()); // Just make sure it eventually get's set correctly

    const QSize screenSize(640, 480);
    const uint maximizedSerial = exec([=] {
        return xdgToplevel()->sendCompleteConfigure(screenSize, { XdgToplevel::state_activated, XdgToplevel::state_maximized });
    });
    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committedConfigureSerial, maximizedSerial);
    QCOMPARE(window.visibility(), QWindow::Maximized);
    QCOMPARE(window.windowStates(), Qt::WindowMaximized);
    QCOMPARE(window.frameGeometry().size(), screenSize);
//    QCOMPARE(window.frameGeometry().topLeft(), QPoint()); // TODO: this doesn't currently work when window decorations are enabled

    const uint fullscreenSerial = exec([=] {
        return xdgToplevel()->sendCompleteConfigure(screenSize, { XdgToplevel::state_activated, XdgToplevel::state_fullscreen });
    });
    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committedConfigureSerial, fullscreenSerial);
    QCOMPARE(window.visibility(), QWindow::FullScreen);
    QCOMPARE(window.windowStates(), Qt::WindowFullScreen);
    QCOMPARE(window.frameGeometry().size(), screenSize);
//    QCOMPARE(window.frameGeometry().topLeft(), QPoint()); // TODO: this doesn't currently work when window decorations are enabled

    // The window should remember its original size
    const uint restoreSerial = exec([=] {
        return xdgToplevel()->sendCompleteConfigure({0, 0}, { XdgToplevel::state_activated });
    });
    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committedConfigureSerial, restoreSerial);
    QCOMPARE(window.visibility(), QWindow::Windowed);
    QCOMPARE(window.windowStates(), Qt::WindowNoState);
    QCOMPARE(window.frameGeometry().size(), windowedSize);
//    QCOMPARE(window.frameGeometry().topLeft(), QPoint()); // TODO: this doesn't currently work when window decorations are enabled
}

void tst_xdgshell::popup()
{
    class Window : public QRasterWindow {
    public:
        void mousePressEvent(QMouseEvent *event) override
        {
            QRasterWindow::mousePressEvent(event);
            m_popup.reset(new QRasterWindow);
            m_popup->setTransientParent(this);
            m_popup->setFlags(Qt::Popup);
            m_popup->resize(100, 100);
            m_popup->show();
        }
        QScopedPointer<QRasterWindow> m_popup;
    };
    Window window;
    window.resize(200, 200);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    QSignalSpy toplevelConfigureSpy(exec([=] { return xdgSurface(); }), &XdgSurface::configureCommitted);
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });
    QTRY_COMPARE(toplevelConfigureSpy.count(), 1);

    uint clickSerial = exec([=] {
        auto *surface = xdgToplevel()->surface();
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(surface, {100, 100});
        p->sendFrame(c);
        uint serial = p->sendButton(client(), BTN_LEFT, Pointer::button_state_pressed);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_released);
        p->sendFrame(c);
        return serial;
    });

    QTRY_VERIFY(window.m_popup);
    QCOMPOSITOR_TRY_VERIFY(xdgPopup());
    QSignalSpy popupConfigureSpy(exec([=] { return xdgPopup()->m_xdgSurface; }), &XdgSurface::configureCommitted);
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_grabbed);
    QCOMPOSITOR_TRY_COMPARE(xdgPopup()->m_grabSerial, clickSerial);

    QRasterWindow *popup = window.m_popup.get();
    QVERIFY(!popup->isExposed()); // wait for configure

    //TODO: Verify it works with a different configure window geometry
    exec([=] { xdgPopup()->sendConfigure(QRect(100, 100, 100, 100)); });

    // Nothing should happen before the *xdg_surface* configure
    QTRY_VERIFY(!popup->isExposed()); // Popup shouldn't be exposed before the first configure event
    QVERIFY(popupConfigureSpy.isEmpty());

    const uint configureSerial = exec([=] {
        return xdgPopup()->m_xdgSurface->sendConfigure();
    });

    // Finally, we're exposed
    QTRY_VERIFY(popup->isExposed());

    // The client is now going to ack the configure
    QTRY_COMPARE(popupConfigureSpy.count(), 1);
    QCOMPARE(popupConfigureSpy.takeFirst().at(0).toUInt(), configureSerial);

    // And attach a buffer
    exec([&] {
        Buffer *buffer = xdgPopup()->surface()->m_committed.buffer;
        QVERIFY(buffer);
        QCOMPARE(buffer->size(), popup->frameGeometry().size());
    });
}

void tst_xdgshell::tooltipOnPopup()
{
    class Popup : public QRasterWindow {
    public:
        explicit Popup(QWindow *parent) {
            setTransientParent(parent);
            setFlags(Qt::Popup);
            resize(100, 100);
            show();
        }
        void mousePressEvent(QMouseEvent *event) override {
            QRasterWindow::mousePressEvent(event);
            m_tooltip = new QRasterWindow;
            m_tooltip->setTransientParent(this);
            m_tooltip->setFlags(Qt::ToolTip);
            m_tooltip->resize(100, 100);
            m_tooltip->show();
        }
        QWindow *m_tooltip = nullptr;
    };

    class Window : public QRasterWindow {
    public:
        void mousePressEvent(QMouseEvent *event) override {
            QRasterWindow::mousePressEvent(event);
            m_popup = new Popup(this);
        }
        Popup *m_popup = nullptr;
    };

    Window window;
    window.resize(200, 200);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel()->m_xdgSurface->m_committedConfigureSerial);

    exec([=] {
        auto *surface = xdgToplevel()->surface();
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(surface, {100, 100});
        p->sendFrame(c);
        p->sendButton(client(), BTN_LEFT, Pointer::button_state_pressed);
        p->sendButton(client(), BTN_LEFT, Pointer::button_state_released);
        p->sendFrame(c);
        p->sendLeave(surface);
        p->sendFrame(c);
    });

    QCOMPOSITOR_TRY_VERIFY(xdgPopup());
    exec([=] { xdgPopup()->sendCompleteConfigure(QRect(100, 100, 100, 100)); });
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_xdgSurface->m_committedConfigureSerial);
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_grabbed);

    exec([=] {
        auto *surface = xdgPopup()->surface();
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(surface, {100, 100});
        p->sendFrame(c);
        p->sendButton(client(), BTN_LEFT, Pointer::button_state_pressed);
        p->sendButton(client(), BTN_LEFT, Pointer::button_state_released);
        p->sendFrame(c);
    });

    QCOMPOSITOR_TRY_VERIFY(xdgPopup(1));
    exec([=] { xdgPopup(1)->sendCompleteConfigure(QRect(100, 100, 100, 100)); });
    QCOMPOSITOR_TRY_VERIFY(xdgPopup(1)->m_xdgSurface->m_committedConfigureSerial);
    QCOMPOSITOR_TRY_VERIFY(!xdgPopup(1)->m_grabbed);

    // Close the middle popup (according protocol this should also destroy any nested popups)
    window.m_popup->close();

    // Close order is verified in XdgSurface::xdg_surface_destroy

    QCOMPOSITOR_TRY_COMPARE(xdgPopup(1), nullptr);
    QCOMPOSITOR_TRY_COMPARE(xdgPopup(0), nullptr);
}

// QTBUG-65680
void tst_xdgshell::switchPopups()
{
    class Popup : public QRasterWindow {
    public:
        explicit Popup(QWindow *parent) {
            setTransientParent(parent);
            setFlags(Qt::Popup);
            resize(10, 10);
            show();
        }
    };

    class Window : public QRasterWindow {
    public:
        void mousePressEvent(QMouseEvent *event) override {
            QRasterWindow::mousePressEvent(event);
            if (!m_popups.empty())
                m_popups.last()->setVisible(false);
        }
        // The bug this checks for, is the case where there is a flushWindowSystemEvents() call
        // somewhere within setVisible(false) before the grab has been released.
        // This leads to the the release event below—including its show() call—to be handled
        // At a time where there is still an active grab, making it illegal for the new popup to
        // grab.
        void mouseReleaseEvent(QMouseEvent *event) override {
            QRasterWindow::mouseReleaseEvent(event);
            m_popups << new Popup(this);
        }
        ~Window() override { qDeleteAll(m_popups); }
        QVector<Popup *> m_popups;
    };

    Window window;
    window.resize(200, 200);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel()->m_xdgSurface->m_committedConfigureSerial);

    exec([=] {
        auto *surface = xdgToplevel()->surface();
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(surface, {100, 100});
        p->sendFrame(c);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_pressed);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_released);
        p->sendFrame(c);
        p->sendLeave(surface);
        p->sendFrame(c);
    });

    QCOMPOSITOR_TRY_VERIFY(xdgPopup());
    exec([=] { xdgPopup()->sendCompleteConfigure(QRect(100, 100, 100, 100)); });
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_xdgSurface->m_committedConfigureSerial);
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_grabbed);

    QSignalSpy firstDestroyed(exec([=] { return xdgPopup(); }), &XdgPopup::destroyRequested);

    exec([=] {
        auto *surface = xdgToplevel()->surface();
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(surface, {100, 100});
        p->sendFrame(c);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_pressed);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_released);
        p->sendFrame(c);
    });

    // The client will now hide one popup and then show another
    firstDestroyed.wait();

    QCOMPOSITOR_TRY_VERIFY(xdgPopup());
    QCOMPOSITOR_TRY_VERIFY(!xdgPopup(1));

    // Verify we still grabbed in case the client has checks to avoid illegal grabs
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_grabbed);

    // Sometimes the popup will select another parent if one is illegal at the time it tries to
    // grab. So we verify we got the intended parent on the compositor side.
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_parentXdgSurface == xdgToplevel()->m_xdgSurface);

    // For good measure just check that configuring works as usual
    exec([=] { xdgPopup()->sendCompleteConfigure(QRect(100, 100, 100, 100)); });
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_xdgSurface->m_committedConfigureSerial);
}

void tst_xdgshell::hidePopupParent()
{
    class Window : public QRasterWindow {
    public:
        void mousePressEvent(QMouseEvent *event) override
        {
            QRasterWindow::mousePressEvent(event);
            m_popup.reset(new QRasterWindow);
            m_popup->setTransientParent(this);
            m_popup->setFlags(Qt::Popup);
            m_popup->resize(100, 100);
            m_popup->show();
        }
        QScopedPointer<QRasterWindow> m_popup;
    };
    Window window;
    window.resize(200, 200);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel()->m_xdgSurface->m_committedConfigureSerial);

    exec([=] {
        auto *surface = xdgToplevel()->surface();
        auto *p = pointer();
        auto *c = client();
        p->sendEnter(surface, {100, 100});
        p->sendFrame(c);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_pressed);
        p->sendButton(c, BTN_LEFT, Pointer::button_state_released);
        p->sendFrame(c);
    });
    QCOMPOSITOR_TRY_VERIFY(xdgPopup());
    exec([=] {
        xdgPopup()->sendConfigure(QRect(100, 100, 100, 100));
        xdgPopup()->m_xdgSurface->sendConfigure();
    });
    QCOMPOSITOR_TRY_VERIFY(xdgPopup()->m_xdgSurface->m_committedConfigureSerial);

    window.hide();
    QCOMPOSITOR_TRY_VERIFY(!xdgToplevel());
}

void tst_xdgshell::pongs()
{
    // Create and show a window to trigger shell integration initialzation,
    // otherwise we don't have anything to send ping events to.
    QRasterWindow window;
    window.resize(200, 200);
    window.show();

    // Verify that the client has bound to the global
    QCOMPOSITOR_TRY_COMPARE(get<XdgWmBase>()->resourceMap().size(), 1);

    QSignalSpy pongSpy(exec([=] { return get<XdgWmBase>(); }), &XdgWmBase::pong);
    const uint serial = exec([=] { return nextSerial(); });
    exec([=] {
        auto *base = get<XdgWmBase>();
        wl_resource *resource = base->resourceMap().first()->handle;
        base->send_ping(resource, serial);
    });
    QTRY_COMPARE(pongSpy.count(), 1);
    QCOMPARE(pongSpy.first().at(0).toUInt(), serial);
}

void tst_xdgshell::minMaxSize()
{
    QRasterWindow window;
    window.setMinimumSize(QSize(100, 100));
    window.setMaximumSize(QSize(1000, 1000));
    window.resize(400, 320);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());

    exec([=] { xdgToplevel()->sendCompleteConfigure(); });

    QCOMPOSITOR_TRY_COMPARE(xdgToplevel()->m_committed.minSize, QSize(100, 100));
    QCOMPOSITOR_TRY_COMPARE(xdgToplevel()->m_committed.maxSize, QSize(1000, 1000));

    window.setMaximumSize(QSize(500, 400));
    QCOMPOSITOR_TRY_COMPARE(xdgToplevel()->m_committed.maxSize, QSize(500, 400));

    window.setMinimumSize(QSize(50, 40));
    QCOMPOSITOR_TRY_COMPARE(xdgToplevel()->m_committed.minSize, QSize(50, 40));
}

void tst_xdgshell::windowGeometry()
{
    QRasterWindow window;
    window.resize(400, 320);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());

    exec([=] { xdgToplevel()->sendCompleteConfigure(); });

    QSize marginsSize;
    marginsSize.setWidth(window.frameMargins().left() + window.frameMargins().right());
    marginsSize.setHeight(window.frameMargins().top() + window.frameMargins().bottom());

    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committed.windowGeometry, QRect(QPoint(0, 0), QSize(400, 320) + marginsSize));

    window.resize(800, 600);
    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committed.windowGeometry, QRect(QPoint(0, 0), QSize(800, 600) + marginsSize));
}

void tst_xdgshell::foreignSurface()
{
    auto *ni = QGuiApplication::platformNativeInterface();
    auto *compositor = static_cast<::wl_compositor *>(ni->nativeResourceForIntegration("compositor"));
    ::wl_surface *foreignSurface = wl_compositor_create_surface(compositor);

    // There *could* be cursor surfaces lying around, we don't want to confuse those with
    // the foreign surface we will be creating.
    const int newSurfaceIndex = exec([&]{
        return get<WlCompositor>()->m_surfaces.size();
    });

    QCOMPOSITOR_TRY_VERIFY(surface(newSurfaceIndex));
    exec([&] {
        pointer()->sendEnter(surface(newSurfaceIndex), {32, 32});
        pointer()->sendLeave(surface(newSurfaceIndex));
    });

    // Just do something to make sure we don't destroy the surface before
    // the pointer events above are handled.
    QSignalSpy spy(exec([=] { return surface(newSurfaceIndex); }), &Surface::commit);
    wl_surface_commit(foreignSurface);
    QTRY_COMPARE(spy.count(), 1);

    wl_surface_destroy(foreignSurface);
}

QCOMPOSITOR_TEST_MAIN(tst_xdgshell)
#include "tst_xdgshell.moc"
