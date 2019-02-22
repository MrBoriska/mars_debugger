#ifndef PAINTLINEITEM_H
#define PAINTLINEITEM_H

#include <QObject>
#include <QtCore>
#include <QGraphicsLineItem>

#include "paintitem.h"

/**
 * @brief For paint Line on PaintScene
 */
class PaintLineItem : public PaintItem, public QGraphicsLineItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 2 };

    PaintLineItem(QGraphicsItem *parent = 0);
    ~PaintLineItem();

    int type() const { return Type; }

    bool editable;
    bool isEditable()
    {
        return editable;
    }

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

};

#endif // PAINTLINEITEM_H
