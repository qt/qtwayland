/****************************************************************************
**
** Copyright (C) 2014 Pier Luigi Fiorini <pierluigi.fiorini@gmail.com>
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Compositor.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QWAYLANDCLIENT_H
#define QWAYLANDCLIENT_H

#include <QtCompositor/qwaylandexport.h>

#include <QObject>

#include <signal.h>

struct wl_client;

QT_BEGIN_NAMESPACE

class QWaylandClientPrivate;

class Q_COMPOSITOR_EXPORT QWaylandClient : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandClient)

    Q_PROPERTY(qint64 userId READ userId CONSTANT)
    Q_PROPERTY(qint64 groupId READ groupId CONSTANT)
    Q_PROPERTY(qint64 processId READ processId CONSTANT)
public:
    ~QWaylandClient();

    static QWaylandClient *fromWlClient(wl_client *wlClient);

    wl_client *client() const;

    qint64 userId() const;
    qint64 groupId() const;

    qint64 processId() const;

    Q_INVOKABLE void kill(int sig = SIGTERM);

public Q_SLOTS:
    void close();

private:
    explicit QWaylandClient(wl_client *client);
};

QT_END_NAMESPACE

#endif // QWAYLANDCLIENT_H
