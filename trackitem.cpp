#include "trackitem.h"
#include "modelconfig.h"

TrackItem::TrackItem(QPainterPath startPosPath, QGraphicsPathItem *parent) :
    QGraphicsPathItem(startPosPath, parent)
{
    editable = true;
    distance = 0;
    this->setZValue(7);

    QPen pen = QPen(Qt::red);
    pen.setWidth(4);
    this->setPen(pen);

}
TrackItem::~TrackItem()
{
    ModelConfig::Instance()->setTrack(nullptr);
    qDebug() << "delete TrackItem(" << dynamic_cast<QGraphicsItem *>(this) << ")";
}

void TrackItem::setEndPoint()
{

    QPainterPath path = this->path();
    QPainterPath::Element el = path.elementAt(path.elementCount()-1);

    QGraphicsEllipseItem *endPointItem = new QGraphicsEllipseItem(this);
    endPointItem->setRect(el.x-10,el.y-10,2*10,2*10);
    endPointItem->setBrush(QBrush(Qt::red));
    endPointItem->setPen(QPen(Qt::red));
}

QVariant TrackItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        emit selectChanged(this);
    } else if (change == ItemPositionHasChanged && scene()) {
        emit positionChanged(this);
    }
    return QGraphicsPathItem::itemChange(change, value);
}

