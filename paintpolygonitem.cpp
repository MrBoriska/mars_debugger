#include "paintpolygonitem.h"

#include "modelconfig.h"
#include "paintscene.h"

PaintPolygonItem::PaintPolygonItem(QGraphicsItem *parent):
    QGraphicsPolygonItem(parent)
{
    this->editable = true;
    this->is_obstacle = false;
    this->setZValue(1);
    this->setBrush(ModelConfig::Instance()->defaultMaterial.color);

    connect(this, SIGNAL(positionChanged(QGraphicsItem*)),this,SLOT(pos_changed()));
    connect(this, SIGNAL(selectChanged(QGraphicsItem*)),this,SLOT(select_changed()));
}

PaintPolygonItem::~PaintPolygonItem()
{
    qDebug() << "delete PaintPolygonItem(" << dynamic_cast<QGraphicsItem *>(this) << ")";
}



void PaintPolygonItem::isObstacle(bool is_obstacle)
{
    this->is_obstacle = is_obstacle;
    // Устанавливаем фон для препятствия
    QBrush brush(QPixmap(":/pictures/icons/obst_background.png"));
    this->setBrush(QBrush(brush));
}

bool PaintPolygonItem::isObstacle()
{
    return this->is_obstacle;
}

QColor PaintPolygonItem::getMaterialColor()
{
    if (!is_obstacle)
        return this->brush().color();
    else {
        qDebug() << "Warnig PaintPolygonItem::getMaterialColor():"
                    " у материала препятствия нет цвета";
        return ModelConfig::Instance()->defaultMaterial.color;
    }
}

void PaintPolygonItem::setMaterialColor(QColor color)
{
    if (!is_obstacle)
        this->setBrush(QBrush(color));
    else {
        qDebug() << "Warnig PaintPolygonItem::setMaterialColor(QColor color):"
                    " нельзя менять материал для препятствия";
    }
}

QVariant PaintPolygonItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemSelectedChange && scene()) {
        emit selectChanged(this);
    } else if (change == ItemPositionHasChanged && scene()) {
        emit positionChanged(this);
    }
    return QGraphicsPolygonItem::itemChange(change, value);
}

void PaintPolygonItem::select_changed() {
    if (this->editable && this->flags().testFlag(QGraphicsItem::ItemIsMovable)) {
        if (!(this->isSelected())) {
            qDebug() << "начинаем тащить";

            QList<QGraphicsItem*> other_selecting = (dynamic_cast<PaintScene*>(this->scene()))->EditorDialog->getSelectedItems();

            foreach(QGraphicsItem* point_, this->childItems()) {

                // предок должен быть PaintPoint
                PaintPoint* point;
                if (point_->type() == PaintPoint::Type)
                    point = dynamic_cast<PaintPoint*>(point_);
                else
                    continue;

                QList<PaintPoint*> point2s = point->getBindNodes();

                // Определение списка точек, которые необходимо обновлять при перемещении.
                if (point2s.count() > 0) {
                    foreach(PaintPoint *point2, point2s) {
                        // Общие для выбранных элементов точки не участвуют в перемещении
                        if (other_selecting.contains(point2->parentItem()))
                            continue;

                        MoveGoal goal;
                        if (point2->parentItem()->type() == PaintPolygonItem::Type) {
                            PaintPolygonItem* item_ = dynamic_cast<PaintPolygonItem*>(point2->parentItem());
                            if (item_->isEditable()) {
                                goal.point_goal = point;
                                goal.point_moving = point2;
                            } else {
                                goal.point_goal = point2;
                                goal.point_moving = point;
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

void PaintPolygonItem::pos_changed() {
    if (this->editable && this->flags().testFlag(QGraphicsItem::ItemIsMovable)) {
        // осуществляем синхронизацию положений точек
        foreach(MoveGoal goal, moving_data) {
            QGraphicsItem *updated_item = goal.point_moving->parentItem();
            if (updated_item->type() == PaintPolygonItem::Type) {
                goal.point_moving->move_binded_points = false;
                goal.point_moving->setPos(
                    goal.point_moving->parentItem()->mapFromScene(goal.point_goal->scenePos())
                );
                goal.point_moving->move_binded_points = true;
            }
        }
    }
}
