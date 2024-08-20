// Copyright (C) 2017-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandtextinputmanager.h"
#include "qwaylandtextinputmanager_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

#include "qwaylandtextinput.h"

QT_BEGIN_NAMESPACE

QWaylandTextInputManagerPrivate::QWaylandTextInputManagerPrivate()
{
}

void QWaylandTextInputManagerPrivate::zwp_text_input_manager_v2_get_text_input(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    Q_Q(QWaylandTextInputManager);
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    QWaylandTextInput *textInput = QWaylandTextInput::findIn(seat);
    if (!textInput)
        textInput = new QWaylandTextInput(seat, compositor);
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    QWaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(QWaylandClient::TextInputProtocol::TextInputV2));
    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \qmltype TextInputManager
  \nativetype QWaylandTextInputManager
  \inqmlmodule QtWayland.Compositor
  \brief Provides access to input methods in the compositor.

  The \c TextInputManager corresponds to the \c zwp_text_input_manager_v2 interface
  in the \c text_input_unstable_v2 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

/*!
  \class QWaylandTextInputManager
  \inmodule QtWaylandCompositor
  \brief Provides access to input methods in the compositor.

  The \c QWaylandTextInputManager corresponds to the \c zwp_text_input_manager_v2 interface
  in the \c text_input_unstable_v2 extension protocol.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

QWaylandTextInputManager::QWaylandTextInputManager()
    : QWaylandCompositorExtensionTemplate<QWaylandTextInputManager>(*new QWaylandTextInputManagerPrivate)
{
}

QWaylandTextInputManager::QWaylandTextInputManager(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandTextInputManager>(compositor, *new QWaylandTextInputManagerPrivate)
{
}

void QWaylandTextInputManager::initialize()
{
    Q_D(QWaylandTextInputManager);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (!compositor) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandTextInputManager";
        return;
    }
    d->init(compositor->display(), 1);
}

const wl_interface *QWaylandTextInputManager::interface()
{
    return QWaylandTextInputManagerPrivate::interface();
}

QByteArray QWaylandTextInputManager::interfaceName()
{
    return QWaylandTextInputManagerPrivate::interfaceName();
}

QT_END_NAMESPACE

#include "moc_qwaylandtextinputmanager.cpp"
