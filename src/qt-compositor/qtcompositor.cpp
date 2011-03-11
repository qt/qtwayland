/****************************************************************************
**
** This file is part of QtCompositor**
**
** Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
**
** Contact:  Nokia Corporation qt-info@nokia.com
**
** You may use this file under the terms of the BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
** names of its contributors may be used to endorse or promote products
** derived from this software without specific prior written permission.
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
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#include "qtcompositor.h"

#include "private/wlcompositor.h"
#include "private/wlsurface.h"

WaylandCompositor::WaylandCompositor(QWidget *topLevelWidget)
    : m_compositor(0)
    , m_toplevel_widget(topLevelWidget)
{
    m_compositor = new Wayland::Compositor(this);
}

WaylandCompositor::~WaylandCompositor()
{
    delete m_compositor;
}

void WaylandCompositor::frameFinished()
{
    m_compositor->frameFinished();
}

void WaylandCompositor::setInputFocus(uint winId)
{
    m_compositor->setInputFocus(winId);
}

void WaylandCompositor::sendMousePressEvent(uint winId, int x, int y, Qt::MouseButton button)
{
    m_compositor->sendMousePressEvent(winId, x, y, button);
}

void WaylandCompositor::sendMouseReleaseEvent(uint winId, int x, int y, Qt::MouseButton button)
{
    m_compositor->sendMouseReleaseEvent(winId, x, y, button);
}

void WaylandCompositor::sendMouseMoveEvent(uint winId, int x, int y)
{
    m_compositor->sendMouseMoveEvent(winId, x, y);
}

void WaylandCompositor::sendKeyPressEvent(uint winId, uint code)
{
    m_compositor->sendKeyPressEvent(winId, code);
}

void WaylandCompositor::sendKeyReleaseEvent(uint winId, uint code)
{
    m_compositor->sendKeyReleaseEvent(winId, code);
}

void WaylandCompositor::setDirectRenderWinId(uint winId)
{
    Q_UNUSED(winId);
}

uint WaylandCompositor::directRenderWinId() const
{
    return 0;
}

bool WaylandCompositor::hasImage(uint winId) const
{
    return m_compositor->getSurfaceFromWinId(winId)->hasImage();
}

const QImage WaylandCompositor::image(uint winId) const
{
    return m_compositor->image(winId);
}

#ifdef QT_COMPOSITOR_WAYLAND_GL
GLuint WaylandCompositor::textureId(uint winId) const
{
    return m_compositor->getSurfaceFromWinId(winId)->textureId();
}

bool WaylandCompositor::hasTexture(uint winId) const
{
    return m_compositor->getSurfaceFromWinId(winId)->hasTexture();
}
#endif

QWidget * WaylandCompositor::topLevelWidget() const
{
    return m_toplevel_widget;
}

Wayland::Compositor * WaylandCompositor::handle() const
{
    return m_compositor;
}

