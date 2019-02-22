#include "unititem.h"

#include <QtCore>
#include <QtGui>
#include <QDebug>

#include "modelconfig.h"

UnitItem::UnitItem(QPointF position, QGraphicsItem *parent):
    QGraphicsPixmapItem(parent)
{
    editable = true;
    //this->setCacheMode(QGraphicsItem::NoCache);

    QPixmap picture(":/pictures/icons/unit.png");
    this->setPixmap(picture);

    this->setPos(position);
    this->setOffset(-picture.width()/2, -picture.height()/2);
    this->update();

    this->setZValue(4);
}

UnitItem::~UnitItem()
{
    ModelConfig::Instance()->delUnit(this);
    qDebug() << "delete UnitItem(" << (QGraphicsItem *)this << ")";
}

QVariant UnitItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        emit selectChanged(this);
    } else if (change == ItemPositionHasChanged && scene()) {
        emit positionChanged(this);
    }
    return QGraphicsPixmapItem::itemChange(change, value);
}
