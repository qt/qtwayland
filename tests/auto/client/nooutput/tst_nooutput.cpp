// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "mockcompositor.h"
#include <QtGui/QScreen>
#include <QtGui/QRasterWindow>

using namespace MockCompositor;

class NoOutputCompositor : public DefaultCompositor {
public:
    NoOutputCompositor()
    {
        exec([this] { removeAll<Output>(); });
        m_config.autoConfigure = false;
    }
};

class tst_nooutput : public QObject, private NoOutputCompositor
{
    Q_OBJECT
private slots:
    void cleanup()
    {
        // There should be no wl_outputs in this test
        QCOMPOSITOR_COMPARE(getAll<Output>().size(), 0);
        QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage()));
    }
    void noScreens();
};

void tst_nooutput::noScreens()
{
    QRasterWindow window;
    window.resize(16, 16);
    window.show();

    QCOMPOSITOR_TRY_VERIFY(xdgToplevel());

    QTRY_VERIFY(window.isVisible());
    // The window should not be exposed before the first xdg_surface configure event
    QTRY_VERIFY(!window.isExposed());

    exec([&] {
        xdgToplevel()->sendConfigure({0, 0}, {}); // Let the window decide the size
        xdgSurface()->sendConfigure(nextSerial());
    });

    QTRY_VERIFY(window.isExposed());
}

QCOMPOSITOR_TEST_MAIN(tst_nooutput)
#include "tst_nooutput.moc"
