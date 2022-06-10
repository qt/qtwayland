// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

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
