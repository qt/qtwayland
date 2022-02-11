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
#include "textinput.h"
#include "qttextinput.h"

#include <QtCore/QString>
#include <QtCore/QByteArray>

#include <QtGui/private/qguiapplication_p.h>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qpa/qplatformintegration.h>
#include <QtGui/qpa/qplatforminputcontextfactory_p.h>

#include <QtTest/QtTest>

using namespace MockCompositor;

class tst_inputcontext : public QObject, private DefaultCompositor
{
    Q_OBJECT
private slots:
    void initTestCase();
    void selectingInputContext_data();
    void selectingInputContext();
    void selectingTextInputProtocol_data();
    void selectingTextInputProtocol();
    void inputContextReconfigurationWhenTogglingTextInputExtension();

private:
    QByteArray inputContextName() const;

    template<typename arg_type>
    void ensurePresentOnCompositor()
    {
        exec([&] {
            QList<arg_type *> extensions = getAll<arg_type>();
            if (extensions.length() > 1)
                QFAIL("Requested type is a singleton, hence there should not be more then one object returned");
            if (extensions.length() == 0)
                add<arg_type>();
        });
    }

    template<typename arg_type>
    void ensureNotPresentOnCompositor()
    {
        exec([&] {
            QList<arg_type *> extensions = getAll<arg_type>();
            if (extensions.length() > 1)
                QFAIL("Requested type is a singleton, hence there should not be more then one object returned");
            if (extensions.length() == 1)
                remove(extensions.first());
        });
    }

    QByteArray mComposeModule = QByteArray("QComposeInputContext"); // default input context
    QByteArray mIbusModule    = QByteArray("QIBusPlatformInputContext");
    QByteArray mTextInputModule = QByteArray("QtWaylandClient::QWaylandInputContext");
    QByteArray mQtTextInputModule = QByteArray("QtWaylandClient::QWaylandInputMethodContext");
};

void tst_inputcontext::initTestCase()
{
    // Verify that plugins are present and valid
    QPlatformInputContext *context = QPlatformInputContextFactory::create(QStringLiteral("compose"));
    QVERIFY(context && context->isValid());

    context = QPlatformInputContextFactory::create(QStringLiteral("ibus"));
    // The ibus plugin depends on properly configured system services, if plugin is not valid
    // verify that wayland qpa plugin properly fallbacks to default input context.
    if (!context || !context->isValid())
        mIbusModule = mComposeModule;
}

QByteArray tst_inputcontext::inputContextName() const
{
    QPlatformIntegration *platformIntegration = QGuiApplicationPrivate::platformIntegration();
    if (platformIntegration->inputContext())
        return platformIntegration->inputContext()->metaObject()->className();

    return QByteArray("");
}

void tst_inputcontext::selectingInputContext_data()
{
    QTest::addColumn<QByteArray>("requestedModule");
    QTest::addColumn<QByteArray>("expectedModule");

    // Test compositor without Text Input extension
    QTest::newRow("ibus")    << QByteArray("ibus")    << mIbusModule;
    QTest::newRow("compose") << QByteArray("compose") << mComposeModule;
    QTest::newRow("empty")   << QByteArray("")        << mComposeModule;
    QTest::newRow("null")    << QByteArray()          << mComposeModule;
    QTest::newRow("fake")    << QByteArray("fake")    << mComposeModule;

    // Test compositor with Text Input extension
    QTest::newRow("ibus:text-input")    << QByteArray("ibus")    << mIbusModule;
    QTest::newRow("compose:text-input") << QByteArray("compose") << mComposeModule;
    QTest::newRow("empty:text-input")   << QByteArray("")        << mComposeModule;
    QTest::newRow("null:text-input")    << QByteArray()          << mTextInputModule;
    QTest::newRow("fake:text-input")    << QByteArray("fake")    << mComposeModule;
}

void tst_inputcontext::selectingInputContext()
{
    QFETCH(QByteArray, requestedModule);
    QFETCH(QByteArray, expectedModule);

    if (requestedModule.isNull())
        qunsetenv("QT_IM_MODULE");
    else
        qputenv("QT_IM_MODULE", requestedModule);

    const bool withTextInputAtCompositorSide = QByteArray(QTest::currentDataTag()).endsWith(":text-input");

    if (withTextInputAtCompositorSide)
        ensurePresentOnCompositor<TextInputManager>();
    else
        ensureNotPresentOnCompositor<TextInputManager>();

    int argc = 0;
    QGuiApplication app(argc, nullptr); // loads the platform plugin

    QCOMPARE(inputContextName(), expectedModule);
}

void tst_inputcontext::selectingTextInputProtocol_data()
{
    QTest::addColumn<bool>("requestQtTextInput");
    QTest::addColumn<bool>("requestTextInput");
    QTest::addColumn<QByteArray>("clientProtocol");
    QTest::addColumn<QByteArray>("expectedModule");

    QTest::newRow("1-1") << true << true << QByteArray() << mQtTextInputModule;
    QTest::newRow("1-2") << true << false << QByteArray() << mQtTextInputModule;
    QTest::newRow("1-3") << false << true << QByteArray() << mTextInputModule;
    QTest::newRow("1-4") << false << false << QByteArray() << mComposeModule;

    QTest::newRow("2-1") << true << true << QByteArray("zwp_text_input_v2") << mTextInputModule;
    QTest::newRow("2-2") << true << false << QByteArray("zwp_text_input_v2") << mComposeModule;
    QTest::newRow("2-3") << false << true << QByteArray("zwp_text_input_v2") << mTextInputModule;
    QTest::newRow("2-4") << false << false << QByteArray("zwp_text_input_v2") << mComposeModule;

    QTest::newRow("3-1") << true << true << QByteArray("qt_text_input_method_v1") << mQtTextInputModule;
    QTest::newRow("3-2") << true << false << QByteArray("qt_text_input_method_v1") << mQtTextInputModule;
    QTest::newRow("3-3") << false << true << QByteArray("qt_text_input_method_v1") << mComposeModule;
    QTest::newRow("3-4") << false << false << QByteArray("qt_text_input_method_v1") << mComposeModule;

    QTest::newRow("4-1") << true << true << QByteArray("qt_text_input_method_v1;zwp_text_input_v2") << mQtTextInputModule;
    QTest::newRow("4-2") << true << false << QByteArray("qt_text_input_method_v1;zwp_text_input_v2") << mQtTextInputModule;
    QTest::newRow("4-3") << false << true << QByteArray("qt_text_input_method_v1;zwp_text_input_v2") << mTextInputModule;
    QTest::newRow("4-4") << false << false << QByteArray("qt_text_input_method_v1;zwp_text_input_v2") << mComposeModule;

    QTest::newRow("5-1") << true << true << QByteArray("zwp_text_input_v2;qt_text_input_method_v1") << mTextInputModule;
    QTest::newRow("5-2") << true << false << QByteArray("zwp_text_input_v2;qt_text_input_method_v1") << mQtTextInputModule;
    QTest::newRow("5-3") << false << true << QByteArray("zwp_text_input_v2;qt_text_input_method_v1") << mTextInputModule;
    QTest::newRow("5-4") << false << false << QByteArray("zwp_text_input_v2;qt_text_input_method_v1") << mComposeModule;
}

void tst_inputcontext::selectingTextInputProtocol()
{
    QFETCH(bool, requestQtTextInput);
    QFETCH(bool, requestTextInput);
    QFETCH(QByteArray, clientProtocol);
    QFETCH(QByteArray, expectedModule);

    exec([] {
        qputenv("QT_IM_MODULE", "qtvirtualkeyboard");
    });

    qunsetenv("QT_IM_MODULE");

    if (clientProtocol.isNull())
        qunsetenv("QT_WAYLAND_TEXT_INPUT_PROTOCOL");
    else
        qputenv("QT_WAYLAND_TEXT_INPUT_PROTOCOL", clientProtocol);

    if (requestTextInput)
        ensurePresentOnCompositor<TextInputManager>();
    else
        ensureNotPresentOnCompositor<TextInputManager>();

    if (requestQtTextInput)
        ensurePresentOnCompositor<QtTextInputManager>();
    else
        ensureNotPresentOnCompositor<QtTextInputManager>();

    int argc = 0;
    QGuiApplication app(argc, nullptr); // loads the platform plugin

    QCOMPARE(inputContextName(), expectedModule);
}

void tst_inputcontext::inputContextReconfigurationWhenTogglingTextInputExtension()
{
    qunsetenv("QT_IM_MODULE");

    ensurePresentOnCompositor<TextInputManager>();
    int argc = 0;
    QGuiApplication app(argc, nullptr); // loads the platform plugin
    QCOMPARE(inputContextName(), mTextInputModule);

    // remove text input extension after the platform plugin has been loaded
    ensureNotPresentOnCompositor<TextInputManager>();
    // QTRY_* because we need to spin the event loop for wayland QPA plugin
    // to handle registry_global_remove()
    QTRY_COMPARE(inputContextName(), mComposeModule);

    // add text input extension after the platform plugin has been loaded
    ensurePresentOnCompositor<TextInputManager>();
    // QTRY_* because we need to spin the event loop for wayland QPA plugin
    // to handle registry_global()
    QTRY_COMPARE(inputContextName(), mTextInputModule);
}

int main(int argc, char *argv[])
{
    QTemporaryDir tmpRuntimeDir;
    qputenv("XDG_RUNTIME_DIR", tmpRuntimeDir.path().toLocal8Bit());
    qputenv("QT_QPA_PLATFORM", "wayland");

    tst_inputcontext tc;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&tc, argc, argv);
}

#include "tst_inputcontext.moc"
