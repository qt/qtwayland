// Copyright (C) 2018 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef VSP2HARDWARELAYERINTEGRATION_H
#define VSP2HARDWARELAYERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlhardwarelayerintegration_p.h>
#include <private/qobject_p.h>

#include <QPoint>
#include <QSize>

struct wl_kms_buffer;

QT_BEGIN_NAMESPACE

namespace QNativeInterface::Private {
struct QVsp2Screen;
}

class QScreen;
class QWaylandSurface;
class QWaylandQuickHardwareLayer;

class Vsp2Layer;

class Vsp2HardwareLayerIntegration : public QtWayland::HardwareLayerIntegration
{
    Q_OBJECT
public:
    explicit Vsp2HardwareLayerIntegration();

    void add(QWaylandQuickHardwareLayer *layer) override;
    void remove(QWaylandQuickHardwareLayer *layer) override;

    void sendFrameCallbacks();
    QList<QSharedPointer<Vsp2Layer>> m_layers;
private:
    void enableVspLayers();
    void disableVspLayers();
    void sortLayersByDepth();
    void recreateVspLayers();
    friend class Vsp2Layer;
};

struct Vsp2Buffer
{
    explicit Vsp2Buffer() = default;
    explicit Vsp2Buffer(wl_kms_buffer *kmsBuffer);

    int dmabufFd = -1;
    uint bytesPerLine = 0;
    uint drmPixelFormat = 0;
    QSize size;
};

class Vsp2Layer : public QObject
{
    Q_OBJECT
public:
    explicit Vsp2Layer(QWaylandQuickHardwareLayer *m_hwLayer, Vsp2HardwareLayerIntegration *integration);
    void enableVspLayer();
    void disableVspLayer();
    bool isEnabled() { return m_layerIndex != -1; }
    QWaylandQuickHardwareLayer *hwLayer() const { return m_hwLayer; }

public Q_SLOTS:
    void handleBufferCommitted();
    void handleSurfaceChanged();
    void updatePosition();
    void updateOpacity();

private:
    wl_kms_buffer *nextKmsBuffer();
    int m_layerIndex = -1;
    QVsp2Screen *m_screen = nullptr;
    QPoint m_position;
    QWaylandQuickHardwareLayer *m_hwLayer = nullptr;
    QWaylandSurface *m_surface = nullptr;
    Vsp2Buffer m_buffer;
};

QT_END_NAMESPACE

#endif // VSP2HARDWARELAYERINTEGRATION_H
