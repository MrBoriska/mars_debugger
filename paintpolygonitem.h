#ifndef PAINTPOLYGONITEM_H
#define PAINTPOLYGONITEM_H

#include <QtCore>
#include <QtGui>
#include <QObject>
#include <QGraphicsPolygonItem>

#include "paintitem.h"

//предварительное обьявление
class PaintPoint;

/**
 * @brief For paint Polygon on PaintScene
 */
class PaintPolygonItem : public PaintItem, public QGraphicsPolygonItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 3 };

    PaintPolygonItem(QGraphicsItem *parent = 0);
    ~PaintPolygonItem();

    int type() const { return Type; }

    bool editable;
    bool isEditable()
    {
        return editable;
    }

    QColor getMaterialColor();
    void setMaterialColor(QColor color);

    bool isObstacle();
    void isObstacle(bool is_obstacle);

public slots:
    void select_changed();
    void pos_changed();

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    bool is_obstacle;

    struct MoveGoal {
        PaintPoint *point_goal;
        PaintPoint *point_moving;
    };
    QList<MoveGoal> moving_data;
};

#endif // PAINTPOLYGONITEM_H
