#ifndef TRACKITEM_H
#define TRACKITEM_H

#include <QObject>
#include <QtCore>
#include <QtGui>
#include <QGraphicsPathItem>
#include <QPainterPath>

#include "paintitem.h"

/**
 * @brief For paint Object Track on PaintScene
 */
class TrackItem : public PaintItem, public QGraphicsPathItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 7 };
    explicit TrackItem(QPainterPath startPosPath, QGraphicsPathItem *parent = 0);
    ~TrackItem();

    int type() const { return Type; }

    bool editable;
    double distance;
    bool isEditable()
    {
        return editable;
    }

    // Завершает ввод траектории и устанавливает целевую точку
    void setEndPoint();

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
};

#endif // TRACKITEM_H
