// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef WINDOW_H
#define WINDOW_H

#include <QOpenGLWindow>
#include <QOpenGLTextureBlitter>

QT_BEGIN_NAMESPACE

class Compositor;

class Window : public QOpenGLWindow
{
    Q_OBJECT
public:
    Window();
    void setCompositor(Compositor *comp);

signals:
    void glReady();

protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *) override;

    void keyPressEvent(QKeyEvent *e) override;
    void keyReleaseEvent(QKeyEvent *e) override;

private:
    QOpenGLTextureBlitter m_textureBlitter;
    Compositor *m_compositor = nullptr;
};

QT_END_NAMESPACE

#endif //WINDOW_H
