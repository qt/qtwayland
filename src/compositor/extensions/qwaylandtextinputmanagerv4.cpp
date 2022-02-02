/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandtextinputmanagerv4.h"
#include "qwaylandtextinputmanagerv4_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

#include "qwaylandtextinputv4.h"

QT_BEGIN_NAMESPACE

QWaylandTextInputManagerV4Private::QWaylandTextInputManagerV4Private()
{
}

void QWaylandTextInputManagerV4Private::zwp_text_input_manager_v4_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(QWaylandTextInputManagerV4);
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    QWaylandTextInputV4 *textInput = QWaylandTextInputV4::findIn(seat);
    if (!textInput) {
        textInput = new QWaylandTextInputV4(seat, compositor);
    }
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    QWaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(QWaylandClient::TextInputProtocol::TextInputV4));
    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \internal
  \preliminary

  \qmltype TextInputManagerV4
  \instantiates QWaylandTextInputManagerV4
  \inqmlmodule QtWayland.Compositor
  \brief Provides access to input methods in the compositor.

  The \c TextInputManagerV4 corresponds to the \c zwp_text_input_manager_v4 interface
  in the \c text_input_unstable_v4 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.

  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

/*!
  \internal
  \preliminary
  \class QWaylandTextInputManagerV4
  \inmodule QtWaylandCompositor
  \brief Provides access to input methods in the compositor.

  The \c QWaylandTextInputManagerV4 corresponds to the \c zwp_text_input_manager_v4 interface
  in the \c text_input_unstable_v4 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

QWaylandTextInputManagerV4::QWaylandTextInputManagerV4()
    : QWaylandCompositorExtensionTemplate<QWaylandTextInputManagerV4>(*new QWaylandTextInputManagerV4Private)
{
}

QWaylandTextInputManagerV4::QWaylandTextInputManagerV4(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandTextInputManagerV4>(compositor, *new QWaylandTextInputManagerV4Private)
{
}

QWaylandTextInputManagerV4::~QWaylandTextInputManagerV4()
{
}

void QWaylandTextInputManagerV4::initialize()
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_D(QWaylandTextInputManagerV4);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandTextInputManagerV4";
        return;
    }
    d->init(compositor->display(), 1);
}

const wl_interface *QWaylandTextInputManagerV4::interface()
{
    return QWaylandTextInputManagerV4Private::interface();
}

QByteArray QWaylandTextInputManagerV4::interfaceName()
{
    return QWaylandTextInputManagerV4Private::interfaceName();
}

QT_END_NAMESPACE
