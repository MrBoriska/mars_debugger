#ifndef OBJECTITEM_H
#define OBJECTITEM_H

#include <QObject>
#include <QtGui>
#include <QtCore>
#include <QGraphicsRectItem>
#include <QTimer>

#include "paintitem.h"
#include "paintpoint.h"

/**
 * @brief Class for control object
 */
class ObjectItem : public PaintItem, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 6 };

    ObjectItem(QGraphicsItem *parent = 0);
    ~ObjectItem();

    int type() const { return Type; }

    bool editable;
    bool isEditable()
    {
        return editable;
    }

    QPointF point_1;
    QPointF point_2;

    QPointF getP1();
    QPointF getP2();
    void setP1(QPointF point);
    void setP2(QPointF point);
    void setRectByTwoPoints(QPointF point_1, QPointF point_2);

    void setMagnetPoints();
    QPointF getMagnetPointPos(int index);
    PaintPoint *getPm1();
    PaintPoint *getPm2();

    // Точка для связи траектории с обьектом управления
    PaintPoint *pm_center;
    PaintPoint *pm_1;
    PaintPoint *pm_2;
    QList<PaintPoint *> pm_captures;

public slots:
    void pos_changed();
    void select_changed();

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);


    struct MoveGoal {
        PaintPoint *point_goal;
        PaintPoint *point_moving;
    };
    QList<MoveGoal> moving_data;

};

#endif // OBJECTITEM_H
