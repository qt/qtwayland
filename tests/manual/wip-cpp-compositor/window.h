// Copyright (C) 2019 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef WINDOW_H
#define WINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLTextureBlitter>

QT_BEGIN_NAMESPACE

class Compositor;
class QWaylandOutput;

class Window : public QOpenGLWindow
{
public:
    explicit Window(Compositor *compositor);

protected:
    void initializeGL() override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
#if QT_CONFIG(wheelevent)
    void wheelEvent(QWheelEvent *event) override;
#endif

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QOpenGLTextureBlitter m_textureBlitter;
    Compositor *m_compositor = nullptr;
    QWaylandOutput *m_output = nullptr;
};

QT_END_NAMESPACE

#endif //WINDOW_H
