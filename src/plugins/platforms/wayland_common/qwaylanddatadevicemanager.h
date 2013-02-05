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

#ifndef QWAYLANDDATADEVICEMANAGER_H
#define QWAYLANDDATADEVICEMANAGER_H

#include "qwaylanddisplay.h"

#include <QtCore/QMap>
#include <QtCore/QMimeData>
#include <QtCore/QStringList>
#include <QtGui/QClipboard>

QT_BEGIN_NAMESPACE

class QWaylandDataOffer;
class QWaylandDataSource;
class QDrag;
class QWaylandShmBuffer;
class QWaylandWindow;

class QWaylandDataDeviceManager
{
public:
    QWaylandDataDeviceManager(QWaylandDisplay *display, uint32_t id);
    ~QWaylandDataDeviceManager();

    struct wl_data_device *getDataDevice(QWaylandInputDevice *inputDevice);

    QWaylandDataOffer *selectionTransfer() const;

    void createAndSetDrag(QDrag *drag);
    QMimeData *dragMime() const;
    bool canDropDrag() const;
    void cancelDrag();

    void createAndSetSelectionSource(QMimeData *mimeData, QClipboard::Mode mode);
    QWaylandDataSource *selectionTransferSource();

    QWaylandDisplay *display()const;

    struct wl_data_device_manager *handle() const;

private:
    struct wl_data_device_manager *m_data_device_manager;
    QWaylandDisplay *m_display;

    QWaylandDataOffer *m_selection_data_offer;
    QWaylandDataSource *m_selection_data_source;

    QWaylandDataOffer *m_drag_data_offer;
    QWaylandDataSource *m_drag_data_source;
    QWaylandWindow *m_drag_current_event_window;
    wl_surface *m_drag_surface;
    wl_surface *m_drag_icon_surface;
    QWaylandShmBuffer *m_drag_icon_buffer;
    bool m_drag_can_drop;
    uint32_t m_drag_last_event_time;
    QPoint m_drag_position;

    static void data_offer(void *data,
                       struct wl_data_device *wl_data_device,
                       struct wl_data_offer *id);
    static void enter(void *data,
                  struct wl_data_device *wl_data_device,
                  uint32_t time,
                  struct wl_surface *surface,
                  wl_fixed_t x,
                  wl_fixed_t y,
                  struct wl_data_offer *id);
    static void leave(void *data,
                  struct wl_data_device *wl_data_device);
    static void motion(void *data,
                   struct wl_data_device *wl_data_device,
                   uint32_t time,
                   wl_fixed_t x,
                   wl_fixed_t y);
    static void drop(void *data,
                 struct wl_data_device *wl_data_device);
    static void selection(void *data,
                      struct wl_data_device *wl_data_device,
                      struct wl_data_offer *id);

    static const struct wl_data_device_listener transfer_device_listener;
};

QT_END_NAMESPACE

#endif // QWAYLANDDATADEVICEMANAGER_H
