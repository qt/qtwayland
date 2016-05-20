/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qwaylandquickitem.h"
#include "qwaylandquickitem_p.h"
#include "qwaylandquicksurface.h"
#include "qwaylandinputmethodcontrol.h"
#include "qwaylandtextinput.h"
#include <QtWaylandCompositor/qwaylandcompositor.h>
#include <QtWaylandCompositor/qwaylandinput.h>
#include <QtWaylandCompositor/qwaylandbufferref.h>
#include <QtWaylandCompositor/private/qwlclientbufferintegration_p.h>

#include <QtGui/QKeyEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QQuickWindow>

#include <QtCore/QMutexLocker>
#include <QtCore/QMutex>

#include <wayland-server.h>
#include <QThread>

#ifndef GL_TEXTURE_EXTERNAL_OES
#define GL_TEXTURE_EXTERNAL_OES 0x8D65
#endif

QT_BEGIN_NAMESPACE

static const struct {
    const char * const vertexShaderSourceFile;
    const char * const fragmentShaderSourceFile;
    GLenum textureTarget;
    int planeCount;
    bool canProvideTexture;
    QSGMaterial::Flags materialFlags;
    QSGMaterialType materialType;
} bufferTypes[] = {
    // BufferFormatEgl_Null
    { "", "", 0, 0, false, 0, {} },

    // BufferFormatEgl_RGB
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_rgbx.frag",
        GL_TEXTURE_2D, 1, true,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_RGBA
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_rgba.frag",
        GL_TEXTURE_2D, 1, true,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_EXTERNAL_OES
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_oes_external.frag",
        GL_TEXTURE_EXTERNAL_OES, 1, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_U_V
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_u_v.frag",
        GL_TEXTURE_2D, 3, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_UV
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_uv.frag",
        GL_TEXTURE_2D, 2, false,
        QSGMaterial::Blending,
        {}
    },

    // BufferFormatEgl_Y_XUXV
    {
        ":/qt-project.org/wayland/compositor/shaders/surface.vert",
        ":/qt-project.org/wayland/compositor/shaders/surface_y_xuxv.frag",
        GL_TEXTURE_2D, 2, false,
        QSGMaterial::Blending,
        {}
    }
};

QWaylandBufferMaterialShader::QWaylandBufferMaterialShader(QWaylandBufferRef::BufferFormatEgl format)
    : QSGMaterialShader()
    , m_format(format)
{
    setShaderSourceFile(QOpenGLShader::Vertex, QString::fromLatin1(bufferTypes[format].vertexShaderSourceFile));
    setShaderSourceFile(QOpenGLShader::Fragment, QString::fromLatin1(bufferTypes[format].fragmentShaderSourceFile));
}

void QWaylandBufferMaterialShader::updateState(const QSGMaterialShader::RenderState &state, QSGMaterial *newEffect, QSGMaterial *oldEffect)
{
    QSGMaterialShader::updateState(state, newEffect, oldEffect);

    QWaylandBufferMaterial *material = static_cast<QWaylandBufferMaterial *>(newEffect);
    material->bind();

    if (state.isMatrixDirty())
        program()->setUniformValue(m_id_matrix, state.combinedMatrix());

    if (state.isOpacityDirty())
        program()->setUniformValue(m_id_opacity, state.opacity());
}

const char * const *QWaylandBufferMaterialShader::attributeNames() const
{
    static char const *const attr[] = { "qt_VertexPosition", "qt_VertexTexCoord", 0 };
    return attr;
}

void QWaylandBufferMaterialShader::initialize()
{
    QSGMaterialShader::initialize();

    m_id_matrix = program()->uniformLocation("qt_Matrix");
    m_id_opacity = program()->uniformLocation("qt_Opacity");

    for (int i = 0; i < bufferTypes[m_format].planeCount; i++) {
        m_id_tex << program()->uniformLocation("tex" + QByteArray::number(i));
        program()->setUniformValue(m_id_tex[i], i);
    }

    Q_ASSERT(m_id_tex.size() == bufferTypes[m_format].planeCount);
}

QWaylandBufferMaterial::QWaylandBufferMaterial(QWaylandBufferRef::BufferFormatEgl format)
    : QSGMaterial()
    , m_format(format)
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    gl->glBindTexture(bufferTypes[m_format].textureTarget, 0);
    setFlag(bufferTypes[m_format].materialFlags);
}

QWaylandBufferMaterial::~QWaylandBufferMaterial()
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();

    for (GLuint texture : m_textures)
        gl->glDeleteTextures(1, &texture);
}

void QWaylandBufferMaterial::setTextureForPlane(int plane, uint texture)
{
    if (plane < 0 || plane >= bufferTypes[m_format].planeCount) {
        qWarning("plane index is out of range");
        return;
    }

    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    const GLenum target = bufferTypes[m_format].textureTarget;

    gl->glBindTexture(target, texture);
    setTextureParameters(target);

    ensureTextures(plane - 1);

    if (m_textures.size() <= plane) {
        m_textures << texture;
    } else {
        std::swap(m_textures[plane], texture);
        gl->glDeleteTextures(1, &texture);
    }
}

void QWaylandBufferMaterial::bind()
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    const GLenum target = bufferTypes[m_format].textureTarget;

    ensureTextures(bufferTypes[m_format].planeCount);

    switch (m_textures.size()) {
    case 3:
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(target, m_textures[2]);
    case 2:
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(target, m_textures[1]);
    case 1:
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(target, m_textures[0]);
    }
}

QSGMaterialType *QWaylandBufferMaterial::type() const
{
    return const_cast<QSGMaterialType *>(&bufferTypes[m_format].materialType);
}

QSGMaterialShader *QWaylandBufferMaterial::createShader() const
{
    return new QWaylandBufferMaterialShader(m_format);
}


void QWaylandBufferMaterial::setTextureParameters(GLenum target)
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    gl->glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    gl->glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//TODO move this into a separate centralized texture management class
void QWaylandBufferMaterial::ensureTextures(int count)
{
    QOpenGLFunctions *gl = QOpenGLContext::currentContext()->functions();
    const GLenum target = bufferTypes[m_format].textureTarget;
    GLuint texture;

    for (int plane = m_textures.size(); plane < count; plane++) {
        gl->glGenTextures(1, &texture);
        gl->glBindTexture(target, texture);
        setTextureParameters(target);
        m_textures << texture;
    }
}

QMutex *QWaylandQuickItemPrivate::mutex = 0;

class QWaylandSurfaceTextureProvider : public QSGTextureProvider
{
public:
    QWaylandSurfaceTextureProvider()
        : m_smooth(false)
        , m_sgTex(0)
    {
    }

    ~QWaylandSurfaceTextureProvider()
    {
        if (m_sgTex)
            m_sgTex->deleteLater();
    }

    void setBufferRef(QWaylandQuickItem *surfaceItem, const QWaylandBufferRef &buffer)
    {
        Q_ASSERT(QThread::currentThread() == thread());
        m_ref = buffer;
        delete m_sgTex;
        m_sgTex = 0;
        if (m_ref.hasBuffer()) {
            if (buffer.isShm()) {
                m_sgTex = surfaceItem->window()->createTextureFromImage(buffer.image());
                if (m_sgTex) {
                    m_sgTex->bind();
                }
            } else {
                QQuickWindow::CreateTextureOptions opt = QQuickWindow::TextureOwnsGLTexture;
                QWaylandQuickSurface *surface = qobject_cast<QWaylandQuickSurface *>(surfaceItem->surface());
                if (surface && surface->useTextureAlpha()) {
                    opt |= QQuickWindow::TextureHasAlphaChannel;
                }

                GLuint texture;
                glGenTextures(1, &texture);
                glBindTexture(GL_TEXTURE_2D, texture);
                buffer.bindToTexture();
                m_sgTex = surfaceItem->window()->createTextureFromId(texture , QSize(surfaceItem->width(), surfaceItem->height()), opt);
            }
        }
        emit textureChanged();
    }

    QSGTexture *texture() const Q_DECL_OVERRIDE
    {
        if (m_sgTex)
            m_sgTex->setFiltering(m_smooth ? QSGTexture::Linear : QSGTexture::Nearest);
        return m_sgTex;
    }

    void setSmooth(bool smooth) { m_smooth = smooth; }
private:
    bool m_smooth;
    QSGTexture *m_sgTex;
    QWaylandBufferRef m_ref;
};

/*!
 * \qmltype WaylandQuickItem
 * \inqmlmodule QtWayland.Compositor
 * \preliminary
 * \brief A Qt Quick item representing a WaylandView.
 *
 * When writing a WaylandCompositor in Qt Quick, this type can be used to display a
 * client's contents on an output device and will pass user input to the
 * client.
 */

/*!
 * \class QWaylandQuickItem
 * \inmodule QtWaylandCompositor
 * \preliminary
 * \brief A Qt Quick item representing a QWaylandView.
 *
 * When writing a QWaylandCompositor in Qt Quick, this class can be used to display a
 * client's contents on an output device and will pass user input to the
 * client.
 */

/*!
 * Constructs a QWaylandQuickItem with the given \a parent.
 */
QWaylandQuickItem::QWaylandQuickItem(QQuickItem *parent)
    : QQuickItem(*new QWaylandQuickItemPrivate(), parent)
{
    d_func()->init();
}

/*!
 * \internal
 */
QWaylandQuickItem::QWaylandQuickItem(QWaylandQuickItemPrivate &dd, QQuickItem *parent)
    : QQuickItem(dd, parent)
{
    d_func()->init();
}

/*!
 * Destroy the QWaylandQuickItem.
 */
QWaylandQuickItem::~QWaylandQuickItem()
{
    Q_D(QWaylandQuickItem);
    disconnect(this, &QQuickItem::windowChanged, this, &QWaylandQuickItem::updateWindow);
    QMutexLocker locker(d->mutex);
    if (d->provider)
        d->provider->deleteLater();
}

/*!
 * \qmlproperty object QtWaylandCompositor::WaylandQuickItem::compositor
 *
 * This property holds the compositor for the surface rendered by this WaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::compositor
 *
 * This property holds the compositor for the surface rendered by this QWaylandQuickItem.
 */
QWaylandCompositor *QWaylandQuickItem::compositor() const
{
    Q_D(const QWaylandQuickItem);
    return d->view->surface() ? d->view->surface()->compositor() : Q_NULLPTR;
}

/*!
 * \qmlproperty object QWaylandQuickItem::view
 *
 * This property holds the view rendered by this WaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::view
 *
 * This property holds the view rendered by this QWaylandQuickItem.
 */
QWaylandView *QWaylandQuickItem::view() const
{
    Q_D(const QWaylandQuickItem);
    return d->view.data();
}

/*!
 * \qmlproperty object QWaylandQuickItem::surface
 *
 * This property holds the surface rendered by this WaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::surface
 *
 * This property holds the surface rendered by this QWaylandQuickItem.
 */

QWaylandSurface *QWaylandQuickItem::surface() const
{
    Q_D(const QWaylandQuickItem);
    return d->view->surface();
}

void QWaylandQuickItem::setSurface(QWaylandSurface *surface)
{
    Q_D(QWaylandQuickItem);
    d->view->setSurface(surface);
    update();
}

/*!
 * \qmlproperty enum QtWaylandCompositor::WaylandQuickItem::origin
 *
 * This property holds the origin of the QWaylandQuickItem.
 */

/*!
 * \property QWaylandQuickItem::origin
 *
 * This property holds the origin of the QWaylandQuickItem.
 */
QWaylandSurface::Origin QWaylandQuickItem::origin() const
{
    Q_D(const QWaylandQuickItem);
    return d->origin;
}

bool QWaylandQuickItem::isTextureProvider() const
{
    Q_D(const QWaylandQuickItem);
    return QQuickItem::isTextureProvider() || d->provider;
}

/*!
 * Returns the texture provider of this QWaylandQuickItem.
 */
QSGTextureProvider *QWaylandQuickItem::textureProvider() const
{
    Q_D(const QWaylandQuickItem);

    if (QQuickItem::isTextureProvider())
        return QQuickItem::textureProvider();

    return d->provider;
}

/*!
 * \internal
 */
void QWaylandQuickItem::mousePressEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (!d->shouldSendInputEvents()) {
        event->ignore();
        return;
    }

    if (!inputRegionContains(event->pos())) {
        event->ignore();
        return;
    }

    QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);

    if (d->focusOnClick)
        takeFocus(inputDevice);

    inputDevice->sendMouseMoveEvent(d->view.data(), event->localPos() / d->scaleFactor(), event->windowPos());
    inputDevice->sendMousePressEvent(event->button());
}

/*!
 * \internal
 */
void QWaylandQuickItem::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(d->view.data(), event->localPos() / d->scaleFactor(), event->windowPos());
    } else {
        emit mouseMove(event->windowPos());
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseReleaseEvent(event->button());
    } else {
        emit mouseRelease();
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::hoverEnterEvent(QHoverEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (!inputRegionContains(event->pos())) {
        event->ignore();
        return;
    }
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(d->view.data(), event->pos(), mapToScene(event->pos()));
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::hoverMoveEvent(QHoverEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (surface()) {
        if (!inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }
    }
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseMoveEvent(d->view.data(), event->pos() / d->scaleFactor(), mapToScene(event->pos()));
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::hoverLeaveEvent(QHoverEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->setMouseFocus(Q_NULLPTR);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::wheelEvent(QWheelEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        if (!inputRegionContains(event->pos())) {
            event->ignore();
            return;
        }

        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendMouseWheelEvent(event->orientation(), event->delta());
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::keyPressEvent(QKeyEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::keyReleaseEvent(QKeyEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents() && hasFocus()) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);
        inputDevice->sendFullKeyEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::touchEvent(QTouchEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents() && d->touchEventsEnabled) {
        QWaylandInputDevice *inputDevice = compositor()->inputDeviceFor(event);

        if (event->type() == QEvent::TouchBegin) {
            QQuickItem *grabber = window()->mouseGrabberItem();
            if (grabber != this)
                grabMouse();
        }

        QPoint pointPos;
        const QList<QTouchEvent::TouchPoint> &points = event->touchPoints();
        if (!points.isEmpty())
            pointPos = points.at(0).pos().toPoint();

        if (event->type() == QEvent::TouchBegin && !inputRegionContains(pointPos)) {
            event->ignore();
            return;
        }

        event->accept();
        if (inputDevice->mouseFocus() != d->view.data()) {
            inputDevice->sendMouseMoveEvent(d->view.data(), pointPos, mapToScene(pointPos));
        }
        inputDevice->sendFullTouchEvent(event);
    } else {
        event->ignore();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::mouseUngrabEvent()
{
    if (surface()) {
        QTouchEvent e(QEvent::TouchCancel);
        touchEvent(&e);
    }
}

#ifndef QT_NO_IM
/*!
 * \internal
 */
void QWaylandQuickItem::inputMethodEvent(QInputMethodEvent *event)
{
    Q_D(QWaylandQuickItem);
    if (d->shouldSendInputEvents()) {
        d->oldSurface->inputMethodControl()->inputMethodEvent(event);
    } else {
        event->ignore();
    }
}
#endif

/*!
 * \internal
 */
void QWaylandQuickItem::surfaceChangedEvent(QWaylandSurface *newSurface, QWaylandSurface *oldSurface)
{
    Q_UNUSED(newSurface);
    Q_UNUSED(oldSurface);
}

void QWaylandQuickItem::handleSubsurfaceAdded(QWaylandSurface *childSurface)
{
    Q_D(QWaylandQuickItem);
    if (d->subsurfaceHandler.isNull()) {
        QWaylandQuickItem *childItem = new QWaylandQuickItem;
        childItem->setSurface(childSurface);
        childItem->setVisible(true);
        childItem->setParentItem(this);
        connect(childSurface, &QWaylandSurface::subsurfacePositionChanged, childItem, &QWaylandQuickItem::handleSubsurfacePosition);
    } else {
        bool success = QMetaObject::invokeMethod(d->subsurfaceHandler, "handleSubsurfaceAdded", Q_ARG(QWaylandSurface *, childSurface));
        if (!success)
            qWarning("QWaylandQuickItem: subsurfaceHandler does not implement handleSubsurfaceAdded()");
    }
}



/*!
  \qmlproperty bool QtWaylandCompositor::WaylandQuickItem::subsurfaceHandler

  This property provides a way to override the default subsurface behavior.

  By default, Qt will create a new SurfaceItem as a child of this item, and maintain the correct position.

  To override the default, assign a handler object to this property. The handler should implement
  a handleSubsurfaceAdded(WaylandSurface) function.

  \code
  ShellSurfaceItem {
      subsurfaceHandler: QtObject {
      function handleSubsurfaceAdded(child) {
        //create custom surface item, and connect the subsurfacePositionChanged signal
      }
  }
  \endcode

  The default value of this property is \c null.
 */


QObject *QWaylandQuickItem::subsurfaceHandler() const
{
    Q_D(const QWaylandQuickItem);
    return d->subsurfaceHandler.data();
}

void QWaylandQuickItem::setSubsurfaceHandler(QObject *handler)
{
    Q_D(QWaylandQuickItem);
    if (d->subsurfaceHandler.data() != handler) {
        d->subsurfaceHandler = handler;
        emit subsurfaceHandlerChanged();
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::handleSurfaceChanged()
{
    Q_D(QWaylandQuickItem);
    if (d->oldSurface) {
        disconnect(d->oldSurface, &QWaylandSurface::mappedChanged, this, &QWaylandQuickItem::surfaceMappedChanged);
        disconnect(d->oldSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickItem::parentChanged);
        disconnect(d->oldSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickItem::updateSize);
        disconnect(d->oldSurface, &QWaylandSurface::bufferScaleChanged, this, &QWaylandQuickItem::updateSize);
        disconnect(d->oldSurface, &QWaylandSurface::configure, this, &QWaylandQuickItem::updateBuffer);
        disconnect(d->oldSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
        disconnect(d->oldSurface, &QWaylandSurface::childAdded, this, &QWaylandQuickItem::handleSubsurfaceAdded);
#ifndef QT_NO_IM
        disconnect(d->oldSurface->inputMethodControl(), &QWaylandInputMethodControl::updateInputMethod, this, &QWaylandQuickItem::updateInputMethod);
#endif
    }
    if (QWaylandSurface *newSurface = d->view->surface()) {
        connect(newSurface, &QWaylandSurface::mappedChanged, this, &QWaylandQuickItem::surfaceMappedChanged);
        connect(newSurface, &QWaylandSurface::parentChanged, this, &QWaylandQuickItem::parentChanged);
        connect(newSurface, &QWaylandSurface::sizeChanged, this, &QWaylandQuickItem::updateSize);
        connect(newSurface, &QWaylandSurface::bufferScaleChanged, this, &QWaylandQuickItem::updateSize);
        connect(newSurface, &QWaylandSurface::configure, this, &QWaylandQuickItem::updateBuffer);
        connect(newSurface, &QWaylandSurface::redraw, this, &QQuickItem::update);
        connect(newSurface, &QWaylandSurface::childAdded, this, &QWaylandQuickItem::handleSubsurfaceAdded);
#ifndef QT_NO_IM
        connect(newSurface->inputMethodControl(), &QWaylandInputMethodControl::updateInputMethod, this, &QWaylandQuickItem::updateInputMethod);
#endif

        if (newSurface->origin() != d->origin) {
            d->origin = newSurface->origin();
            emit originChanged();
        }
        if (window()) {
            QWaylandOutput *output = newSurface->compositor()->outputFor(window());
            d->view->setOutput(output);
        }

        updateSize();
    }
    surfaceChangedEvent(d->view->surface(), d->oldSurface);
    d->oldSurface = d->view->surface();
#ifndef QT_NO_IM
    updateInputMethod(Qt::ImQueryInput);
#endif
}

/*!
 * Calling this function causes the item to take the focus of the
 * input \a device.
 */
void QWaylandQuickItem::takeFocus(QWaylandInputDevice *device)
{
    forceActiveFocus();

    if (!surface())
        return;

    QWaylandInputDevice *target = device;
    if (!target) {
        target = compositor()->defaultInputDevice();
    }
    target->setKeyboardFocus(surface());
    QWaylandTextInput *textInput = QWaylandTextInput::findIn(target);
    if (textInput)
        textInput->setFocus(surface());
}

/*!
 * \internal
 */
void QWaylandQuickItem::surfaceMappedChanged()
{
    update();
}

/*!
 * \internal
 */
void QWaylandQuickItem::parentChanged(QWaylandSurface *newParent, QWaylandSurface *oldParent)
{
    Q_UNUSED(oldParent);

    if (newParent) {
        setPaintEnabled(true);
        setVisible(true);
        setOpacity(1);
        setEnabled(true);
    }
}

/*!
 * \internal
 */
void QWaylandQuickItem::updateSize()
{
    Q_D(QWaylandQuickItem);
    if (d->sizeFollowsSurface && surface()) {
        setSize(surface()->size() * (d->scaleFactor() / surface()->bufferScale()));
    }
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandQuickItem::focusOnClick
 *
 * This property specifies whether the WaylandQuickItem should take focus when
 * it is clicked.
 *
 * The default is true.
 */

/*!
 * \property QWaylandQuickItem::focusOnClick
 *
 * This property specifies whether the QWaylandQuickItem should take focus when
 * it is clicked.
 *
 * The default is true.
 */
bool QWaylandQuickItem::focusOnClick() const
{
    Q_D(const QWaylandQuickItem);
    return d->focusOnClick;
}

void QWaylandQuickItem::setFocusOnClick(bool focus)
{
    Q_D(QWaylandQuickItem);
    if (d->focusOnClick == focus)
        return;

    d->focusOnClick = focus;
    emit focusOnClickChanged();
}

/*!
 * Returns true if the input region of this item's surface contains the
 * position given by \a localPosition.
 */
bool QWaylandQuickItem::inputRegionContains(const QPointF &localPosition)
{
    Q_D(QWaylandQuickItem);
    if (QWaylandSurface *s = surface())
        return s->inputRegionContains(localPosition.toPoint() / d->scaleFactor());
    return false;
}

/*!
 * \qmlproperty bool QtWaylandCompositor::WaylandQuickItem::sizeFollowsSurface
 *
 * This property specifies whether the size of the item should always match
 * the size of its surface.
 *
 * The default is true.
 */

/*!
 * \property QWaylandQuickItem::sizeFollowsSurface
 *
 * This property specifies whether the size of the item should always match
 * the size of its surface.
 *
 * The default is true.
 */
bool QWaylandQuickItem::sizeFollowsSurface() const
{
    Q_D(const QWaylandQuickItem);
    return d->sizeFollowsSurface;
}

void QWaylandQuickItem::setSizeFollowsSurface(bool sizeFollowsSurface)
{
    Q_D(QWaylandQuickItem);
    if (d->sizeFollowsSurface == sizeFollowsSurface)
        return;
    d->sizeFollowsSurface = sizeFollowsSurface;
    emit sizeFollowsSurfaceChanged();
}

#ifndef QT_NO_IM
QVariant QWaylandQuickItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    return inputMethodQuery(query, QVariant());
}

QVariant QWaylandQuickItem::inputMethodQuery(Qt::InputMethodQuery query, QVariant argument) const
{
    Q_D(const QWaylandQuickItem);

    if (query == Qt::ImEnabled)
        return QVariant((flags() & ItemAcceptsInputMethod) != 0);

    if (d->oldSurface)
        return d->oldSurface->inputMethodControl()->inputMethodQuery(query, argument);

    return QVariant();
}
#endif

/*!
    \qmlproperty bool QtWayland::QWaylandSurfaceItem::paintEnabled

    If this property is true, the \l item is hidden, though the texture
    will still be updated. As opposed to hiding the \l item by
    setting \l{Item::visible}{visible} to false, setting this property to true
    will not prevent mouse or keyboard input from reaching \l item.
*/
bool QWaylandQuickItem::paintEnabled() const
{
    Q_D(const QWaylandQuickItem);
    return d->paintEnabled;
}

void QWaylandQuickItem::setPaintEnabled(bool enabled)
{
    Q_D(QWaylandQuickItem);
    d->paintEnabled = enabled;
    update();
}

bool QWaylandQuickItem::touchEventsEnabled() const
{
    Q_D(const QWaylandQuickItem);
    return d->touchEventsEnabled;
}

void QWaylandQuickItem::updateBuffer(bool hasBuffer)
{
    Q_D(QWaylandQuickItem);
    Q_UNUSED(hasBuffer);
    if (d->origin != surface()->origin()) {
        d->origin = surface()->origin();
        emit originChanged();
    }
}

void QWaylandQuickItem::updateWindow()
{
    Q_D(QWaylandQuickItem);
    if (d->connectedWindow) {
        disconnect(d->connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickItem::beforeSync);
    }

    d->connectedWindow = window();

    if (d->connectedWindow) {
        connect(d->connectedWindow, &QQuickWindow::beforeSynchronizing, this, &QWaylandQuickItem::beforeSync, Qt::DirectConnection);
    }

    if (compositor() && d->connectedWindow) {
        QWaylandOutput *output = compositor()->outputFor(d->connectedWindow);
        Q_ASSERT(output);
        d->view->setOutput(output);
    }
}

void QWaylandQuickItem::beforeSync()
{
    Q_D(QWaylandQuickItem);
    if (d->view->advance()) {
        d->newTexture = true;
        update();
    }
}

#ifndef QT_NO_IM
void QWaylandQuickItem::updateInputMethod(Qt::InputMethodQueries queries)
{
    Q_D(QWaylandQuickItem);

    setFlag(QQuickItem::ItemAcceptsInputMethod,
            d->oldSurface ? d->oldSurface->inputMethodControl()->enabled() : false);
    QQuickItem::updateInputMethod(queries | Qt::ImEnabled);
}
#endif

QSGNode *QWaylandQuickItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    Q_D(QWaylandQuickItem);
    const bool mapped = surface() && surface()->isMapped() && d->view->currentBuffer().hasBuffer();

    if (!mapped || !d->paintEnabled) {
        delete oldNode;
        return 0;
    }

    QWaylandBufferRef ref = d->view->currentBuffer();
    const bool invertY = ref.origin() == QWaylandSurface::OriginBottomLeft;
    const QRectF rect = invertY ? QRectF(0, height(), width(), -height())
                                : QRectF(0, 0, width(), height());

    if (ref.isShm() || bufferTypes[ref.bufferFormatEgl()].canProvideTexture) {
        QSGSimpleTextureNode *node = static_cast<QSGSimpleTextureNode *>(oldNode);

        if (!node)
            node = new QSGSimpleTextureNode();

        if (!d->provider)
            d->provider = new QWaylandSurfaceTextureProvider();

        if (d->newTexture) {
            d->newTexture = false;
            d->provider->setBufferRef(this, ref);
            node->setTexture(d->provider->texture());
        }

        d->provider->setSmooth(smooth());
        node->setRect(rect);

        return node;
    } else {
        Q_ASSERT(!d->provider);

        QSGGeometryNode *node = static_cast<QSGGeometryNode *>(oldNode);

        if (!node)
            node = new QSGGeometryNode;

        QSGGeometry *geometry = node->geometry();
        QWaylandBufferMaterial *material = static_cast<QWaylandBufferMaterial *>(node->material());

        if (!geometry)
            geometry = new QSGGeometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4);

        if (!material)
            material = new QWaylandBufferMaterial(ref.bufferFormatEgl());

        if (d->newTexture) {
            d->newTexture = false;
            for (int plane = 0; plane < bufferTypes[ref.bufferFormatEgl()].planeCount; plane++)
                if (uint texture = ref.textureForPlane(plane))
                    material->setTextureForPlane(plane, texture);
            material->bind();
            ref.bindToTexture();
        }

        ref.updateTexture();
        QSGGeometry::updateTexturedRectGeometry(geometry, rect, QRectF(0, 0, 1, 1));

        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry, true);

        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial, true);

        return node;
    }

    Q_UNREACHABLE();
}

void QWaylandQuickItem::setTouchEventsEnabled(bool enabled)
{
    Q_D(QWaylandQuickItem);
    if (d->touchEventsEnabled != enabled) {
        d->touchEventsEnabled = enabled;
        emit touchEventsEnabledChanged();
    }
}

bool QWaylandQuickItem::inputEventsEnabled() const
{
    Q_D(const QWaylandQuickItem);
    return d->inputEventsEnabled;
}

void QWaylandQuickItem::setInputEventsEnabled(bool enabled)
{
    Q_D(QWaylandQuickItem);
    if (d->inputEventsEnabled != enabled) {
        d->setInputEventsEnabled(enabled);
        emit inputEventsEnabledChanged();
    }
}

void QWaylandQuickItem::lower()
{
    QQuickItem *parent = parentItem();
    Q_ASSERT(parent);
    QQuickItem *bottom = parent->childItems().first();
    if (this != bottom)
        stackBefore(bottom);
}

void QWaylandQuickItem::raise()
{
    QQuickItem *parent = parentItem();
    Q_ASSERT(parent);
    QQuickItem *top = parent->childItems().last();
    if (this != top)
        stackAfter(top);
}

/*!
 * \internal
 *
 * Sets the position of this item relative to the parent item.
 */
void QWaylandQuickItem::handleSubsurfacePosition(const QPoint &pos)
{
    Q_D(QWaylandQuickItem);
    QQuickItem::setPosition(pos * d->scaleFactor());
}

QT_END_NAMESPACE

