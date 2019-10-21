#include "objectitem.h"
#include "paintscene.h"
#include "modelconfig.h"

#include <QDebug>

ObjectItem::ObjectItem(QGraphicsItem *parent):
    QGraphicsRectItem(parent)
{
    editable = true;
    this->setZValue(6);

    pm_1 = nullptr;
    pm_2 = nullptr;
    pm_center = nullptr;

    this->setBrush(QBrush(Qt::yellow));
    QPen pen = QPen(Qt::red);
    pen.setWidth(3);
    this->setPen(pen);

    connect(this, SIGNAL(positionChanged(QGraphicsItem*)),this,SLOT(pos_changed()));
    connect(this, SIGNAL(selectChanged(QGraphicsItem*)),this,SLOT(select_changed()));

}

ObjectItem::~ObjectItem() {
    ModelConfig::Instance()->setObject(nullptr);

    qDebug() << "delete ObjectItem(" << dynamic_cast<QGraphicsItem *>(this) << ")";
}

QPointF ObjectItem::getP1() {
    return this->point_1;
}

QPointF ObjectItem::getP2() {
    return this->point_2;
}

void ObjectItem::setP1(QPointF point) {
    this->point_1 = point;
    this->setRectByTwoPoints(this->point_1, this->point_2);
}

void ObjectItem::setP2(QPointF point) {
    this->point_2 = point;
    this->setRectByTwoPoints(this->point_1, this->point_2);
}

void ObjectItem::setRectByTwoPoints(QPointF p_1, QPointF p_2)
{
    this->point_1 = p_1;
    this->point_2 = p_2;

    qreal x;
    qreal y;
    qreal w;
    qreal h;

    // Система координат +x(>) -y(^)

    if (p_2.x() >= p_1.x()) {
        // правые квадранты относительно p_1
        x = p_1.x();
        w = p_2.x() - x;
    } else {
        // левые квадранты относительно p_1
        x = p_2.x();
        w = p_1.x() - x;
    }
    if (p_2.y() >= p_1.y()) {
        // нижние квадранты относительно p_1
        y = p_1.y();
        h = p_2.y() - y;
    } else {
        // верхние квадранты относительно p_1
        y = p_2.y();
        h = p_1.y() - y;
    }

    // Cоздаем его так, чтобы
    this->setRect(-w/2,-h/2,w,h);
    this->setPos(x+w/2,y+h/2);
}

void ObjectItem::setMagnetPoints()
{
    pm_center = new PaintPoint(PaintPoint::CENTRAL, this);

    this->pm_captures.append(pm_1 = new PaintPoint(0, this));
    this->pm_captures.append(new PaintPoint(1, this));
    this->pm_captures.append(new PaintPoint(2, this));
    this->pm_captures.append(pm_2 = new PaintPoint(3, this));
}

PaintPoint* ObjectItem::getPm1()
{
    return pm_1;
}

PaintPoint* ObjectItem::getPm2()
{
    return pm_2;
}

QPointF ObjectItem::getMagnetPointPos(int index) {
    if (index == PaintPoint::CENTRAL) {
        return this->rect().center();
    }

    /* TODO: нужен алгоритм для оптимальной расстановки креплений на обьекте управления
     * количество креплений брать из настроек
     * форма обьекта - прямоугольник.
    */
    if (index >= 0) {
        if (index == 0) return this->rect().topLeft();
        if (index == 1) return this->rect().topRight();
        if (index == 2) return this->rect().bottomRight();
        if (index == 3) return this->rect().bottomLeft();
    }

    return QPointF();
}

QVariant ObjectItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        emit selectChanged(this);
    } else if (change == ItemPositionChange && scene()) {
        emit positionChanged(this);
    }
    return QGraphicsRectItem::itemChange(change, value);
}

void ObjectItem::select_changed() {
    if (this->editable && this->flags().testFlag(QGraphicsItem::ItemIsMovable)) {
        if (!(this->isSelected())) {
            qDebug() << "начинаем тащить";

            QList<QGraphicsItem*> other_selecting = (dynamic_cast<PaintScene*>(this->scene()))->EditorDialog->getSelectedItems();

            // Перебираем все привязанные к обьекту точки
            foreach(QGraphicsItem* point_, this->childItems()) {

                // предок должен быть PaintPoint
                PaintPoint* point;
                if (point_->type() == PaintPoint::Type)
                    point = dynamic_cast<PaintPoint*>(point_);
                else
                    continue;

                // Ищем связанные с точками обьекта точки
                QList<PaintPoint*> point2s = point->getBindNodes();

                // Определение списка точек, которые необходимо обновлять при перемещении.
                if (point2s.count() > 0) {
                    foreach(PaintPoint *point2, point2s) {
                        // Общие для выбранных элементов точки не участвуют в перемещении
                        if (other_selecting.contains(point2->parentItem()))
                            continue;

                        MoveGoal goal;
                        if (point2->parentItem()->type() == UnitItem::Type) {
                            UnitItem* item_ = dynamic_cast<UnitItem*>(point2->parentItem());
                            if (item_->isEditable()) {
                                goal.point_goal = point;
                                goal.point_moving = point2;
                            }
                            moving_data.append(goal);
                        }
                    }
                }
            }
        } else {
            qDebug() << "заканчиваем тащить";
            moving_data.clear();
        }
    }
}

void ObjectItem::pos_changed() {
    if (this->editable && this->flags().testFlag(QGraphicsItem::ItemIsMovable)) {
        // осуществляем синхронизацию положений точек
        foreach(MoveGoal goal, moving_data) {
            QGraphicsItem *updated_item = goal.point_moving->parentItem();
            if (updated_item->type() == UnitItem::Type) {
                goal.point_moving->move_binded_points = false;
                updated_item->setPos(goal.point_goal->scenePos());
                goal.point_moving->move_binded_points = true;
            }
        }
    }
}

