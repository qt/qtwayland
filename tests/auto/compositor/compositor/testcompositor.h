// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwaylandcompositor.h"
#include "qwaylandsurface.h"
#include "qwaylandwlshell.h"

class TestCompositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    TestCompositor(bool createInputDev = false);
    void create() override;
    void flushClients();

public slots:
    void onSurfaceCreated(QWaylandSurface *surface);
    void onSurfaceAboutToBeDestroyed(QWaylandSurface *surface);

protected:
    QWaylandSeat *createSeat() override;
    QWaylandKeyboard *createKeyboardDevice(QWaylandSeat *seat) override;

public:
    QList<QWaylandSurface *> surfaces;
    QWaylandWlShell* shell;
    bool m_createSeat;
};

