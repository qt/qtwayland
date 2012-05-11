#ifndef WAYLANDSURFACENODE_H
#define WAYLANDSURFACENODE_H

#include "waylandsurfacetexturematerial.h"

#include <QtQuick/QSGGeometryNode>
#include <QtQuick/QSGOpaqueTextureMaterial>

class WaylandSurfaceItem;
class QSGTexture;

class WaylandSurfaceNode : public QSGGeometryNode
{
public:
    WaylandSurfaceNode(WaylandSurfaceItem *item = 0);
    ~WaylandSurfaceNode();

    void preprocess();
    void updateTexture();

    void setRect(const QRectF &rect);
    inline void setRect(qreal x, qreal y, qreal w, qreal h) { setRect(QRectF(x, y, w, h)); }

    void setTexture(QSGTexture *texture);
    QSGTexture *texture() const;

    bool isTextureUpdated() const { return m_textureUpdated; }
    void setTextureUpdated(bool textureUpdated) { m_textureUpdated = textureUpdated; }

    WaylandSurfaceItem *item() const { return m_item; }
    void setItem(WaylandSurfaceItem *item);

private:

    WaylandSurfaceItem *m_item;
    bool m_textureUpdated;
    bool m_useTextureAlpha;

    QSGGeometry m_geometry;
    QSGSimpleMaterial<WaylandSurfaceTextureState> *m_textureMaterial;
    QSGSimpleMaterial<WaylandSurfaceTextureState> *m_opaqueTextureMaterial;
    QSGSimpleMaterial<WaylandSurfaceTextureState> *m_currentMaterial;

    QRectF m_rect;
};

#endif // WAYLANDSURFACENODE_H
