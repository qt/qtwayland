// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

#ifndef QWLTEXTUREORPHANAGE_P_H
#define QWLTEXTUREORPHANAGE_P_H

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

#include <QObject>
#include <QMutex>
#include <QLoggingCategory>
#include <QtWaylandCompositor/qtwaylandcompositorglobal.h>

QT_BEGIN_NAMESPACE

class QOpenGLContext;
class QOpenGLTexture;

Q_DECLARE_LOGGING_CATEGORY(qLcWTO)

namespace QtWayland {

class Q_WAYLANDCOMPOSITOR_EXPORT QWaylandTextureOrphanage : public QObject
{
    Q_OBJECT

public:
    QWaylandTextureOrphanage(){};
    ~QWaylandTextureOrphanage();

    static QWaylandTextureOrphanage *instance();

    // texture that isn't needed anymore will be "take care of" (killed) appropriately
    void admitTexture(QOpenGLTexture *tex, QOpenGLContext *ctx);

    // uses QOpenGLContext::currentContext to call deleteTexturesByContext on all shared ctx
    void deleteTextures();

public slots:
    // uses sender() to call deleteTexturesByContext
    void onContextAboutToBeDestroyed(QOpenGLContext *ctx);

private:
    void deleteTexturesByContext(QOpenGLContext *ctx);

    // tracks all the orphanes that need to be deleted
    QMultiHash<QOpenGLContext *, QOpenGLTexture *> m_orphanedTextures;

    QMutex m_containerLock;
};

} // namespace QtWayland

QT_END_NAMESPACE
#endif
