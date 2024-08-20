// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandtextinputmanagerv3.h"
#include "qwaylandtextinputmanagerv3_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

#include "qwaylandtextinputv3.h"

QT_BEGIN_NAMESPACE

QWaylandTextInputManagerV3Private::QWaylandTextInputManagerV3Private()
{
}

void QWaylandTextInputManagerV3Private::zwp_text_input_manager_v3_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_Q(QWaylandTextInputManagerV3);
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    QWaylandTextInputV3 *textInput = QWaylandTextInputV3::findIn(seat);
    if (!textInput) {
        textInput = new QWaylandTextInputV3(seat, compositor);
    }
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    QWaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(QWaylandClient::TextInputProtocol::TextInputV3));
    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \internal
  \preliminary

  \qmltype TextInputManagerV3
  \nativetype QWaylandTextInputManagerV3
  \inqmlmodule QtWayland.Compositor
  \brief Provides access to input methods in the compositor.

  The \c TextInputManagerV3 corresponds to the \c zwp_text_input_manager_v3 interface
  in the \c text_input_unstable_v3 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.

  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

/*!
  \internal
  \preliminary
  \class QWaylandTextInputManagerV3
  \inmodule QtWaylandCompositor
  \brief Provides access to input methods in the compositor.

  The \c QWaylandTextInputManagerV3 corresponds to the \c zwp_text_input_manager_v3 interface
  in the \c text_input_unstable_v3 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
  \note This protocol is currently a work-in-progress and only exists in Qt for validation purposes. It may change at any time.
*/

QWaylandTextInputManagerV3::QWaylandTextInputManagerV3()
    : QWaylandCompositorExtensionTemplate<QWaylandTextInputManagerV3>(*new QWaylandTextInputManagerV3Private)
{
}

QWaylandTextInputManagerV3::QWaylandTextInputManagerV3(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandTextInputManagerV3>(compositor, *new QWaylandTextInputManagerV3Private)
{
}

QWaylandTextInputManagerV3::~QWaylandTextInputManagerV3()
{
}

void QWaylandTextInputManagerV3::initialize()
{
    qCDebug(qLcWaylandCompositorTextInput) << Q_FUNC_INFO;

    Q_D(QWaylandTextInputManagerV3);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandTextInputManagerV3";
        return;
    }
    d->init(compositor->display(), 1);
}

const wl_interface *QWaylandTextInputManagerV3::interface()
{
    return QWaylandTextInputManagerV3Private::interface();
}

QByteArray QWaylandTextInputManagerV3::interfaceName()
{
    return QWaylandTextInputManagerV3Private::interfaceName();
}

QT_END_NAMESPACE
