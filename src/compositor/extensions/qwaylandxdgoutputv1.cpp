/****************************************************************************
**
** Copyright (C) 2019 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
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

#include <QWaylandCompositor>

#include "qwaylandxdgoutputv1_p.h"
#include "qwaylandoutput_p.h"

#include <wayland-server.h>

QT_BEGIN_NAMESPACE

/*!
 * \qmltype XdgOutputManagerV1
 * \inqmlmodule QtWayland.Compositor
 * \since 5.14
 * \brief Provides an extension for describing outputs in a desktop oriented fashion.
 *
 * The XdgOutputManagerV1 extension provides a way for a compositor to describe outputs in a way
 * that is more in line with the concept of an output on desktop oriented systems.
 *
 * Some information may not make sense in other applications such as IVI systems.
 *
 * Typically the global compositor space on a desktop system is made of a
 * contiguous or overlapping set of rectangular regions.
 *
 * XdgOutputManagerV1 corresponds to the Wayland interface, \c zxdg_output_manager_v1.
 *
 * To provide the functionality of the extension in a compositor, create an instance of the
 * XdgOutputManagerV1 component and add it to the list of extensions supported by the compositor,
 * and associated each XdgOutputV1 with its WaylandOutput:
 *
 * \qml \QtMinorVersion
 * import QtWayland.Compositor 1.\1
 *
 * WaylandCompositor {
 *     XdgOutputManagerV1 {
 *         WaylandOutput {
 *             id: output1
 *
 *             position: Qt.point(0, 0)
 *             window: Window {}
 *
 *             XdgOutputV1 {
 *                 name: "WL-1"
 *                 logicalPosition: output1.position
 *                 logicalSize: Qt.size(output1.geometry.width / output1.scaleFactor,
 *                                      output1.geometry.height / output1.scaleFactor)
 *             }
 *         }
 *
 *         WaylandOutput {
 *             id: output2
 *
 *             position: Qt.point(800, 0)
 *             window: Window {}
 *
 *             XdgOutputV1 {
 *                 name: "WL-2"
 *                 logicalPosition: output2.position
 *                 logicalSize: Qt.size(output2.geometry.width / output2.scaleFactor,
 *                                      output2.geometry.height / output2.scaleFactor)
 *             }
 *         }
 *     }
 * }
 * \endqml
 */

/*!
 * \class QWaylandXdgOutputManagerV1
 * \inmodule QtWaylandCompositor
 * \since 5.14
 * \brief Provides an extension for describing outputs in a desktop oriented fashion.
 *
 * The QWaylandXdgOutputManagerV1 extension provides a way for a compositor to describe outputs in a way
 * that is more in line with the concept of an output on desktop oriented systems.
 *
 * Some information may not make sense in other applications such as IVI systems.
 *
 * QWaylandXdgOutputManagerV1 corresponds to the Wayland interface, \c zxdg_output_manager_v1.
 */

/*!
 * Constructs a QWaylandXdgOutputManagerV1 object.
 */
QWaylandXdgOutputManagerV1::QWaylandXdgOutputManagerV1()
    : QWaylandCompositorExtensionTemplate<QWaylandXdgOutputManagerV1>(*new QWaylandXdgOutputManagerV1Private())
{
}

/*!
 * Constructs a QWaylandXdgOutputManagerV1 object for the provided \a compositor.
 */
QWaylandXdgOutputManagerV1::QWaylandXdgOutputManagerV1(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandXdgOutputManagerV1>(compositor, *new QWaylandXdgOutputManagerV1Private())
{
}

// QWaylandXdgOutputManagerV1Private

/*!
 * Initializes the extension.
 */
void QWaylandXdgOutputManagerV1::initialize()
{
    Q_D(QWaylandXdgOutputManagerV1);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qCWarning(qLcWaylandCompositor) << "Failed to find QWaylandCompositor when initializing QWaylandXdgOutputManagerV1";
        return;
    }
    d->init(compositor->display(), d->interfaceVersion());
}

/*!
 * Returns the Wayland interface for QWaylandXdgOutputManagerV1.
 */
const wl_interface *QWaylandXdgOutputManagerV1::interface()
{
    return QWaylandXdgOutputManagerV1Private::interface();
}

// QWaylandXdgOutputManagerV1Private

void QWaylandXdgOutputManagerV1Private::registerXdgOutput(QWaylandOutput *output, QWaylandXdgOutputV1 *xdgOutput)
{
    if (!xdgOutputs.contains(output)) {
        xdgOutputs[output] = xdgOutput;
        QWaylandOutputPrivate::get(output)->xdgOutput = xdgOutput;
    }
}

void QWaylandXdgOutputManagerV1Private::unregisterXdgOutput(QWaylandOutput *output)
{
    xdgOutputs.remove(output);
}

void QWaylandXdgOutputManagerV1Private::zxdg_output_manager_v1_get_xdg_output(Resource *resource,
                                                                              uint32_t id,
                                                                              wl_resource *outputResource)
{
    Q_Q(QWaylandXdgOutputManagerV1);

    // Verify if the associated output exist
    auto *output = QWaylandOutput::fromResource(outputResource);
    if (!output) {
        qCWarning(qLcWaylandCompositor,
                  "The client is requesting a QWaylandXdgOutputV1 for a "
                  "QWaylandOutput that doesn't exist");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT, "output not found");
        return;
    }

    // Do we have a QWaylandXdgOutputV1 for this output?
    if (!xdgOutputs.contains(output)) {
        qCWarning(qLcWaylandCompositor,
                  "The client is requesting a QWaylandXdgOutputV1 that the compositor "
                  "didn't create before");
        wl_resource_post_error(resource->handle, WL_DISPLAY_ERROR_INVALID_OBJECT,
                               "compositor didn't create a QWaylandXdgOutputV1 for this zxdg_output_v1 object");
        return;
    }

    // Bind QWaylandXdgOutputV1 and initialize
    auto *xdgOutput = xdgOutputs[output];
    auto *xdgOutputPrivate = QWaylandXdgOutputV1Private::get(xdgOutput);
    Q_ASSERT(xdgOutputPrivate);
    xdgOutputPrivate->setManager(q);
    xdgOutputPrivate->setOutput(output);
    xdgOutputPrivate->add(resource->client(), id, qMin(resource->version(), QWaylandXdgOutputV1Private::interfaceVersion()));
}

// QWaylandXdgOutputV1

QWaylandXdgOutputV1::QWaylandXdgOutputV1()
    : QObject(*new QWaylandXdgOutputV1Private)
{
}

QWaylandXdgOutputV1::QWaylandXdgOutputV1(QWaylandOutput *output, QWaylandXdgOutputManagerV1 *manager)
    : QObject(*new QWaylandXdgOutputV1Private)
{
    Q_D(QWaylandXdgOutputV1);

    // Set members before emitting changed signals so that handlers will
    // see both already set and not nullptr, avoiding potential crashes
    d->manager = manager;
    d->output = output;

    QWaylandXdgOutputManagerV1Private::get(d->manager)->registerXdgOutput(output, this);

    emit managerChanged();
    emit outputChanged();
}

QWaylandXdgOutputV1::~QWaylandXdgOutputV1()
{
    Q_D(QWaylandXdgOutputV1);

    if (d->manager)
        QWaylandXdgOutputManagerV1Private::get(d->manager)->unregisterXdgOutput(d->output);
}

/*!
 * \qmlproperty XdgOutputManagerV1 QtWaylandCompositor::XdgOutputV1::manager
 * \readonly
 *
 * This property holds the object that manages this XdgOutputV1.
 */
/*!
 * \property QWaylandXdgOutputV1::manager
 * \readonly
 *
 * This property holds the object that manages this QWaylandXdgOutputV1.
 */
QWaylandXdgOutputManagerV1 *QWaylandXdgOutputV1::manager() const
{
    Q_D(const QWaylandXdgOutputV1);
    return d->manager;
}

/*!
 * \qmlproperty WaylandOutput QtWaylandCompositor::XdgOutputV1::output
 * \readonly
 *
 * This property holds the WaylandOutput associated with this XdgOutputV1.
 */
/*!
 * \property QWaylandXdgOutputV1::output
 * \readonly
 *
 * This property holds the QWaylandOutput associated with this QWaylandXdgOutputV1.
 */
QWaylandOutput *QWaylandXdgOutputV1::output() const
{
    Q_D(const QWaylandXdgOutputV1);
    return d->output;
}

/*!
 * \qmlproperty string QtWaylandCompositor::XdgOutputV1::name
 *
 * This property holds the name of this output.
 *
 * The naming convention is compositor defined, but limited to alphanumeric
 * characters and dashes ("-").  Each name is unique and will also remain
 * consistent across sessions with the same hardware and software configuration.
 *
 * Examples of names include "HDMI-A-1", "WL-1", "X11-1" etc...
 * However don't assume the name reflects the underlying technology.
 *
 * Changing this property after initialization doesn't take effect.
 */
/*!
 * \property QWaylandXdgOutputV1::name
 *
 * This property holds the name of this output.
 *
 * The naming convention is compositor defined, but limited to alphanumeric
 * characters and dashes ("-").  Each name is unique and will also remain
 * consistent across sessions with the same hardware and software configuration.
 *
 * Examples of names include "HDMI-A-1", "WL-1", "X11-1" etc...
 * However don't assume the name reflects the underlying technology.
 *
 * Changing this property after initialization doesn't take effect.
 */
QString QWaylandXdgOutputV1::name() const
{
    Q_D(const QWaylandXdgOutputV1);
    return d->name;
}

void QWaylandXdgOutputV1::setName(const QString &name)
{
    Q_D(QWaylandXdgOutputV1);

    if (d->name == name)
        return;

    // Can't change after clients bound to xdg-output
    if (d->initialized) {
        qCWarning(qLcWaylandCompositor, "QWaylandXdgOutputV1::name cannot be changed after initialization");
        return;
    }

    d->name = name;
    emit nameChanged();
}

/*!
 *  \qmlproperty string QtWaylandCompositor::XdgOutputV1::description
 *
 *  This property holds the description of this output.
 *
 *  No convention is defined for the description.
 *
 * Changing this property after initialization doesn't take effect.
 */
/*!
 * \property QWaylandXdgOutputV1::description
 *
 *  This property holds the description of this output.
 *
 *  No convention is defined for the description.
 *
 * Changing this property after initialization doesn't take effect.
 */
QString QWaylandXdgOutputV1::description() const
{
    Q_D(const QWaylandXdgOutputV1);
    return d->description;
}

void QWaylandXdgOutputV1::setDescription(const QString &description)
{
    Q_D(QWaylandXdgOutputV1);

    if (d->description == description)
        return;

    // Can't change after clients bound to xdg-output
    if (d->initialized) {
        qCWarning(qLcWaylandCompositor, "QWaylandXdgOutputV1::description cannot be changed after initialization");
        return;
    }

    d->description = description;
    emit descriptionChanged();
}

/*!
 * \qmlproperty point QtWaylandCompositor::XdgOutputV1::logicalPosition
 *
 * This property holds the coordinates of the output within the global compositor space.
 *
 * The default value is 0,0.
 */
/*!
 * \property QWaylandXdgOutputV1::logicalPosition
 *
 * This property holds the coordinates of the output within the global compositor space.
 *
 * The default value is 0,0.
 */
QPoint QWaylandXdgOutputV1::logicalPosition() const
{
    Q_D(const QWaylandXdgOutputV1);
    return d->logicalPos;
}

void QWaylandXdgOutputV1::setLogicalPosition(const QPoint &position)
{
    Q_D(QWaylandXdgOutputV1);

    if (d->logicalPos == position)
        return;

    d->logicalPos = position;
    if (d->initialized) {
        d->sendLogicalPosition(position);
        d->sendDone();
    }
    emit logicalPositionChanged();
    emit logicalGeometryChanged();
}

/*!
 * \qmlproperty size QtWaylandCompositor::XdgOutputV1::logicalSize
 *
 * This property holds the size of the output in the global compositor space.
 *
 * The default value is -1,-1 which is invalid.
 *
 * Please remember that this is the logical size, not the physical size.
 * For example, for a WaylandOutput mode 3840x2160 and a scale factor 2:
 * \list
 * \li A compositor not scaling the surface buffers, will report a logical size of 3840x2160.
 * \li A compositor automatically scaling the surface buffers, will report a logical size of 1920x1080.
 * \li A compositor using a fractional scale of 1.5, will report a logical size of 2560x1620.
 * \endlist
 */
/*!
 * \property QWaylandXdgOutputV1::logicalSize
 *
 * This property holds the size of the output in the global compositor space.
 *
 * The default value is -1,-1 which is invalid.
 *
 * Please remember that this is the logical size, not the physical size.
 * For example, for a WaylandOutput mode 3840x2160 and a scale factor 2:
 * \list
 * \li A compositor not scaling the surface buffers, will report a logical size of 3840x2160.
 * \li A compositor automatically scaling the surface buffers, will report a logical size of 1920x1080.
 * \li A compositor using a fractional scale of 1.5, will report a logical size of 2560x1620.
 * \endlist
 */
QSize QWaylandXdgOutputV1::logicalSize() const
{
    Q_D(const QWaylandXdgOutputV1);
    return d->logicalSize;
}

void QWaylandXdgOutputV1::setLogicalSize(const QSize &size)
{
    Q_D(QWaylandXdgOutputV1);

    if (d->logicalSize == size)
        return;

    d->logicalSize = size;
    if (d->initialized) {
        d->sendLogicalSize(size);
        d->sendDone();
    }
    emit logicalSizeChanged();
    emit logicalGeometryChanged();
}

/*!
 * \qmlproperty rect QtWaylandCompositor::XdgOutputV1::logicalGeometry
 * \readonly
 *
 * This property holds the position and size of the output in the global compositor space.
 * It's the combination of the logical position and logical size.
 *
 * \sa XdgOutputV1::logicalPosition
 * \sa XdgOutputV1::logicalSize
 */
/*!
 * \property QWaylandXdgOutputV1::logicalGeometry
 * \readonly
 *
 * This property holds the position and size of the output in the global compositor space.
 * It's the combination of the logical position and logical size.
 *
 * \sa QWaylandXdgOutputV1::logicalPosition
 * \sa QWaylandXdgOutputV1::logicalSize
 */
QRect QWaylandXdgOutputV1::logicalGeometry() const
{
    Q_D(const QWaylandXdgOutputV1);
    return QRect(d->logicalPos, d->logicalSize);
}

// QWaylandXdgOutputV1Private

void QWaylandXdgOutputV1Private::sendLogicalPosition(const QPoint &position)
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_logical_position(resource->handle, position.x(), position.y());
    needToSendDone = true;
}

void QWaylandXdgOutputV1Private::sendLogicalSize(const QSize &size)
{
    const auto values = resourceMap().values();
    for (auto *resource : values)
        send_logical_size(resource->handle, size.width(), size.height());
    needToSendDone = true;
}

void QWaylandXdgOutputV1Private::sendDone()
{
    if (needToSendDone) {
        const auto values = resourceMap().values();
        for (auto *resource : values) {
            if (resource->version() < 3)
                send_done(resource->handle);
        }
        needToSendDone = false;
    }
}

void QWaylandXdgOutputV1Private::setManager(QWaylandXdgOutputManagerV1 *_manager)
{
    Q_Q(QWaylandXdgOutputV1);

    if (!_manager) {
        qCWarning(qLcWaylandCompositor,
                  "Cannot associate a null QWaylandXdgOutputManagerV1 to QWaylandXdgOutputV1 %p", this);
        return;
    }

    if (manager == _manager)
        return;

    if (manager) {
        qCWarning(qLcWaylandCompositor,
                  "Cannot associate a different QWaylandXdgOutputManagerV1 to QWaylandXdgOutputV1 %p "
                  "after initialization", this);
        return;
    }

    manager = _manager;
    emit q->managerChanged();
}

void QWaylandXdgOutputV1Private::setOutput(QWaylandOutput *_output)
{
    Q_Q(QWaylandXdgOutputV1);

    if (!_output) {
        qCWarning(qLcWaylandCompositor,
                  "Cannot associate a null QWaylandOutput to QWaylandXdgOutputV1 %p", this);
        return;
    }

    if (output == _output)
        return;

    if (output) {
        qCWarning(qLcWaylandCompositor,
                  "Cannot associate a different QWaylandOutput to QWaylandXdgOutputV1 %p "
                  "after initialization", this);
        return;
    }

    // Assign output above manager, to make both values valid in handlers
    output = _output;

    if (!manager) {
        // Try to find the manager from the output parents
        for (auto *p = output->parent(); p != nullptr; p = p->parent()) {
            if (auto *m = qobject_cast<QWaylandXdgOutputManagerV1 *>(p)) {
                manager = m;
                emit q->managerChanged();
                break;
            }
        }
    }

    emit q->outputChanged();

    // Register the output
    if (manager)
        QWaylandXdgOutputManagerV1Private::get(manager)->registerXdgOutput(output, q);
}

void QWaylandXdgOutputV1Private::zxdg_output_v1_bind_resource(Resource *resource)
{
    send_logical_position(resource->handle, logicalPos.x(), logicalPos.y());
    send_logical_size(resource->handle, logicalSize.width(), logicalSize.height());
    if (resource->version() >= ZXDG_OUTPUT_V1_NAME_SINCE_VERSION)
        send_name(resource->handle, name);
    if (resource->version() >= ZXDG_OUTPUT_V1_DESCRIPTION_SINCE_VERSION)
        send_description(resource->handle, description);
    send_done(resource->handle);

    initialized = true;
}

void QWaylandXdgOutputV1Private::zxdg_output_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

QT_END_NAMESPACE
