#include "waylandsurfacenode.h"
#include "waylandsurfaceitem.h"

#include <QtCore/QMutexLocker>
#include <QtQuick/QSGTexture>
#include <QtQuick/QSGSimpleTextureNode>
#include <QtQuick/QSGFlatColorMaterial>

WaylandSurfaceNode::WaylandSurfaceNode(WaylandSurfaceItem *item)
    : m_item(item)
    , m_textureUpdated(false)
    , m_useTextureAlpha(false)
    , m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
{
    m_textureMaterial = WaylandSurfaceTextureMaterial::createMaterial();
    m_opaqueTextureMaterial = WaylandSurfaceTextureOpaqueMaterial::createMaterial();

    m_currentMaterial = m_opaqueTextureMaterial;

    setGeometry(&m_geometry);
    setMaterial(m_currentMaterial);

    if (m_item)
        m_item->m_node = this;
    setFlag(UsePreprocess,true);
}


WaylandSurfaceNode::~WaylandSurfaceNode()
{
    QMutexLocker locker(WaylandSurfaceItem::mutex);
    if (m_item)
        m_item->m_node = 0;
    delete m_textureMaterial;
    delete m_opaqueTextureMaterial;
}

void WaylandSurfaceNode::preprocess()
{
    QMutexLocker locker(WaylandSurfaceItem::mutex);

    if (m_item && m_item->surface()) {
        //Update if the item is dirty and we haven't done an updateTexture for this frame
        if (m_item->m_damaged && !m_textureUpdated) {
            m_item->updateTexture();
            updateTexture();
        }
    }
    //Reset value for next frame: we have not done updatePaintNode yet
    m_textureUpdated = false;
}

void WaylandSurfaceNode::updateTexture()
{
    Q_ASSERT(m_item && m_item->textureProvider());

    //If m_item->useTextureAlpha has changed to true use m_texureMaterial
    //otherwise use m_opaqueTextureMaterial.
    if (m_item->useTextureAlpha() != m_useTextureAlpha) {
        m_useTextureAlpha = m_item->useTextureAlpha();
        if (m_useTextureAlpha) {
            m_currentMaterial = m_textureMaterial;
        } else {
            m_currentMaterial = m_opaqueTextureMaterial;
        }
        setMaterial(m_currentMaterial);
    }

    QSGTexture *texture = m_item->textureProvider()->texture();
    setTexture(texture);
}

void WaylandSurfaceNode::setRect(const QRectF &rect)
{
    if (m_rect == rect)
        return;
    m_rect = rect;

    if (texture()) {
        QSize ts = texture()->textureSize();
        QRectF sourceRect(0, 0, ts.width(), ts.height());
        QSGGeometry::updateTexturedRectGeometry(&m_geometry, m_rect, texture()->convertToNormalizedSourceRect(sourceRect));
    }
}

void WaylandSurfaceNode::setTexture(QSGTexture *texture)
{
    if (m_currentMaterial->state()->texture() == texture)
        return;
    m_currentMaterial->state()->setTexture(texture);

    QSize ts = texture->textureSize();
    QRectF sourceRect(0, 0, ts.width(), ts.height());
    QSGGeometry::updateTexturedRectGeometry(&m_geometry, m_rect, texture->convertToNormalizedSourceRect(sourceRect));
    markDirty(DirtyMaterial);
}

QSGTexture *WaylandSurfaceNode::texture() const
{
    return m_currentMaterial->state()->texture();
}

void WaylandSurfaceNode::setItem(WaylandSurfaceItem *item)
{
    m_item = item;
}
