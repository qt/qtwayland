// Copyright (C) 2015 LG Electronics Ltd, author: <mikko.levonmaa@lge.com>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SHMWINDOW_H
#define SHMWINDOW_H

#include <QtGui>

class ShmWindow : public QRasterWindow
{
    Q_OBJECT
public:
    explicit ShmWindow(QWindow *parent);

protected:
    void paintEvent(QPaintEvent *event) override;
    void timerEvent(QTimerEvent *) override;

private:
    int m_rotation;
    int m_timer;
};
#endif // SHMWINDOW_H
