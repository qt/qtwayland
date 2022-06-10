// Copyright (C) 2015 LG Electronics Ltd, author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "shmwindow.h"

#include <QPainter>
#include <QDebug>

ShmWindow::ShmWindow(QWindow *parent)
   : QRasterWindow(parent)
   , m_rotation(0)
{
    m_timer = startTimer(100);
}

void ShmWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer) {
        m_rotation++;
        update();
    }
}

void ShmWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.fillRect(0, 0, width(), height(), Qt::white);

    qreal xc = width() * 0.5;
    qreal yc = height() * 0.5;
    painter.translate(xc, yc);
    painter.rotate(m_rotation);
    painter.drawText(QRectF(-xc, -yc, width(), height()), Qt::AlignCenter, QStringLiteral("SHM"));
}

