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
****************************************************************************/

#ifndef VSP2HARDWARELAYERINTEGRATION_H
#define VSP2HARDWARELAYERINTEGRATION_H

#include <QtWaylandCompositor/private/qwlhardwarelayerintegration_p.h>
#include <private/qobject_p.h>

#include <QPoint>
#include <QSize>

struct wl_kms_buffer;

QT_BEGIN_NAMESPACE

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
    QVector<QSharedPointer<Vsp2Layer>> m_layers;
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

public slots:
    void handleBufferCommitted();
    void handleSurfaceChanged();
    void updatePosition();
    void updateOpacity();

private:
    wl_kms_buffer *nextKmsBuffer();
    int m_layerIndex = -1;
    QScreen *m_screen = nullptr;
    QPoint m_position;
    QWaylandQuickHardwareLayer *m_hwLayer = nullptr;
    QWaylandSurface *m_surface = nullptr;
    Vsp2Buffer m_buffer;
};

QT_END_NAMESPACE

#endif // VSP2HARDWARELAYERINTEGRATION_H
