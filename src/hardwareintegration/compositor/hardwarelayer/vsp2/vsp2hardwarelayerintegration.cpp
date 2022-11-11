// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "vsp2hardwarelayerintegration.h"

extern "C" {
#define private priv
#include <wayland-kms.h>
#undef private
}

#include <private/qwaylandquickhardwarelayer_p.h>
#include <private/qwaylandquickitem_p.h>
#include <private/qwaylandview_p.h>
#include <QWaylandQuickOutput>
#include <QQuickWindow>

#include <qpa/qlatformscreen_p.h>

using namespace QNativeInterface::Private;

QT_BEGIN_NAMESPACE

Vsp2Buffer::Vsp2Buffer(wl_kms_buffer *kmsBuffer)
    : dmabufFd(kmsBuffer->fd)
    , bytesPerLine(kmsBuffer->stride)
    , drmPixelFormat(kmsBuffer->format)
    , size(kmsBuffer->width, kmsBuffer->height)
{
}

Vsp2Layer::Vsp2Layer(QWaylandQuickHardwareLayer *hwLayer, Vsp2HardwareLayerIntegration *integration)
    : m_hwLayer(hwLayer)
{
    auto *wlItem = m_hwLayer->waylandItem();
    m_screen = dynamic_cast<QVsp2Screen*>(wlItem->window()->screen()->handle());
    Q_ASSERT(m_screen);

    connect(hwLayer, &QWaylandQuickHardwareLayer::stackingLevelChanged, this, [integration](){
        integration->recreateVspLayers();
    });
    connect(hwLayer->waylandItem(), &QWaylandQuickItem::surfaceChanged, this, &Vsp2Layer::handleSurfaceChanged);
    connect(hwLayer->waylandItem(), &QQuickItem::opacityChanged, this, &Vsp2Layer::updateOpacity);
    connect(hwLayer->waylandItem()->window(), &QQuickWindow::afterSynchronizing, this, &Vsp2Layer::updatePosition);
    hwLayer->setSceneGraphPainting(false);
    QWaylandViewPrivate::get(hwLayer->waylandItem()->view())->independentFrameCallback = true;
    handleSurfaceChanged();
}

void Vsp2Layer::enableVspLayer()
{
    auto *kmsBuffer = nextKmsBuffer();

    if (!kmsBuffer)
        return;

    m_buffer = Vsp2Buffer(kmsBuffer);
    updatePosition();

    m_layerIndex = m_screen->addLayer(m_buffer.dmabufFd, m_buffer.size, m_position, m_buffer.drmPixelFormat, m_buffer.bytesPerLine);

    auto *wlItem = m_hwLayer->waylandItem();
    wlItem->surface()->frameStarted();
    updateOpacity();
}

void Vsp2Layer::disableVspLayer()
{
    m_screen->removeLayer(m_layerIndex);
    m_layerIndex = -1;
    m_screen = nullptr;
}

void Vsp2Layer::handleBufferCommitted()
{
    if (!isEnabled()) {
        enableVspLayer();
        return;
    }

    auto *kmsBuffer = nextKmsBuffer();

    Vsp2Buffer newBuffer(kmsBuffer);
    if (m_buffer.dmabufFd != -1) {
        bool formatChanged = false;
        formatChanged |= newBuffer.bytesPerLine != m_buffer.bytesPerLine;
        formatChanged |= newBuffer.size != m_buffer.size;
        formatChanged |= newBuffer.drmPixelFormat != m_buffer.drmPixelFormat;
        if (formatChanged) {
            qWarning() << "The VSP2 Wayland hardware layer integration doesn't support changing"
                       << "surface formats, this will most likely fail";
        }
    }

    m_buffer = newBuffer;
    m_screen->setLayerBuffer(m_layerIndex, m_buffer.dmabufFd);

    auto *wlItem = m_hwLayer->waylandItem();
    wlItem->surface()->frameStarted();
}

void Vsp2Layer::handleSurfaceChanged()
{
    auto newSurface = m_hwLayer->waylandItem()->surface();

    if (Q_UNLIKELY(newSurface == m_surface))
        return;

    if (this->m_surface)
        disconnect(this->m_surface, &QWaylandSurface::redraw, this, &Vsp2Layer::handleBufferCommitted);
    if (newSurface)
        connect(newSurface, &QWaylandSurface::redraw, this, &Vsp2Layer::handleBufferCommitted, Qt::DirectConnection);

    this->m_surface = newSurface;
}

void Vsp2Layer::updatePosition()
{
    QWaylandQuickItem *wlItem = m_hwLayer->waylandItem();
    QRectF localGeometry(0, 0, wlItem->width(), wlItem->height());
    auto lastMatrix = QWaylandQuickItemPrivate::get(wlItem)->lastMatrix;
    auto globalGeometry = lastMatrix.mapRect(localGeometry);

    if (m_buffer.size != globalGeometry.size().toSize()) {
        qWarning() << "wl_buffer size != WaylandQuickItem size and scaling has not been"
                   << "implemented for the vsp2 hardware layer integration";
    }

    m_position = globalGeometry.topLeft().toPoint();
    if (isEnabled())
        m_screen->setLayerPosition(m_layerIndex, m_position);
}

void Vsp2Layer::updateOpacity()
{
    if (isEnabled()) {
        qreal opacity = m_hwLayer->waylandItem()->opacity();
        m_screen->setLayerAlpha(m_layerIndex, opacity);
    }
}

wl_kms_buffer *Vsp2Layer::nextKmsBuffer()
{
    Q_ASSERT(m_hwLayer && m_hwLayer->waylandItem());
    QWaylandQuickItem *wlItem = m_hwLayer->waylandItem();
    auto view = wlItem->view();
    Q_ASSERT(view);

    view->advance();
    auto wlBuffer = view->currentBuffer().wl_buffer();

    if (!wlBuffer)
        return nullptr;

    struct wl_kms_buffer *kmsBuffer = wayland_kms_buffer_get(wlBuffer);

    if (!kmsBuffer)
        qWarning() << "Failed to get wl_kms_buffer for wl_buffer:" << wl_resource_get_id(wlBuffer);

    return kmsBuffer;
}

void Vsp2HardwareLayerIntegration::enableVspLayers()
{
    for (auto &layer : std::as_const(m_layers)) {
        Q_ASSERT(!layer->isEnabled());
        layer->enableVspLayer();
    }
}

void Vsp2HardwareLayerIntegration::disableVspLayers()
{
    for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it) {
        if ((*it)->isEnabled())
            (*it)->disableVspLayer();
    }
}

void Vsp2HardwareLayerIntegration::sortLayersByDepth()
{
    std::sort(m_layers.begin(), m_layers.end(), [](auto &l1, auto &l2){
        return l1->hwLayer()->stackingLevel() < l2->hwLayer()->stackingLevel();
    });
}

void Vsp2HardwareLayerIntegration::recreateVspLayers() {
    disableVspLayers();
    sortLayersByDepth();
    enableVspLayers();
}

Vsp2HardwareLayerIntegration::Vsp2HardwareLayerIntegration()
{
    if (QGuiApplication::platformName() != "eglfs") {
        qWarning() << "Vsp2 layers are currently only supported on the eglfs platform plugin"
                   << "with the eglfs_kms_vsp2 device integration.\n"
                   << "You need to set QT_QPA_PLATFORM=eglfs and QT_QPA_EGLFS_INTEGRATION=eglfs_kms_vsp2";
    }
    static Vsp2HardwareLayerIntegration *s_instance = this;
    auto screen = dynamic_cast<QVsp2Screen*>(QGuiApplication::primaryScreen()->handle());
    screen->addBlendListener([](){
        s_instance->sendFrameCallbacks();
    });
}

void Vsp2HardwareLayerIntegration::add(QWaylandQuickHardwareLayer *hwLayer)
{
    disableVspLayers();
    m_layers.append(QSharedPointer<Vsp2Layer>(new Vsp2Layer(hwLayer, this)));
    sortLayersByDepth();
    enableVspLayers();
}

void Vsp2HardwareLayerIntegration::remove(QWaylandQuickHardwareLayer *hwLayer)
{
    disableVspLayers();
    for (auto it = m_layers.begin(); it != m_layers.end(); ++it) {
        if ((*it)->hwLayer() == hwLayer) {
            m_layers.erase(it);
            break;
        }
    }
    enableVspLayers();
}

void Vsp2HardwareLayerIntegration::sendFrameCallbacks()
{
    for (auto &layer : std::as_const(m_layers)) {
        if (auto *surface = layer->hwLayer()->waylandItem()->surface())
            surface->sendFrameCallbacks();
    }
}

QT_END_NAMESPACE
