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

#include "qwaylanddecoration.h"

#include "qwaylandwindow.h"
#include "qwaylandshellsurface.h"
#include "qwaylandinputdevice.h"
#include "qwaylandscreen.h"

#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QCursor>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QLinearGradient>

QT_BEGIN_NAMESPACE

#define BUTTON_SPACING 5

#ifndef QT_NO_IMAGEFORMAT_XPM
#  define BUTTON_WIDTH 10

static const char * const qt_close_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"..........",
".##....##.",
"..##..##..",
"...####...",
"....##....",
"...####...",
"..##..##..",
".##....##.",
"..........",
".........."};

static const char * const qt_maximize_xpm[]={
"10 10 2 1",
"# c #000000",
". c None",
"#########.",
"#########.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#.......#.",
"#########.",
".........."};

static const char * const qt_minimize_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"..........",
"..........",
"..........",
"..........",
"..........",
"..........",
"..........",
".#######..",
".#######..",
".........."};

static const char * const qt_normalizeup_xpm[] = {
"10 10 2 1",
"# c #000000",
". c None",
"...######.",
"...######.",
"...#....#.",
".######.#.",
".######.#.",
".#....###.",
".#....#...",
".#....#...",
".######...",
".........."};
#else
#  define BUTTON_WIDTH 22
#endif

QWaylandDecoration::QWaylandDecoration(QWaylandWindow *window)
    : m_window(window->window())
    , m_wayland_window(window)
    , m_isDirty(true)
    , m_decorationContentImage(0)
    , m_margins(3,30,3,3)
    , m_mouseButtons(Qt::NoButton)
{
    m_wayland_window->setDecoration(this);

    QPalette palette;
    m_foregroundColor = palette.color(QPalette::Active, QPalette::HighlightedText);
    m_backgroundColor = palette.color(QPalette::Active, QPalette::Highlight);

    QTextOption option(Qt::AlignHCenter | Qt::AlignVCenter);
    option.setWrapMode(QTextOption::NoWrap);
    m_windowTitle.setTextOption(option);
}

QWaylandDecoration::~QWaylandDecoration()
{
    m_wayland_window->setDecoration(0);
}

const QImage &QWaylandDecoration::contentImage()
{
    if (m_isDirty) {
        //Update the decoration backingstore

        m_decorationContentImage = QImage(window()->frameGeometry().size(), QImage::Format_ARGB32_Premultiplied);
        m_decorationContentImage.fill(Qt::transparent);
        this->paint(&m_decorationContentImage);

        m_isDirty = false;
    }

    return m_decorationContentImage;
}

void QWaylandDecoration::update()
{
    m_isDirty = true;
}

void QWaylandDecoration::paint(QPaintDevice *device)
{
    QRect surfaceRect(QPoint(), window()->frameGeometry().size());
    QRect clips[] =
    {
        QRect(0, 0, surfaceRect.width(), margins().top()),
        QRect(0, surfaceRect.height() - margins().bottom(), surfaceRect.width(), margins().bottom()),
        QRect(0, margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom()),
        QRect(surfaceRect.width() - margins().right(), margins().top(), margins().left(), surfaceRect.height() - margins().top() - margins().bottom())
    };

    QRect top = clips[0];

    QPainter p(device);
    p.setRenderHint(QPainter::Antialiasing);

    // Title bar
    QPoint gradCenter(top.center()+ QPoint(30, 60));
    QLinearGradient grad(top.topLeft(), top.bottomLeft());
    QColor base(backgroundColor());
    grad.setColorAt(0, base.lighter(100));
    grad.setColorAt(1, base.darker(180));
    QPainterPath roundedRect;
    roundedRect.addRoundedRect(surfaceRect, 6, 6);
    for (int i = 0; i < 4; ++i) {
        p.save();
        p.setClipRect(clips[i]);
        p.fillPath(roundedRect, grad);
        p.restore();
    }

    // Window icon
    QIcon icon = m_wayland_window->windowIcon();
    if (!icon.isNull()) {
        QPixmap pixmap = icon.pixmap(QSize(128, 128));
        QPixmap scaled = pixmap.scaled(22, 22, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QRectF iconRect(0, 0, 22, 22);
        p.drawPixmap(iconRect.adjusted(margins().left() + BUTTON_SPACING, 4,
                                       margins().left() + BUTTON_SPACING, 4),
                     scaled, iconRect);
    }

    // Window title
    QString windowTitleText = window()->title();
    if (!windowTitleText.isEmpty()) {
        if (m_windowTitle.text() != windowTitleText) {
            m_windowTitle.setText(windowTitleText);
            m_windowTitle.prepare();
        }

        QRect titleBar = top;
        titleBar.setLeft(m_margins.left() + BUTTON_SPACING +
            (icon.isNull() ? 0 : 22 + BUTTON_SPACING));
        titleBar.setRight(minimizeButtonRect().left() - BUTTON_SPACING);

        p.save();
        p.setClipRect(titleBar);
        p.setPen(m_foregroundColor);
        QSizeF size = m_windowTitle.size();
        int dx = (top.width() - size.width()) /2;
        int dy = (top.height()- size.height()) /2;
        QFont font = p.font();
        font.setBold(true);
        p.setFont(font);
        QPoint windowTitlePoint(top.topLeft().x() + dx,
                 top.topLeft().y() + dy);
        p.drawStaticText(windowTitlePoint,m_windowTitle);
        p.restore();
    }

#ifndef QT_NO_IMAGEFORMAT_XPM
    p.save();

    // Close button
    QPixmap closePixmap(qt_close_xpm);
    p.drawPixmap(closeButtonRect(), closePixmap, closePixmap.rect());

    // Maximize button
    QPixmap maximizePixmap(m_wayland_window->isMaximized()
                           ? qt_normalizeup_xpm : qt_maximize_xpm);
    p.drawPixmap(maximizeButtonRect(), maximizePixmap, maximizePixmap.rect());

    // Minimize button
    QPixmap minimizePixmap(qt_minimize_xpm);
    p.drawPixmap(minimizeButtonRect(), minimizePixmap, minimizePixmap.rect());

    p.restore();
#else
    // We don't need antialiasing from now on
    p.setRenderHint(QPainter::Antialiasing, false);

    QRectF rect;

    // Default pen
    QPen pen(m_foregroundColor);
    p.setPen(pen);

    // Close button
    p.save();
    rect = closeButtonRect();
    p.drawRect(rect);
    qreal crossSize = rect.height() / 2;
    QPointF crossCenter(rect.center());
    QRectF crossRect(crossCenter.x() - crossSize / 2, crossCenter.y() - crossSize / 2, crossSize, crossSize);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawLine(crossRect.topLeft(), crossRect.bottomRight());
    p.drawLine(crossRect.bottomLeft(), crossRect.topRight());
    p.restore();

    // Maximize button
    p.save();
    p.drawRect(maximizeButtonRect());
    rect = maximizeButtonRect().adjusted(5, 5, -5, -5);
    if (m_wayland_window->isMaximized()) {
        QRectF rect1 = rect.adjusted(rect.width() / 3, 0, 0, -rect.height() / 3);
        QRectF rect2 = rect.adjusted(0, rect.height() / 4, -rect.width() / 4, 0);
        p.drawRect(rect1);
        p.drawRect(rect2);
    } else {
        p.setPen(m_foregroundColor);
        p.drawRect(rect);
        p.drawLine(rect.left(), rect.top() + 1, rect.right(), rect.top() + 1);
    }
    p.restore();

    // Minimize button
    p.save();
    p.drawRect(minimizeButtonRect());
    rect = minimizeButtonRect().adjusted(5, 5, -5, -5);
    pen.setWidth(2);
    p.setPen(pen);
    p.drawLine(rect.bottomLeft(), rect.bottomRight());
    p.restore();
#endif
}

bool QWaylandDecoration::handleMouse(QWaylandInputDevice *inputDevice, const QPointF &local, const QPointF &global, Qt::MouseButtons b, Qt::KeyboardModifiers mods)

{
    Q_UNUSED(global);

    // Figure out what area mouse is in
    if (closeButtonRect().contains(local) && isLeftClicked(b)) {
        QWindowSystemInterface::handleCloseEvent(m_window);
    } else if (maximizeButtonRect().contains(local) && isLeftClicked(b)) {
        m_window->setWindowState(m_wayland_window->isMaximized() ? Qt::WindowNoState : Qt::WindowMaximized);
    } else if (minimizeButtonRect().contains(local) && isLeftClicked(b)) {
        m_window->setWindowState(Qt::WindowMinimized);
    } else if (local.y() <= m_margins.top()) {
        processMouseTop(inputDevice,local,b,mods);
    } else if (local.y() > m_window->height() - m_margins.bottom() + m_margins.top()) {
        processMouseBottom(inputDevice,local,b,mods);
    } else if (local.x() <= m_margins.left()) {
        processMouseLeft(inputDevice,local,b,mods);
    } else if (local.x() > m_window->width() - m_margins.right() + m_margins.left()) {
        processMouseRight(inputDevice,local,b,mods);
    } else {
        m_wayland_window->restoreMouseCursor(inputDevice);
        m_mouseButtons = b;
        return false;
    }

    m_mouseButtons = b;
    return true;
}

bool QWaylandDecoration::inMouseButtonPressedState() const
{
    return m_mouseButtons & Qt::NoButton;
}

void QWaylandDecoration::startResize(QWaylandInputDevice *inputDevice, enum wl_shell_surface_resize resize, Qt::MouseButtons buttons)
{
    if (isLeftClicked(buttons)) {
        m_wayland_window->shellSurface()->resize(inputDevice, resize);
        inputDevice->removeMouseButtonFromState(Qt::LeftButton);
    }
}

void QWaylandDecoration::startMove(QWaylandInputDevice *inputDevice, Qt::MouseButtons buttons)
{
    if (isLeftClicked(buttons)) {
        m_wayland_window->shellSurface()->move(inputDevice);
        inputDevice->removeMouseButtonFromState(Qt::LeftButton);
    }
}

void QWaylandDecoration::processMouseTop(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods);
    if (local.y() <= m_margins.bottom()) {
        if (local.x() <= margins().left()) {
            //top left bit
            m_wayland_window->setMouseCursor(inputDevice, Qt::SizeFDiagCursor);
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_LEFT,b);
        } else if (local.x() > m_window->width() - margins().right()) {
            //top right bit
            m_wayland_window->setMouseCursor(inputDevice, Qt::SizeBDiagCursor);
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP_RIGHT,b);
        } else {
            //top reszie bit
            m_wayland_window->setMouseCursor(inputDevice, Qt::SplitVCursor);
            startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_TOP,b);
        }
    } else {
        m_wayland_window->restoreMouseCursor(inputDevice);
        startMove(inputDevice,b);
    }

}

void QWaylandDecoration::processMouseBottom(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(mods);
    if (local.x() <= margins().left()) {
        //bottom left bit
        m_wayland_window->setMouseCursor(inputDevice, Qt::SizeBDiagCursor);
        startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_LEFT,b);
    } else if (local.x() > m_window->width() - margins().right()) {
        //bottom right bit
        m_wayland_window->setMouseCursor(inputDevice, Qt::SizeFDiagCursor);
        startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_BOTTOM_RIGHT,b);
    } else {
        //bottom bit
        m_wayland_window->setMouseCursor(inputDevice, Qt::SplitVCursor);
        startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_BOTTOM,b);
    }
}

void QWaylandDecoration::processMouseLeft(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
    m_wayland_window->setMouseCursor(inputDevice, Qt::SplitHCursor);
    startResize(inputDevice,WL_SHELL_SURFACE_RESIZE_LEFT,b);
}

void QWaylandDecoration::processMouseRight(QWaylandInputDevice *inputDevice, const QPointF &local, Qt::MouseButtons b, Qt::KeyboardModifiers mods)
{
    Q_UNUSED(local);
    Q_UNUSED(mods);
    m_wayland_window->setMouseCursor(inputDevice, Qt::SplitHCursor);
    startResize(inputDevice, WL_SHELL_SURFACE_RESIZE_RIGHT,b);
}

bool QWaylandDecoration::isLeftClicked(Qt::MouseButtons newMouseButtonState)
{
    if ((!m_mouseButtons & Qt::LeftButton) && (newMouseButtonState & Qt::LeftButton))
        return true;
    return false;
}

bool QWaylandDecoration::isLeftReleased(Qt::MouseButtons newMouseButtonState)
{
    if ((m_mouseButtons & Qt::LeftButton) && !(newMouseButtonState & Qt::LeftButton))
        return true;
    return false;
}

QRectF QWaylandDecoration::closeButtonRect() const
{
    return QRectF(window()->frameGeometry().width() - BUTTON_WIDTH - BUTTON_SPACING * 2,
                  (m_margins.top() - BUTTON_WIDTH) / 2, BUTTON_WIDTH, BUTTON_WIDTH);
}

QRectF QWaylandDecoration::maximizeButtonRect() const
{
    return QRectF(window()->frameGeometry().width() - BUTTON_WIDTH * 2 - BUTTON_SPACING * 3,
                  (m_margins.top() - BUTTON_WIDTH) / 2, BUTTON_WIDTH, BUTTON_WIDTH);
}

QRectF QWaylandDecoration::minimizeButtonRect() const
{
    return QRectF(window()->frameGeometry().width() - BUTTON_WIDTH * 3 - BUTTON_SPACING * 4,
                  (m_margins.top() - BUTTON_WIDTH) / 2, BUTTON_WIDTH, BUTTON_WIDTH);
}

void QWaylandDecoration::setForegroundColor(const QColor &c)
{
    m_foregroundColor = c;
}

void QWaylandDecoration::setBackgroundColor(const QColor &c)
{
    m_backgroundColor = c;
}

QT_END_NAMESPACE
