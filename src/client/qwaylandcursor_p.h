/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
******************************************************************************/

#ifndef QWAYLANDCURSOR_H
#define QWAYLANDCURSOR_H

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

#include <qpa/qplatformcursor.h>
#include <QtCore/QMap>
#include <QtWaylandClient/qtwaylandclientglobal.h>

#if QT_CONFIG(cursor)

#include <memory>

struct wl_cursor;
struct wl_cursor_image;
struct wl_cursor_theme;

QT_BEGIN_NAMESPACE

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandDisplay;
class QWaylandScreen;
class QWaylandShm;

class Q_WAYLAND_CLIENT_EXPORT QWaylandCursorTheme
{
public:
    static std::unique_ptr<QWaylandCursorTheme> create(QWaylandShm *shm, int size, const QString &themeName);
    ~QWaylandCursorTheme();
    ::wl_cursor *cursor(Qt::CursorShape shape);

protected:
    enum WaylandCursor {
        ArrowCursor = Qt::ArrowCursor,
        UpArrowCursor,
        CrossCursor,
        WaitCursor,
        IBeamCursor,
        SizeVerCursor,
        SizeHorCursor,
        SizeBDiagCursor,
        SizeFDiagCursor,
        SizeAllCursor,
        BlankCursor,
        SplitVCursor,
        SplitHCursor,
        PointingHandCursor,
        ForbiddenCursor,
        WhatsThisCursor,
        BusyCursor,
        OpenHandCursor,
        ClosedHandCursor,
        DragCopyCursor,
        DragMoveCursor,
        DragLinkCursor,
        // The following are used for cursors that don't have equivalents in Qt
        ResizeNorthCursor = Qt::CustomCursor + 1,
        ResizeSouthCursor,
        ResizeEastCursor,
        ResizeWestCursor,
        ResizeNorthWestCursor,
        ResizeSouthEastCursor,
        ResizeNorthEastCursor,
        ResizeSouthWestCursor,

        NumWaylandCursors
    };

    explicit QWaylandCursorTheme(struct ::wl_cursor_theme *theme) : m_theme(theme) {}
    struct ::wl_cursor *requestCursor(WaylandCursor shape);
    struct ::wl_cursor_theme *m_theme = nullptr;
    wl_cursor *m_cursors[NumWaylandCursors] = {};
};

class Q_WAYLAND_CLIENT_EXPORT QWaylandCursor : public QPlatformCursor
{
public:
    explicit QWaylandCursor(QWaylandDisplay *display);

    void changeCursor(QCursor *cursor, QWindow *window) override;
    void pointerEvent(const QMouseEvent &event) override;
    QPoint pos() const override;
    void setPos(const QPoint &pos) override;

    static QSharedPointer<QWaylandBuffer> cursorBitmapBuffer(QWaylandDisplay *display, const QCursor *cursor);

protected:
    QWaylandDisplay *mDisplay = nullptr;
    QPoint mLastPos;
};

}

QT_END_NAMESPACE

#endif // cursor
#endif // QWAYLANDCURSOR_H
