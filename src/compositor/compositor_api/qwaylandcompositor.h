/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandCompositor module of the Qt Toolkit.
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

#ifndef QWAYLANDCOMPOSITOR_H
#define QWAYLANDCOMPOSITOR_H

#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>
#include <QtWaylandCompositor/qwaylandcompositorextension.h>
#include <QtWaylandCompositor/QWaylandOutput>

#include <QObject>
#include <QImage>
#include <QRect>
#include <QLoggingCategory>

struct wl_display;

QT_BEGIN_NAMESPACE

class QInputEvent;

class QMimeData;
class QUrl;
class QOpenGLContext;
class QWaylandCompositorPrivate;
class QWaylandClient;
class QWaylandSurface;
class QWaylandSeat;
class QWaylandGlobalInterface;
class QWaylandView;
class QWaylandPointer;
class QWaylandKeyboard;
class QWaylandTouch;
class QWaylandSurfaceGrabber;
class QWaylandBufferRef;

Q_WAYLAND_COMPOSITOR_EXPORT Q_DECLARE_LOGGING_CATEGORY(qLcWaylandCompositor)
Q_WAYLAND_COMPOSITOR_EXPORT Q_DECLARE_LOGGING_CATEGORY(qLcWaylandCompositorHardwareIntegration)
Q_DECLARE_LOGGING_CATEGORY(qLcWaylandCompositorInputMethods)

class Q_WAYLAND_COMPOSITOR_EXPORT QWaylandCompositor : public QWaylandObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWaylandCompositor)
    Q_PROPERTY(QByteArray socketName READ socketName WRITE setSocketName NOTIFY socketNameChanged)
    Q_PROPERTY(bool created READ isCreated NOTIFY createdChanged)
    Q_PROPERTY(bool retainedSelection READ retainedSelectionEnabled WRITE setRetainedSelectionEnabled NOTIFY retainedSelectionChanged)
    Q_PROPERTY(QWaylandOutput *defaultOutput READ defaultOutput WRITE setDefaultOutput NOTIFY defaultOutputChanged)
    Q_PROPERTY(bool useHardwareIntegrationExtension READ useHardwareIntegrationExtension WRITE setUseHardwareIntegrationExtension NOTIFY useHardwareIntegrationExtensionChanged)
    Q_PROPERTY(QWaylandSeat *defaultSeat READ defaultSeat NOTIFY defaultSeatChanged)

public:
    QWaylandCompositor(QObject *parent = nullptr);
    ~QWaylandCompositor() override;

    virtual void create();
    bool isCreated() const;

    void setSocketName(const QByteArray &name);
    QByteArray socketName() const;

    Q_INVOKABLE void addSocketDescriptor(int fd);

    ::wl_display *display() const;
    uint32_t nextSerial();

    QList<QWaylandClient *>clients() const;
    Q_INVOKABLE void destroyClientForSurface(QWaylandSurface *surface);
    Q_INVOKABLE void destroyClient(QWaylandClient *client);

    QList<QWaylandSurface *> surfaces() const;
    QList<QWaylandSurface *> surfacesForClient(QWaylandClient* client) const;

    Q_INVOKABLE QWaylandOutput *outputFor(QWindow *window) const;

    QWaylandOutput *defaultOutput() const;
    void setDefaultOutput(QWaylandOutput *output);
    QList<QWaylandOutput *> outputs() const;

    uint currentTimeMsecs() const;

    void setRetainedSelectionEnabled(bool enabled);
    bool retainedSelectionEnabled() const;
    void overrideSelection(const QMimeData *data);

    QWaylandSeat *defaultSeat() const;

    QWaylandSeat *seatFor(QInputEvent *inputEvent);

    bool useHardwareIntegrationExtension() const;
    void setUseHardwareIntegrationExtension(bool use);

    virtual void grabSurface(QWaylandSurfaceGrabber *grabber, const QWaylandBufferRef &buffer);

public Q_SLOTS:
    void processWaylandEvents();

Q_SIGNALS:
    void createdChanged();
    void socketNameChanged(const QByteArray &socketName);
    void retainedSelectionChanged(bool retainedSelection);

    void surfaceRequested(QWaylandClient *client, uint id, int version);
    void surfaceCreated(QWaylandSurface *surface);
    void surfaceAboutToBeDestroyed(QWaylandSurface *surface);
    void subsurfaceChanged(QWaylandSurface *child, QWaylandSurface *parent);

    void defaultOutputChanged();
    void defaultSeatChanged(QWaylandSeat *newDevice, QWaylandSeat *oldDevice);

    void useHardwareIntegrationExtensionChanged();

    void outputAdded(QWaylandOutput *output);
    void outputRemoved(QWaylandOutput *output);

protected:
    virtual void retainedSelectionReceived(QMimeData *mimeData);
    virtual QWaylandSeat *createSeat();
    virtual QWaylandPointer *createPointerDevice(QWaylandSeat *seat);
    virtual QWaylandKeyboard *createKeyboardDevice(QWaylandSeat *seat);
    virtual QWaylandTouch *createTouchDevice(QWaylandSeat *seat);

    QWaylandCompositor(QWaylandCompositorPrivate &dptr, QObject *parent = nullptr);
};

QT_END_NAMESPACE

#endif // QWAYLANDCOMPOSITOR_H
