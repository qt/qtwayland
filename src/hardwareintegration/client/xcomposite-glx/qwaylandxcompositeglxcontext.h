/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QWAYLANDXCOMPOSITEGLXCONTEXT_H
#define QWAYLANDXCOMPOSITEGLXCONTEXT_H

#include <qpa/qplatformopenglcontext.h>

#include "qwaylandxcompositeglxintegration.h"
#include <QtGui/private/qglxconvenience_p.h>

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandXCompositeGLXWindow;
class QWaylandShmBuffer;

class QWaylandXCompositeGLXContext : public QPlatformOpenGLContext
{
public:
    QWaylandXCompositeGLXContext(const QSurfaceFormat &format, QPlatformOpenGLContext *share, Display *display, int screen);

    QSurfaceFormat format() const override;

    void swapBuffers(QPlatformSurface *surface) override;

    bool makeCurrent(QPlatformSurface *surface) override;
    void doneCurrent() override;

    QFunctionPointer getProcAddress(const char *procName) override;

private:
    GLXContext m_context;

    Display *m_display = nullptr;
    QSurfaceFormat m_format;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDXCOMPOSITEGLXCONTEXT_H
