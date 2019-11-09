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
#if QT_CONFIG(opengl)
#include <QtGui/QOpenGLWindow>
#endif

using namespace MockCompositor;

class tst_surface : public QObject, private DefaultCompositor
{
    Q_OBJECT
private slots:
    void cleanup() { QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage())); }
    void createDestroySurface();
    void waitForFrameCallbackRaster();
#if QT_CONFIG(opengl)
    void waitForFrameCallbackGl();
#endif
    void negotiateShmFormat();

    // Subsurfaces
    void createSubsurface();
    void createSubsurfaceForHiddenParent();
};

void tst_surface::createDestroySurface()
{
    QWindow window;
    window.show();

    QCOMPOSITOR_TRY_VERIFY(surface());

    window.destroy();
    QCOMPOSITOR_TRY_VERIFY(!surface());
}

void tst_surface::waitForFrameCallbackRaster()
{
    QSKIP("TODO: This currently fails, needs a fix");
    class TestWindow : public QRasterWindow {
    public:
        explicit TestWindow() { resize(40, 40); }
        void paintEvent(QPaintEvent *event) override
        {
            Q_UNUSED(event);
            update();
        }
    };
    TestWindow window;
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    QSignalSpy bufferSpy(exec([=] { return xdgSurface()->m_surface; }), &Surface::bufferCommitted);
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });

    // We should get the first buffer without waiting for a frame callback
    QTRY_COMPARE(bufferSpy.count(), 1);
    bufferSpy.removeFirst();

    // Make sure we follow frame callbacks for some frames
    for (int i = 0; i < 5; ++i) {
        xdgPingAndWaitForPong(); // Make sure things have happened on the client
        exec([&] {
            QVERIFY(bufferSpy.empty()); // Make sure no extra buffers have arrived
            QVERIFY(!xdgToplevel()->surface()->m_waitingFrameCallbacks.empty());
            xdgToplevel()->surface()->sendFrameCallbacks();
        });
        QTRY_COMPARE(bufferSpy.count(), 1);
        bufferSpy.removeFirst();
    }
}

#if QT_CONFIG(opengl)
void tst_surface::waitForFrameCallbackGl()
{
    QSKIP("TODO: This currently fails, needs a fix");
    class TestWindow : public QOpenGLWindow {
    public:
        explicit TestWindow()
        {
            resize(40, 40);
            connect(this, &QOpenGLWindow::frameSwapped,
                    this, QOverload<>::of(&QPaintDeviceWindow::update));
            update();
        }
        void paintGL() override
        {
            glClearColor(1, 0, 0, 1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    };
    TestWindow window;
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    QSignalSpy bufferSpy(exec([=] { return xdgSurface()->m_surface; }), &Surface::bufferCommitted);
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });

    // We should get the first buffer without waiting for a frame callback
    QTRY_COMPARE(bufferSpy.count(), 1);
    bufferSpy.removeFirst();

    // Make sure we follow frame callbacks for some frames
    for (int i = 0; i < 5; ++i) {
        xdgPingAndWaitForPong(); // Make sure things have happened on the client
        exec([&] {
            QVERIFY(bufferSpy.empty()); // Make sure no extra buffers have arrived
            QVERIFY(!xdgToplevel()->surface()->m_waitingFrameCallbacks.empty());
            xdgToplevel()->surface()->sendFrameCallbacks();
        });
        QTRY_COMPARE(bufferSpy.count(), 1);
        bufferSpy.removeFirst();
    }
}
#endif // QT_CONFIG(opengl)

void tst_surface::negotiateShmFormat()
{
    QSKIP("TODO: I'm not sure why we're choosing xrgb over argb in this case...");
    QRasterWindow window;
    window.setFlag(Qt::FramelessWindowHint); // decorations force alpha
    QSurfaceFormat format;
    format.setAlphaBufferSize(0);
    window.setFormat(format);
    window.resize(64, 48);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    QSignalSpy bufferSpy(exec([=] { return xdgSurface()->m_surface; }), &Surface::bufferCommitted);
    const uint serial = exec([=] { return xdgToplevel()->sendCompleteConfigure(); });
    QCOMPOSITOR_TRY_COMPARE(xdgSurface()->m_committedConfigureSerial, serial);
    exec([&] {
        Buffer *buffer = xdgToplevel()->surface()->m_committed.buffer;
        QVERIFY(buffer);
        auto *shmBuffer = ShmBuffer::fromBuffer(buffer);
        QVERIFY(shmBuffer);
        qDebug() << "shmBuffer->m_format" << shmBuffer->m_format;
        QCOMPARE(shmBuffer->m_format, Shm::format_xrgb8888);
    });
}

void tst_surface::createSubsurface()
{
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });
    QCOMPOSITOR_TRY_VERIFY(xdgSurface()->m_committedConfigureSerial);

    QRasterWindow subWindow;
    subWindow.setParent(&window);
    subWindow.resize(64, 64);
    subWindow.show();
    QCOMPOSITOR_TRY_VERIFY(subSurface());
}

// Used to cause a crash in libwayland (QTBUG-79674)
void tst_surface::createSubsurfaceForHiddenParent()
{
    QRasterWindow window;
    window.resize(64, 64);
    window.show();
    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());
    exec([=] { xdgToplevel()->sendCompleteConfigure(); });
    QCOMPOSITOR_TRY_VERIFY(xdgSurface()->m_committedConfigureSerial);

    window.hide();

    QRasterWindow subWindow;
    subWindow.setParent(&window);
    subWindow.resize(64, 64);
    subWindow.show();

    // Make sure the client doesn't quit before it has a chance to crash
    xdgPingAndWaitForPong();
}

QCOMPOSITOR_TEST_MAIN(tst_surface)
#include "tst_surface.moc"
