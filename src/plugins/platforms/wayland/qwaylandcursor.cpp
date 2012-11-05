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

#include "qwaylandcursor.h"

#include "qwaylanddisplay.h"
#include "qwaylandinputdevice.h"
#include "qwaylandscreen.h"
#include "qwaylandshmbackingstore.h"

#include <QtGui/QImageReader>
#include <QDebug>

#include <wayland-cursor.h>

#define ARRAY_LENGTH(a) sizeof(a) / sizeof(a[0])

/*
 * The following correspondences between file names and cursors was copied
 * from: https://bugs.kde.org/attachment.cgi?id=67313
 */

static const char *bottom_left_corners[] = {
    "size_fdiag",
    "bottom_left_corner",
    "sw-resize"
};

static const char *bottom_right_corners[] = {
    "size_bdiag",
    "bottom_right_corner",
    "se-resize"
};

static const char *bottom_sides[] = {
    "bottom_side",
    "s-resize"
};

static const char *grabbings[] = {
    "grabbing",
    "closedhand",
    "208530c400c041818281048008011002"
};

static const char *left_ptrs[] = {
    "left_ptr",
    "default",
    "top_left_arrow",
    "left-arrow"
};

static const char *left_sides[] = {
    "left_side",
    "w-resize"
};

static const char *right_sides[] = {
    "right_side",
    "e-resize"
};

static const char *top_left_corners[] = {
    "top_left_corner",
    "nw-resize"
};

static const char *top_right_corners[] = {
    "top_right_corner",
    "ne-resize"
};

static const char *top_sides[] = {
    "top_side",
    "n-resize"
};

static const char *xterms[] = {
    "xterm",
    "ibeam",
    "text"
};

static const char *hand1s[] = {
    "hand1",
    "pointer",
    "pointing_hand",
    "e29285e634086352946a0e7090d73106"
};

static const char *watches[] = {
    "watch",
    "wait",
    "0426c94ea35c87780ff01dc239897213"
};

struct cursor_alternatives {
    const char **names;
    size_t count;
};

static const struct cursor_alternatives cursors[] = {
    {left_ptrs, ARRAY_LENGTH(left_ptrs)},
    {bottom_left_corners, ARRAY_LENGTH(bottom_left_corners)},
    {bottom_right_corners, ARRAY_LENGTH(bottom_right_corners)},
    {bottom_sides, ARRAY_LENGTH(bottom_sides)},
    {grabbings, ARRAY_LENGTH(grabbings)},
    {left_sides, ARRAY_LENGTH(left_sides)},
    {right_sides, ARRAY_LENGTH(right_sides)},
    {top_left_corners, ARRAY_LENGTH(top_left_corners)},
    {top_right_corners, ARRAY_LENGTH(top_right_corners)},
    {top_sides, ARRAY_LENGTH(top_sides)},
    {xterms, ARRAY_LENGTH(xterms)},
    {hand1s, ARRAY_LENGTH(hand1s)},
    {watches, ARRAY_LENGTH(watches)},
};

QWaylandCursor::QWaylandCursor(QWaylandScreen *screen)
    : mDisplay(screen->display())
{
    //TODO: Make wl_cursor_theme_load arguments configurable here
    mCursorTheme = wl_cursor_theme_load("default", 32, mDisplay->shm());
    mCursors = new wl_cursor*[ARRAY_LENGTH(cursors)];

    for (uint i = 0; i < ARRAY_LENGTH(cursors); i++) {
        struct wl_cursor *cursor = NULL;
        for (uint j = 0; !cursor && j < cursors[i].count; ++j)
            cursor = wl_cursor_theme_get_cursor(mCursorTheme, cursors[i].names[j]);

        if (!cursor)
            qDebug() << "could not load cursor" << cursors[i].names[0];

        mCursors[i] = cursor;
    }
}

QWaylandCursor::~QWaylandCursor()
{
    wl_cursor_theme_destroy(mCursorTheme);
    delete[] mCursors;
}

void QWaylandCursor::changeCursor(QCursor *cursor, QWindow *window)
{
    Q_UNUSED(window)

    int pointer = 0;
    switch (cursor->shape()) {
    case Qt::UpArrowCursor:
    case Qt::CrossCursor:
    case Qt::WaitCursor:
        break;
    case Qt::IBeamCursor:
    case Qt::SizeVerCursor: /* 5 */
    case Qt::SizeHorCursor:
    case Qt::SizeBDiagCursor:
        pointer = 2;
        break;
    case Qt::SizeFDiagCursor:
        pointer = 1;
        break;
    case Qt::SizeAllCursor:
    case Qt::BlankCursor:   /* 10 */
        break;
    case Qt::SplitVCursor:
        pointer = 3;
        break;
    case Qt::SplitHCursor:
        pointer = 6;
        break;
    case Qt::PointingHandCursor:
    case Qt::ForbiddenCursor:
    case Qt::WhatsThisCursor:   /* 15 */
    case Qt::BusyCursor:
    case Qt::OpenHandCursor:
    case Qt::ClosedHandCursor:
        pointer = 4;
        break;
    case Qt::DragCopyCursor:
    case Qt::DragMoveCursor: /* 20 */
    case Qt::DragLinkCursor:
    case Qt::BitmapCursor:
    default:
        break;
    }

    struct wl_cursor *cur = mCursors[pointer];
    if (!cur)
        return;

    struct wl_cursor_image *image = cur->images[0];

    struct wl_buffer *buffer = wl_cursor_image_get_buffer(image);
    if (!buffer)
        return;

    mDisplay->setCursor(buffer, image);
}

void QWaylandDisplay::setCursor(struct wl_buffer *buffer, struct wl_cursor_image *image)
{
    /* Qt doesn't tell us which input device we should set the cursor
     * for, so set it for all devices. */
    for (int i = 0; i < mInputDevices.count(); i++) {
        QWaylandInputDevice *inputDevice = mInputDevices.at(i);
        inputDevice->setCursor(buffer, image);
    }
}

void QWaylandCursor::pointerEvent(const QMouseEvent &event)
{
    mLastPos = event.globalPos();
}

QPoint QWaylandCursor::pos() const
{
    return mLastPos;
}

void QWaylandCursor::setPos(const QPoint &pos)
{
    Q_UNUSED(pos);
    qWarning() << "QWaylandCursor::setPos: not implemented";
}
