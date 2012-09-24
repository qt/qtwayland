/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the plugins of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "waylandsurfacetexturematerial.h"
#include <QtGui/QOpenGLContext>

static const char wayland_surface_texture_material_vertex[] =
           "uniform highp mat4 qt_Matrix;                      \n"
           "attribute highp vec4 qt_VertexPosition;            \n"
           "attribute highp vec2 qt_VertexTexCoord;            \n"
           "varying highp vec2 qt_TexCoord;                    \n"
           "void main() {                                      \n"
           "    qt_TexCoord = qt_VertexTexCoord;               \n"
           "    gl_Position = qt_Matrix * qt_VertexPosition;   \n"
           "}";


static const char wayland_surface_texture_opaque_material_fragment[] =
           "varying highp vec2 qt_TexCoord;                    \n"
           "uniform sampler2D qt_Texture;                      \n"
           "uniform lowp float qt_Opacity;                     \n"
           "void main() {                                      \n"
           "    gl_FragColor = vec4(texture2D(qt_Texture, qt_TexCoord).rgb, 1.0) * qt_Opacity; \n"
           "}";

static const char wayland_surface_texture_material_fragment[] =
           "varying highp vec2 qt_TexCoord;                    \n"
           "uniform sampler2D qt_Texture;                      \n"
           "uniform lowp float qt_Opacity;                     \n"
           "void main() {                                      \n"
           "    gl_FragColor = texture2D(qt_Texture, qt_TexCoord) * qt_Opacity; \n"
           "}";

QList<QByteArray> WaylandSurfaceTextureMaterial::attributes() const
{
    QList<QByteArray> attributeList;
    attributeList << "qt_VertexPosition";
    attributeList << "qt_VertexTexCoord";
    return attributeList;
}

void WaylandSurfaceTextureMaterial::updateState(const WaylandSurfaceTextureState *newState, const WaylandSurfaceTextureState *oldState)
{
    Q_UNUSED(oldState);
    newState->texture()->bind();
}

const char *WaylandSurfaceTextureMaterial::vertexShader() const
{
    return wayland_surface_texture_material_vertex;
}

const char *WaylandSurfaceTextureMaterial::fragmentShader() const
{
    return wayland_surface_texture_material_fragment;
}

QList<QByteArray> WaylandSurfaceTextureOpaqueMaterial::attributes() const
{
    QList<QByteArray> attributeList;
    attributeList << "qt_VertexPosition";
    attributeList << "qt_VertexTexCoord";
    return attributeList;
}

void WaylandSurfaceTextureOpaqueMaterial::updateState(const WaylandSurfaceTextureState *newState, const WaylandSurfaceTextureState *oldState)
{
    Q_UNUSED(oldState);
    newState->texture()->bind();
}

const char *WaylandSurfaceTextureOpaqueMaterial::vertexShader() const
{
    return wayland_surface_texture_material_vertex;
}

const char *WaylandSurfaceTextureOpaqueMaterial::fragmentShader() const
{
    return wayland_surface_texture_opaque_material_fragment;
}
