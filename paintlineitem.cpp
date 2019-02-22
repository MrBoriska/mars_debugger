#include "paintlineitem.h"

#include <QtCore>
#include <QtGui>

#include <QDebug>

PaintLineItem::PaintLineItem(QGraphicsItem *parent):
    QGraphicsLineItem(parent)
{
    editable = true;
    this->setZValue(2);

    QPen pencil;
    pencil.setColor(Qt::blue);
    pencil.setWidth(25);
    this->setPen(pencil);
}

PaintLineItem::~PaintLineItem()
{
    qDebug() << "delete PaintLineItem(" << (QGraphicsItem *)this << ")";
}

QVariant PaintLineItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        emit selectChanged(this);
    } else if (change == ItemPositionHasChanged && scene()) {
        emit positionChanged(this);
    }
    return QGraphicsLineItem::itemChange(change, value);
}

