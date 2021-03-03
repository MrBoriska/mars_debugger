#ifndef UNITITEM_H
#define UNITITEM_H

#include <QObject>
#include <QtCore>
#include <QGraphicsPixmapItem>

#include "paintitem.h"

/**
 * @brief For paint control Unit on PaintScene
 */
class UnitItem: public PaintItem, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 1 };
    UnitItem(QPointF position, QGraphicsItem *parent = nullptr);
    ~UnitItem();

    int type() const { return Type; }

    bool editable;
    bool isEditable()
    {
        return editable;
    }

    QPointF getMagnetPointPos(int index);

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif // UNITITEM_H
