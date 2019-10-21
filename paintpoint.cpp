#include "paintpoint.h"

#include <QtCore>
#include <QtGui>
#include <QDebug>
#include <QGraphicsScene>

#include "paintpolygonitem.h"
#include "objectitem.h"
#include "unititem.h"

PaintPoint::PaintPoint(int index, QGraphicsItem *parent) :
    QGraphicsEllipseItem(parent)
{
    editable = true;
    move_binded_points = true;
    this->setZValue(5);
    this->setPen(QPen(Qt::transparent));

    this->radius = 10;
    this->index = 0;

    if (index >= -1 && parent) {
        this->index = index;
        if (parent->type() == PaintPolygonItem::Type) {
            this->createPoint((dynamic_cast<PaintPolygonItem *>(parent))->polygon().at(index), radius);
        }
        else if (parent->type() == ObjectItem::Type) {
            this->createPoint((dynamic_cast<ObjectItem *>(parent))->getMagnetPointPos(index), radius);
        }
        else if (parent->type() == UnitItem::Type) {
            this->createPoint(QPoint(0,0), radius);
        }

    }

    //this->setBrush(QBrush(Qt::red));

    this->setAcceptHoverEvents(true);
    connect(this, SIGNAL(positionChanged(QGraphicsItem*)), this, SLOT(pos_changed()));
}

PaintPoint::~PaintPoint()
{
    qDebug() << "delete PaintPoint(" << dynamic_cast<QGraphicsItem *>(this) << ")";

    // unbind от присоединенных points
    foreach(PaintPoint* point, nodes) {
        point->unbindNode(this);
    }
}

void PaintPoint::createPoint(QPointF point, qreal R)
{
    qreal x = point.x();
    qreal y = point.y();

    this->setRect(-R,-R,2*R,2*R);
    this->setPos(x,y);
}

void PaintPoint::setPointPos(QPointF point)
{
    this->createPoint(point, radius);
}

void PaintPoint::bindNode(PaintPoint* point_item)
{
    nodes.append(point_item);

    qDebug() << "PaintPoint(" << dynamic_cast<QGraphicsItem *>(this) << ")"
             << " by node " << QString::number(this->index)
             << " bind " << "PaintPoint(" << dynamic_cast<QGraphicsItem *>(point_item) << ")"
             << " by node " << QString::number(point_item->index);
}

void PaintPoint::unbindNode(PaintPoint *point_item)
{
    nodes.removeOne(point_item);
}

QList<PaintPoint*> PaintPoint::getBindNodes()
{
    return nodes;
}

QVariant PaintPoint::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        emit selectChanged(this);
    } else if (change == ItemPositionHasChanged && scene()) {
        emit positionChanged(this);
    }
    return QGraphicsEllipseItem::itemChange(change, value);
}

void PaintPoint::hoverEnterEvent(QGraphicsSceneHoverEvent * event)
{
    this->setPen(QPen(Qt::black));
    QGraphicsItem::hoverEnterEvent(event);
    //qDebug() << "point index " << QString::number(this->index) << (QGraphicsItem*)this;
}

void PaintPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent * event)
{
    this->setPen(QPen(Qt::transparent));
    QGraphicsItem::hoverLeaveEvent(event);
}

void PaintPoint::pos_changed()
{
    if (index >= -1 && this->parentItem()) {
        if (this->parentItem()->type() == PaintPolygonItem::Type) {
            // Обновляем положение присоединенного узла полигона согласно положению точки
            PaintPolygonItem *poly_item = dynamic_cast<PaintPolygonItem*>(this->parentItem());
            QPolygonF polygon = poly_item->polygon();
            polygon[this->index] = this->pos();
            poly_item->setPolygon(polygon);

            // Если точка перемещается отдельно,
            // то необходимо самостоятельно обновлять положения присоединенных точек
            if (move_binded_points) {
                foreach(PaintPoint *point, this->nodes) {
                    QGraphicsItem *updated_item = point->parentItem();
                    if (updated_item->type() == PaintPolygonItem::Type) {
                        point->move_binded_points = false;
                        point->setPos(
                            point->parentItem()->mapFromScene(this->scenePos())
                        );
                        point->move_binded_points = true;
                    }
                }
            }
        }
    }
}

