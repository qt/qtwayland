/****************************************************************************
**
** Copyright (C) 2019 The Qt Company Ltd.
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
#include <QtGui/QScreen>
#include <QtGui/QRasterWindow>

using namespace MockCompositor;

class NoOutputCompositor : public DefaultCompositor {
public:
    NoOutputCompositor()
    {
        exec([this] { removeAll<Output>(); });
        m_config.autoConfigure = true;
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

    // We have to handle showing a window when there are no real outputs
    QCOMPOSITOR_TRY_VERIFY(xdgSurface() && xdgSurface()->m_committedConfigureSerial);
}

QCOMPOSITOR_TEST_MAIN(tst_nooutput)
#include "tst_nooutput.moc"
