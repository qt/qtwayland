/****************************************************************************
**
** Copyright (C) 2021 LG Electronics Inc.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
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
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtQml/qqmlextensionplugin.h>
#include <QtQml/qqml.h>

#include <QtWaylandCompositor/qwaylandquickextension.h>
#include <QtWaylandCompositor/private/qwaylandpresentationtime_p.h>

QT_BEGIN_NAMESPACE

Q_COMPOSITOR_DECLARE_QUICK_EXTENSION_CLASS(QWaylandPresentationTime)

/*!
    \qmlmodule QtWayland.Compositor.PresentationTime
    \title Qt Wayland Presentation Time Extension
    \ingroup qmlmodules
    \since 6.3
    \brief Provides tracking the timing when a frame is presented on screen.

    \section2 Summary
    The PresentationTime extension provides a way to track rendering timing
    for a surface. Client can request feedbacks associated with a surface,
    then compositor send events for the feedback with the time when the surface
    is presented on-screen.

    PresentationTime corresponds to the Wayland \c wp_presentation interface.

    \section2 Usage
    To use this module, import it like this:
    \qml
    import QtWayland.Compositor.PresentationTime
    \endqml
*/

class QWaylandCompositorPresentationTimePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
public:
    void registerTypes(const char *uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("QtWayland.Compositor.PresentationTime"));
        defineModule(uri);
    }

    static void defineModule(const char *uri)
    {
        qmlRegisterModule(uri, QT_VERSION_MAJOR, QT_VERSION_MINOR);
        qmlRegisterType<QWaylandPresentationTime>(uri, 1, 0, "PresentationTime");
    }
};
QT_END_NAMESPACE

#include "qwaylandcompositorpresentationtimeplugin.moc"
