// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef COMPOSITOR_H
#define COMPOSITOR_H

#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandView>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class Window;
class QOpenGLTexture;
class QWaylandIviApplication;
class QWaylandIviSurface;

class View : public QWaylandView
{
    Q_OBJECT
public:
    explicit View(int iviId) : m_iviId(iviId) {}
    QOpenGLTexture *getTexture();
    int iviId() const { return m_iviId; }

    QRect globalGeometry() const { return QRect(globalPosition(), surface()->destinationSize()); }
    void setGlobalPosition(const QPoint &globalPos) { m_pos = globalPos; m_positionSet = true; }
    QPoint globalPosition() const { return m_pos; }
    QPoint mapToLocal(const QPoint &globalPos) const;
    QSize size() const { return surface() ? surface()->destinationSize() : QSize(); }

    void initPosition(const QSize &screenSize, const QSize &surfaceSize);

private:
    QOpenGLTexture *m_texture = nullptr;
    bool m_positionSet = false;
    QPoint m_pos;
    int m_iviId;
};

class Compositor : public QWaylandCompositor
{
    Q_OBJECT
public:
    Compositor(Window *window);
    ~Compositor() override;
    void create() override;

    QList<View*> views() const { return m_views; }
    View *viewAt(const QPoint &position);
    void raise(View *view);

    void handleMousePress(const QPoint &position, Qt::MouseButton button);
    void handleMouseRelease(const QPoint &position, Qt::MouseButton button, Qt::MouseButtons buttons);
    void handleMouseMove(const QPoint &position);
    void handleMouseWheel(const QPoint &angleDelta);

    void handleKeyPress(quint32 nativeScanCode);
    void handleKeyRelease(quint32 nativeScanCode);

    void startRender();
    void endRender();

private slots:
    void onIviSurfaceCreated(QWaylandIviSurface *iviSurface);
    void triggerRender();

    void viewSurfaceDestroyed();
private:
    Window *m_window = nullptr;
    QWaylandIviApplication *m_iviApplication = nullptr;
    QList<View*> m_views;
    QPointer<View> m_mouseView;
};

QT_END_NAMESPACE

#endif // COMPOSITOR_H
