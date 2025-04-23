#include "MedianMapQuickItem.h"
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>
#include <QVector3D>

MedianMapQuickItem::MedianMapQuickItem() {
    setFlag(ItemHasContents, true); // enables updatePaintNode()
}

void MedianMapQuickItem::setVizData(VizData* data) {
    if (vizData_ != data) {
        vizData_ = data;
        emit vizDataChanged();
        update(); // trigger repaint
    }
}

QSGNode* MedianMapQuickItem::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) {
    if (!vizData_ || !vizData_->rasterData) return nullptr;

    QSGGeometryNode* node = static_cast<QSGGeometryNode*>(oldNode);
    if (!node) {
        node = new QSGGeometryNode();
        auto geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), 0);
        geometry->setDrawingMode(QSGGeometry::DrawPoints);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);

        auto material = new QSGFlatColorMaterial();
        material->setColor(Qt::red);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
    }

    QSGGeometry* geometry = node->geometry();
    const int n = vizData_->rasterData->nLocations;
    geometry->allocate(n);

    QSGGeometry::Point2D* vertices = geometry->vertexDataAsPoint2D();

    for (int i = 0; i < n; ++i) {
        auto [row, col] = vizData_->rasterData->locationPair1DTo2D[i];
        vertices[i].set(float(col), float(row));
        // You can store color via material too (or extend geometry attributes for per-vertex color)
    }

    node->markDirty(QSGNode::DirtyGeometry);
    return node;
}
