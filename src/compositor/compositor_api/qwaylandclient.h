// Copyright (C) 2017 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDCLIENT_H
#define QWAYLANDCLIENT_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qtwaylandqmlinclude.h>

#include <QtCore/QObject>

#include <signal.h>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandClientPrivate;
class QWaylandCompositor;

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandClient : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandClient)

    Q_PROPERTY(QWaylandCompositor *compositor READ compositor CONSTANT)
    Q_PROPERTY(qint64 userId READ userId CONSTANT)
    Q_PROPERTY(qint64 groupId READ groupId CONSTANT)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)
    Q_MOC_INCLUDE("qwaylandcompositor.h")

    QML_NAMED_ELEMENT(WaylandClient)
    QML_ADDED_IN_VERSION(1, 0)
    QML_UNCREATABLE("")
public:
    ~QWaylandClient() override;

    enum TextInputProtocol {
        NoProtocol = 0,
        QtTextInputMethodV1 = 1,
        TextInputV2 = 2,
        TextInputV4 = 4,

        QtTextInputMethod = QtTextInputMethodV1,
        TextInput = TextInputV2
    };
    Q_DECLARE_FLAGS(TextInputProtocols, TextInputProtocol)

    TextInputProtocols textInputProtocols() const;
    void setTextInputProtocols(TextInputProtocols p);

    static QWaylandClient *fromWlClient(QWaylandCompositor *compositor, wl_client *wlClient);

    QWaylandCompositor *compositor() const;

    wl_client *client() const;

    qint64 userId() const;
    qint64 groupId() const;

    qint64 processId() const;

    Q_INVOKABLE void kill(int signal = SIGTERM);

public Q_SLOTS:
    void close();

private:
    explicit QWaylandClient(QWaylandCompositor *compositor, wl_client *client);
};

QT_END_NAMESPACE

#endif // QWAYLANDCLIENT_H
