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
#include <QtGui/QScreen>

#include <qwayland-server-xdg-output-unstable-v1.h>

using namespace MockCompositor;

// TODO: move to shared folder?
class XdgOutputV1 : public QObject, public QtWaylandServer::zxdg_output_v1
{
public:
    explicit XdgOutputV1(Output *output)
        : m_output(output)
        , m_logicalGeometry(m_output->m_data.position, QSize(m_output->m_data.mode.resolution / m_output->m_data.scale))
        , m_name(QString("WL-%1").arg(s_nextId++))
    {}

    void addResource(wl_client *client, int id, int version)
    {
        auto *resource = add(client, id, version)->handle;
        send_logical_size(resource, m_logicalGeometry.width(), m_logicalGeometry.height());
        send_logical_position(resource, m_logicalGeometry.x(), m_logicalGeometry.y());
        if (version >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION)
            send_name(resource, m_name);
        if (version >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION)
            send_description(resource, m_description);
        send_done(resource);
    }
    Output *m_output = nullptr;
    QRect m_logicalGeometry;
    QString m_name;
    QString m_description = "This is an Xdg Output description";
    static int s_nextId;
};

int XdgOutputV1::s_nextId = 1;

class XdgOutputManagerV1 : public Global, public QtWaylandServer::zxdg_output_manager_v1
{
    Q_OBJECT
public:
    explicit XdgOutputManagerV1(CoreCompositor *compositor, int version = 2)
        : QtWaylandServer::zxdg_output_manager_v1(compositor->m_display, version)
        , m_version(version)
    {}
    int m_version = 1; // TODO: remove on libwayland upgrade
    QMap<Output *, XdgOutputV1 *> m_xdgOutputs;
    XdgOutputV1 *getXdgOutput(Output *output)
    {
        if (auto *xdgOutput = m_xdgOutputs.value(output))
            return xdgOutput;
        return m_xdgOutputs[output] = new XdgOutputV1(output); // TODO: free memory
    }

protected:
    void zxdg_output_manager_v1_get_xdg_output(Resource *resource, uint32_t id, wl_resource *outputResource) override
    {
        auto *output = fromResource<Output>(outputResource);
        auto *xdgOutput = getXdgOutput(output);
        xdgOutput->addResource(resource->client(), id, resource->version());
    }
};

class XdgOutputV1Compositor : public DefaultCompositor {
public:
    explicit XdgOutputV1Compositor()
    {
        exec([this] {
            int version = 2; // version 2 of of unstable-v1
            add<XdgOutputManagerV1>(version);
        });
    }
    XdgOutputV1 *xdgOutput(int i = 0) { return get<XdgOutputManagerV1>()->getXdgOutput(output(i)); }
};

class tst_xdgoutput : public QObject, private XdgOutputV1Compositor
{
    Q_OBJECT
private slots:
    void cleanup();
    void primaryScreen();
    void overrideGeometry();
};

void tst_xdgoutput::cleanup()
{
    QCOMPOSITOR_COMPARE(getAll<Output>().size(), 1); // Only the default output should be left
    QTRY_VERIFY2(isClean(), qPrintable(dirtyMessage()));
}

void tst_xdgoutput::primaryScreen()
{
    // Verify that the client has bound to the global
    QCOMPOSITOR_TRY_COMPARE(get<XdgOutputManagerV1>()->resourceMap().size(), 1);
    exec([=] {
        auto *resource = xdgOutput()->resourceMap().value(client());
        QCOMPARE(resource->version(), 2);
    });
    auto *s = QGuiApplication::primaryScreen();
    QTRY_COMPARE(s->size(), QSize(1920, 1080));
    QTRY_COMPARE(s->geometry().topLeft(), QPoint(0, 0));
    QTRY_COMPARE(s->name(), QString("WL-1"));
}

void tst_xdgoutput::overrideGeometry()
{
    exec([=] {
        auto *output = add<Output>();
        auto *xdgOutput = get<XdgOutputManagerV1>()->getXdgOutput(output);
        xdgOutput->m_logicalGeometry = QRect(10, 20, 800, 1200);
    });

    QTRY_COMPARE(QGuiApplication::screens().size(), 2);
    auto *s = QGuiApplication::screens()[1];

    QTRY_COMPARE(s->size(), QSize(800, 1200));
    QTRY_COMPARE(s->geometry().topLeft(), QPoint(10, 20));

    exec([=] { remove(output(1)); });
}

QCOMPOSITOR_TEST_MAIN(tst_xdgoutput)
#include "tst_xdgoutput.moc"
