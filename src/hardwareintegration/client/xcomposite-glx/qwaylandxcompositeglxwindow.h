/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#ifndef QWAYLANDXCOMPOSITEGLXWINDOW_H
#define QWAYLANDXCOMPOSITEGLXWINDOW_H

#include <QtWaylandClient/private/qwaylandwindow_p.h>
#include "qwaylandxcompositeglxintegration.h"
#include "qwaylandxcompositeglxcontext.h"

#include <QtCore/QWaitCondition>

#include <QtWaylandClient/private/qwaylandbuffer_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandXCompositeGLXWindow : public QWaylandWindow
{
public:
    QWaylandXCompositeGLXWindow(QWindow *window, QWaylandXCompositeGLXIntegration *glxIntegration);
    WindowType windowType() const override;

    void setGeometry(const QRect &rect) override;

    Window xWindow() const;

    QWaylandBuffer *buffer() { return mBuffer; }

private:
    void createSurface();

    QWaylandXCompositeGLXIntegration *m_glxIntegration = nullptr;

    Window m_xWindow = 0;
    QWaylandBuffer *mBuffer = nullptr;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXCOMPOSITEGLXWINDOW_H
