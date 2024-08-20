// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandqttextinputmethodmanager.h"
#include "qwaylandqttextinputmethodmanager_p.h"

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>

#include "qwaylandqttextinputmethod.h"

QT_BEGIN_NAMESPACE

QWaylandQtTextInputMethodManagerPrivate::QWaylandQtTextInputMethodManagerPrivate()
{
}

void QWaylandQtTextInputMethodManagerPrivate::text_input_method_manager_v1_get_text_input_method(Resource *resource, uint32_t id, struct ::wl_resource *seatResource)
{
    Q_Q(QWaylandQtTextInputMethodManager);
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(q->extensionContainer());
    QWaylandSeat *seat = QWaylandSeat::fromSeatResource(seatResource);
    QWaylandQtTextInputMethod *textInput = QWaylandQtTextInputMethod::findIn(seat);
    if (textInput == nullptr)
        textInput = new QWaylandQtTextInputMethod(seat, compositor);
    textInput->add(resource->client(), id, wl_resource_get_version(resource->handle));
    QWaylandClient *client = QWaylandClient::fromWlClient(compositor, resource->client());
    QWaylandClient::TextInputProtocols p = client->textInputProtocols();
    client->setTextInputProtocols(p.setFlag(QWaylandClient::TextInputProtocol::QtTextInputMethodV1));

    if (!textInput->isInitialized())
        textInput->initialize();
}

/*!
  \qmltype QtTextInputMethodManager
  \nativetype QWaylandQtTextInputMethodManager
  \inqmlmodule QtWayland.Compositor
  \since 6.0
  \brief Provides access to input methods in the compositor.

  The \c QtTextInputMethodManager corresponds to the \c qt-text-input-method-manager interface
  in the \c qt-text-input-method-unstable-v1 extension protocol. It is specifically designed
  to be used with a Qt-based input method, such as Qt Virtual Keyboard.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

/*!
   \class QWaylandQtTextInputMethodManager
   \inmodule QtWaylandCompositor
   \since 6.0
   \brief Provides access to input methods in the compositor.

   The \c QWaylandQtTextInputMethodManager class corresponds to the \c qt-text-input-method-manager interface
   in the \c qt-text-input-method-unstable-v1 extension protocol. It is specifically designed
   to be used with a Qt-based input method, such as Qt Virtual Keyboard.

  Instantiating this as child of a \l WaylandCompositor adds it to the list of interfaces available
  to the client. If a client binds to it, then it will be used to communciate text input to
  that client.
*/

QWaylandQtTextInputMethodManager::QWaylandQtTextInputMethodManager()
    : QWaylandCompositorExtensionTemplate<QWaylandQtTextInputMethodManager>(*new QWaylandQtTextInputMethodManagerPrivate)
{
}

QWaylandQtTextInputMethodManager::QWaylandQtTextInputMethodManager(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<QWaylandQtTextInputMethodManager>(compositor, *new QWaylandQtTextInputMethodManagerPrivate)
{
}

void QWaylandQtTextInputMethodManager::initialize()
{
    Q_D(QWaylandQtTextInputMethodManager);

    QWaylandCompositorExtensionTemplate::initialize();
    QWaylandCompositor *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    if (compositor == nullptr) {
        qWarning() << "Failed to find QWaylandCompositor when initializing QWaylandQtTextInputMethodManager";
        return;
    }

    d->init(compositor->display(), 1);
}

const wl_interface *QWaylandQtTextInputMethodManager::interface()
{
    return QWaylandQtTextInputMethodManagerPrivate::interface();
}

QByteArray QWaylandQtTextInputMethodManager::interfaceName()
{
    return QWaylandQtTextInputMethodManagerPrivate::interfaceName();
}

QT_END_NAMESPACE

#include "moc_qwaylandqttextinputmethodmanager.cpp"
