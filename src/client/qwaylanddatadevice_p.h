/****************************************************************************
**
** Copyright (C) 2016 Klarälvdalens Datakonsult AB (KDAB).
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWaylandClient module of the Qt Toolkit.
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


#ifndef QWAYLANDDATADEVICE_H
#define QWAYLANDDATADEVICE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <qtwaylandclientglobal_p.h>
#include <QObject>
#include <QPointer>
#include <QPoint>

#include <QtWaylandClient/private/qwayland-wayland.h>

QT_REQUIRE_CONFIG(wayland_datadevice);

QT_BEGIN_NAMESPACE

class QMimeData;
class QWindow;

namespace QtWaylandClient {

class QWaylandDisplay;
class QWaylandDataDeviceManager;
class QWaylandDataOffer;
class QWaylandDataSource;
class QWaylandInputDevice;
class QWaylandWindow;

class QWaylandDataDevice : public QObject, public QtWayland::wl_data_device
{
    Q_OBJECT
public:
    QWaylandDataDevice(QWaylandDataDeviceManager *manager, QWaylandInputDevice *inputDevice);
    ~QWaylandDataDevice() override;

    QWaylandDataOffer *selectionOffer() const;
    void invalidateSelectionOffer();
    QWaylandDataSource *selectionSource() const;
    void setSelectionSource(QWaylandDataSource *source);

#if QT_CONFIG(draganddrop)
    QWaylandDataOffer *dragOffer() const;
    bool startDrag(QMimeData *mimeData, QWaylandWindow *icon);
    void cancelDrag();
#endif

protected:
    void data_device_data_offer(struct ::wl_data_offer *id) override;

#if QT_CONFIG(draganddrop)
    void data_device_drop() override;
    void data_device_enter(uint32_t serial, struct ::wl_surface *surface, wl_fixed_t x, wl_fixed_t y, struct ::wl_data_offer *id) override;
    void data_device_leave() override;
    void data_device_motion(uint32_t time, wl_fixed_t x, wl_fixed_t y) override;
#endif
    void data_device_selection(struct ::wl_data_offer *id) override;

private Q_SLOTS:
    void selectionSourceCancelled();

#if QT_CONFIG(draganddrop)
    void dragSourceCancelled();
    void dragSourceTargetChanged(const QString &mimeType);
#endif

private:
#if QT_CONFIG(draganddrop)
    QPoint calculateDragPosition(int x, int y, QWindow *wnd) const;
#endif

    QWaylandDisplay *m_display = nullptr;
    QWaylandInputDevice *m_inputDevice = nullptr;
    uint32_t m_enterSerial = 0;
    QPointer<QWindow> m_dragWindow;
    QPoint m_dragPoint;
    QScopedPointer<QWaylandDataOffer> m_dragOffer;
    QScopedPointer<QWaylandDataOffer> m_selectionOffer;
    QScopedPointer<QWaylandDataSource> m_selectionSource;

    QScopedPointer<QWaylandDataSource> m_dragSource;
};

}

QT_END_NAMESPACE

#endif // QWAYLANDDATADEVICE_H
