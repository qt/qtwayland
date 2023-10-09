// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#include "qwltextureorphanage_p.h"

#include <QOpenGLContext>
#include <QOpenGLTexture>
#include <QDebug>
#include <QtTypeTraits>
#include <QMutexLocker>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(qLcWTO, "qt.waylandcompositor.orphanage")

Q_GLOBAL_STATIC(QtWayland::QWaylandTextureOrphanage, inst)

namespace QtWayland {

QWaylandTextureOrphanage::~QWaylandTextureOrphanage()
{
    QMutexLocker locker(&m_containerLock);
    if (!m_orphanedTextures.isEmpty()) {
        qCWarning(qLcWTO) << Q_FUNC_INFO << "m_orphanedTextures container isn't empty! content:"
                          << m_orphanedTextures;
    }
}

QWaylandTextureOrphanage *QWaylandTextureOrphanage::instance()
{
    return inst;
}

void QWaylandTextureOrphanage::admitTexture(QOpenGLTexture *tex, QOpenGLContext *ctx)
{
    qCDebug(qLcWTO) << Q_FUNC_INFO << "got a texture (" << (void *)tex
                    << ") ready to be deleted! It's ctx:" << ctx;

    {
        QMutexLocker locker(&m_containerLock);
        m_orphanedTextures.insert(ctx, tex);
    }

    connect(ctx, &QOpenGLContext::aboutToBeDestroyed, this,
            [this, ctx]() { this->onContextAboutToBeDestroyed(ctx); },
            Qt::ConnectionType(Qt::DirectConnection));
}

void QWaylandTextureOrphanage::deleteTextures()
{
    QOpenGLContext *cCtx = QOpenGLContext::currentContext();

    if (cCtx == nullptr) {
        qCWarning(qLcWTO) << Q_FUNC_INFO << "cannot delete textures without current OpenGL context";
        return;
    }

    {
        QMutexLocker locker(&m_containerLock);

        for (QOpenGLContext *aCtx : m_orphanedTextures.keys()) {
            if (QOpenGLContext::areSharing(cCtx, aCtx)) {

                qCDebug(qLcWTO) << Q_FUNC_INFO << "currentContext (" << cCtx
                                << ") and ctx of orphane(s) (" << aCtx
                                << ") are shared! => deleteTexturesByContext";

                deleteTexturesByContext(aCtx);
            }
        }
    }
}

void QWaylandTextureOrphanage::onContextAboutToBeDestroyed(QOpenGLContext *ctx)
{
    Q_ASSERT(ctx != nullptr);

    qCDebug(qLcWTO) << Q_FUNC_INFO << " ctx (" << ctx
                    << ") fired aboutToBeDestroyed => deleteTexturesByContext(ctx)";

    {
        QMutexLocker locker(&m_containerLock);
        deleteTexturesByContext(ctx);
    }
}

void QWaylandTextureOrphanage::deleteTexturesByContext(QOpenGLContext *ctx)
{
    // NOTE: We are (by class-internal design) locked (m_containerLock)
    // when we enter this function!
    // If not (e.g.: someone changes something in/around this class),
    // then in a debug-build we will fail below:
    Q_ASSERT(!m_containerLock.tryLock());

    QList<QOpenGLTexture *> texturesToDelete = m_orphanedTextures.values(ctx);
    m_orphanedTextures.remove(ctx);

    for (QOpenGLTexture *tex : texturesToDelete) {
        delete tex;
        qCDebug(qLcWTO) << Q_FUNC_INFO << " texture (" << (void *)tex << ") got deleted";
    }
}

} // namespace QtWayland

QT_END_NAMESPACE

#include "moc_qwltextureorphanage_p.cpp"
