// Copyright (C) 2017 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWAYLANDQUICKITEM_P_H
#define QWAYLANDQUICKITEM_P_H

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

#include <QtQuick/private/qquickitem_p.h>
#include <QtQuick/QSGMaterialShader>
#include <QtQuick/QSGMaterial>

#include <QtWaylandCompositor/QWaylandQuickItem>
#include <QtWaylandCompositor/QWaylandOutput>

#include <QtCore/qpointer.h>

QT_BEGIN_NAMESPACE

class QWaylandSurfaceTextureProvider;
class QMutex;
class QOpenGLTexture;

#if QT_CONFIG(opengl)
class QWaylandBufferMaterialShader : public QSGMaterialShader
{
public:
    QWaylandBufferMaterialShader(QWaylandBufferRef::BufferFormatEgl format);

    bool updateUniformData(RenderState &state,
                           QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void updateSampledImage(RenderState &state, int binding, QSGTexture **texture,
                            QSGMaterial *newMaterial, QSGMaterial *oldMaterial) override;
    void setupExternalOESShader(const QString &shaderFilename);
};

class QWaylandBufferMaterial : public QSGMaterial
{
public:
    QWaylandBufferMaterial(QWaylandBufferRef::BufferFormatEgl format);
    ~QWaylandBufferMaterial() override;

    void setTextureForPlane(int plane, QOpenGLTexture *texture, QSGTexture *scenegraphTexture);
    void setBufferRef(QWaylandQuickItem *surfaceItem, const QWaylandBufferRef &ref);

    void bind();
    void updateScenegraphTextures(QRhi *rhi);

    QSGMaterialType *type() const override;
    QSGMaterialShader *createShader(QSGRendererInterface::RenderMode renderMode) const override;

private:
    friend QWaylandBufferMaterialShader;

    void setTextureParameters(GLenum target);
    void ensureTextures(int count);

    const QWaylandBufferRef::BufferFormatEgl m_format;
    QVarLengthArray<QOpenGLTexture*, 3> m_textures;
    QVarLengthArray<QSGTexture*, 3> m_scenegraphTextures;
    QWaylandBufferRef m_bufferRef;
};
#endif // QT_CONFIG(opengl)

class QWaylandQuickItemPrivate : public QQuickItemPrivate
{
    Q_DECLARE_PUBLIC(QWaylandQuickItem)
public:
    QWaylandQuickItemPrivate() = default;

    void init()
    {
        Q_Q(QWaylandQuickItem);
        if (!mutex)
            mutex = new QMutex;

        view.reset(new QWaylandView(q));
        q->setFlag(QQuickItem::ItemHasContents);

        q->update();

        q->setSmooth(true);

        setInputEventsEnabled(true);
        QObject::connect(q, &QQuickItem::windowChanged, q, &QWaylandQuickItem::updateWindow);
        QObject::connect(view.data(), &QWaylandView::surfaceChanged, q, &QWaylandQuickItem::surfaceChanged);
        QObject::connect(view.data(), &QWaylandView::surfaceChanged, q, &QWaylandQuickItem::handleSurfaceChanged);
        QObject::connect(view.data(), &QWaylandView::surfaceDestroyed, q, &QWaylandQuickItem::surfaceDestroyed);
        QObject::connect(view.data(), &QWaylandView::outputChanged, q, &QWaylandQuickItem::outputChanged);
        QObject::connect(view.data(), &QWaylandView::outputChanged, q, &QWaylandQuickItem::updateOutput);
        QObject::connect(view.data(), &QWaylandView::bufferLockedChanged, q, &QWaylandQuickItem::bufferLockedChanged);
        QObject::connect(view.data(), &QWaylandView::allowDiscardFrontBufferChanged, q, &QWaylandQuickItem::allowDiscardFrontBuffer);

        q->updateWindow();
    }

    static const QWaylandQuickItemPrivate* get(const QWaylandQuickItem *item) { return item->d_func(); }

    void setInputEventsEnabled(bool enable)
    {
        Q_Q(QWaylandQuickItem);
        q->setAcceptedMouseButtons(enable ? (Qt::LeftButton | Qt::MiddleButton | Qt::RightButton |
                                   Qt::ExtraButton1 | Qt::ExtraButton2 | Qt::ExtraButton3 | Qt::ExtraButton4 |
                                   Qt::ExtraButton5 | Qt::ExtraButton6 | Qt::ExtraButton7 | Qt::ExtraButton8 |
                                   Qt::ExtraButton9 | Qt::ExtraButton10 | Qt::ExtraButton11 |
                                   Qt::ExtraButton12 | Qt::ExtraButton13) : Qt::NoButton);
        q->setAcceptTouchEvents(enable);
        q->setAcceptHoverEvents(enable);
        inputEventsEnabled = enable;
    }

    void handleDragEnded(QWaylandSeat *seat);
    void handleDragUpdate(QWaylandSeat *seat, const QPointF &globalPosition);

    bool shouldSendInputEvents() const { return view->surface() && inputEventsEnabled; }
    qreal scaleFactor() const;

    QWaylandQuickItem *findSibling(QWaylandSurface *surface) const;
    void placeAboveSibling(QWaylandQuickItem *sibling);
    void placeBelowSibling(QWaylandQuickItem *sibling);
    void placeAboveParent();
    void placeBelowParent();

    virtual void raise();
    virtual void lower();

    static QMutex *mutex;

    QScopedPointer<QWaylandView> view;
    QPointer<QWaylandSurface> oldSurface;
    mutable QWaylandSurfaceTextureProvider *provider = nullptr;
    QMetaObject::Connection texProviderConnection;
    bool paintEnabled = true;
    bool touchEventsEnabled = true;
    bool inputEventsEnabled = true;
    bool isDragging = false;
    bool newTexture = false;
    bool focusOnClick = true;
    bool belowParent = false;
#if QT_CONFIG(opengl)
    bool paintByProvider = false;
#endif
    QPointF hoverPos;
    QMatrix4x4 lastMatrix;

    QQuickWindow *connectedWindow = nullptr;
    QWaylandOutput *connectedOutput = nullptr;
    QWaylandSurface::Origin origin = QWaylandSurface::OriginTopLeft;
    QPointer<QObject> subsurfaceHandler;
    QList<QWaylandSeat *> touchingSeats;
};

QT_END_NAMESPACE

#endif  /*QWAYLANDQUICKITEM_P_H*/
