#ifndef PAINTPOINT_H
#define PAINTPOINT_H

#include <QObject>
#include <QtCore>
#include <QGraphicsEllipseItem>

#include "paintitem.h"
#include "paintlineitem.h"
#include "paintpolygonitem.h"

/**
 * @brief Класс для описания служебных точек для привязки и т.п.
 */
class PaintPoint : public PaintItem, public QGraphicsEllipseItem
{
    Q_OBJECT
public:
    enum { Type = UserType + 4 };

    enum { CENTRAL = -1, NONE = -2 };

    /* index = NONE - default value
     * index = CENTRAL - central point
     * index >= 0 - index nodes */
    explicit PaintPoint(int index = NONE, QGraphicsItem *parent = nullptr);
    ~PaintPoint();

    int type() const { return Type; }

    int index;
    bool move_binded_points;
    bool editable;
    bool isEditable()
    {
        return editable;
    }


    // Привязывает узел элемента
    void bindNode(PaintPoint *point_item);
    // Отвязывает узел элемента
    void unbindNode(PaintPoint *point_item);
    // Возвращает все привязанные узлы
    QList<PaintPoint*> getBindNodes();
    // Устанавливает положение точки(не узла)
    void setPointPos(QPointF point);

public slots:
    void pos_changed();

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

private:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    // Узлы привязанных к точке элементов
    QList<PaintPoint*> nodes;

    // радиус области точки по умолчанию
    qreal radius;

    /**
     * @brief Непосредственно выполняет отрисовку точки
     * @param point, координаты точки
     * @param R, радиус области точки
     */
    void createPoint(QPointF point, qreal R);
};



#endif // PAINTPOINT_H
