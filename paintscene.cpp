#include "paintscene.h"

#include <QtCore>
#include <QtGui>
#include <QString>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>

#include "modelconfig.h"

#include <cmath>

#define PI 3.14159265

PaintScene::PaintScene(QObject *parent) : QGraphicsScene(parent)
{
    basePolygon = nullptr;
    baseBorderPolygon = nullptr;

    polygon_item = nullptr;
    unit_item = nullptr;
    object_item = nullptr;
    track_item = nullptr;
    current_point = nullptr;
    paint_mod = OffMOD;

    prev_angle = 0;

    pen = QPen(Qt::black);
    pen.setWidth(3);
    brush = QBrush(Qt::lightGray);

    connect(this, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(sceneRectShow(QRectF)));
}

PaintScene::~PaintScene()
{

}

void PaintScene::safeClear()
{
    this->clear();

    basePolygon = nullptr;
    baseBorderPolygon = nullptr;
    polygon_item = nullptr;
    unit_item = nullptr;
    object_item = nullptr;
    track_item = nullptr;
    current_point = nullptr;
    paint_mod = OffMOD;
    prev_angle = 0;
    pen = QPen(Qt::black);
    pen.setWidth(3);
    brush = QBrush(Qt::lightGray);
}


bool PaintScene::isBasePolygon(PaintPolygonItem *p)
{
    if (p == basePolygon)
        return true;
    if (p == baseBorderPolygon)
        return true;
    return false;
}

void PaintScene::sceneRectShow(const QRectF &rect)
{
    qDebug() << rect;

    this->setSceneRect(rect);

    if (basePolygon != nullptr) {
        removeItem(basePolygon);
        delete basePolygon;
    }
    basePolygon = new PaintPolygonItem();
    basePolygon->setZValue(0);
    basePolygon->setPolygon(QPolygon(rect.toRect()));
    basePolygon->setMaterialColor(ModelConfig::Instance()->defaultMaterial.color);

    (new PaintPoint(0, basePolygon))->editable = false;
    (new PaintPoint(1, basePolygon))->editable = false;
    (new PaintPoint(2, basePolygon))->editable = false;
    (new PaintPoint(3, basePolygon))->editable = false;

    basePolygon->editable = false;

    this->addItem(basePolygon);

    // добавление граничного элемента
    if (baseBorderPolygon != nullptr) {
        removeItem(baseBorderPolygon);
        delete baseBorderPolygon;
    }

    qreal b_w = ModelConfig::Instance()->getSceneBorderWidth();
    qreal s_w = rect.width();
    qreal s_h = rect.height();

    // set points for border polygon-obstacle
    QPolygonF polygon;
    polygon << QPointF(0, 0)
            << QPointF(s_w+2*b_w, 0)
            << QPointF(s_w+2*b_w, s_h+2*b_w)
            << QPointF(0, s_h+2*b_w)
            << QPointF(0, 0)
            << QPointF(b_w, b_w)
            << QPointF(b_w, s_h+b_w)
            << QPointF(s_w+b_w, s_h+b_w)
            << QPointF(s_w+b_w, b_w)
            << QPointF(b_w, b_w);

    baseBorderPolygon = new PaintPolygonItem();
    baseBorderPolygon->setZValue(4);
    baseBorderPolygon->isObstacle(true);
    baseBorderPolygon->setPolygon(polygon);
    baseBorderPolygon->editable = false;

    this->addItem(baseBorderPolygon);
}

void PaintScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier)) {
        if (paint_mod == PolygonMOD || paint_mod == ObstacleMOD)
        {
            if (polygon_item == nullptr) {
                if (event->buttons() == Qt::LeftButton) {
                    // Создание полигона из двух точек
                    // Одна начальная, а другая будет перемещаться вслед за мышью(в mouseMoveEvent)
                    QPolygonF polygon;

                    // Определение позиции с учетом "магнитных" точек(от других элементов под мышью)
                    QPointF nodePos = event->scenePos();
                    fsetMagnetPointPos(event->scenePos(), &nodePos);

                    // Создаем полигон
                    polygon << nodePos << event->scenePos();
                    polygon_item = new PaintPolygonItem();
                    polygon_item->setPolygon(polygon);

                    // Доп. для полигонов-препятствий
                    if (paint_mod == ObstacleMOD)
                        polygon_item->isObstacle(true);

                    // Создаем новые магнитные точки
                    PaintPoint *current_point1 = new PaintPoint(0, polygon_item);
                    current_point = new PaintPoint(1, polygon_item);

                    this->addItem(polygon_item);

                    // Устанавливаем связи между магнитными точками
                    if (magnet_points.size() > 0) {
                        // Определяем, можно ли будет перемещать эти точки потом
                        // Если нет, то всем этим точкам запрещаем редактирование
                        bool movable_points = true;
                        foreach(PaintPoint *magnet_point, magnet_points) {
                            if (!(magnet_point->isEditable())) {
                                movable_points = false;
                                break;
                            }
                        }

                        if (!movable_points) {
                            current_point1->editable = false;
                        }
                        foreach(PaintPoint *magnet_point, magnet_points) {
                            magnet_point->bindNode(current_point1);
                            current_point1->bindNode(magnet_point);
                            if (!movable_points) {
                                magnet_point->editable = false;
                            }
                        }
                    }
                }
            } else {
                if (event->buttons() == Qt::LeftButton) {
                    // Переход к установке следующей точки полигона
                    QPolygonF polygon = polygon_item->polygon();

                    polygon << event->scenePos();
                    polygon_item->setPolygon(polygon);
                    polygon_item->update();

                    // Устанавливаем связи между магнитными точками
                    if (magnet_points.size() > 0) {
                        // Определяем, можно ли будет перемещать эти точки потом
                        // Если нет, то всем этим точкам запрещаем редактирование
                        bool movable_points = true;
                        foreach(PaintPoint *magnet_point, magnet_points) {
                            if (!(magnet_point->isEditable())) {
                                movable_points = false;
                                break;
                            }
                        }

                        if (!movable_points) {
                            current_point->editable = false;
                        }
                        foreach(PaintPoint *magnet_point, magnet_points) {
                            magnet_point->bindNode(current_point);
                            current_point->bindNode(magnet_point);
                            if (!movable_points) {
                                magnet_point->editable = false;
                            }
                        }
                    }

                    current_point = new PaintPoint(polygon.count()-1, polygon_item);

                } else if (event->buttons() == Qt::RightButton) {
                    QPolygonF polygon = polygon_item->polygon();

                    // отмена ввода последней точки полигона
                    polygon.pop_back();
                    // удаление привязанной к последней точке полигона PointItem
                    this->removeItem(current_point);
                    delete current_point;

                    // если полигон состоит из более чем двух узлов, сохраняем его
                    if (polygon.count() > 2) {
                        // Применяем изменения
                        polygon_item->setPolygon(polygon);
                        polygon_item->update();

                        // При нажатии ПКМ завершаем ввод
                        emit addedItem(polygon_item);
                    } else {
                        delete polygon_item;
                    }
                    polygon_item = nullptr;
                }
            }
        } else if (paint_mod == UnitMOD && event->buttons() == Qt::LeftButton) {
            if (unit_item == nullptr) {
                // Создание юнита
                QPointF unitPos = event->scenePos();
                fsetMagnetPointPos(event->scenePos(), &unitPos, ObjectItem::Type);
                unit_item = new UnitItem(unitPos);
                current_point = new PaintPoint(PaintPoint::CENTRAL, unit_item);

                if (magnet_points.size() > 0) {
                    foreach(PaintPoint *magnet_point, magnet_points) {
                        magnet_point->bindNode(current_point);
                        current_point->bindNode(magnet_point);
                    }
                }

                this->addItem(unit_item);
            } else {
                ModelConfig::Instance()->addUnit(unit_item);
                emit addedItem(unit_item);
                unit_item = nullptr;
                prev_angle = 0;
            }

        } else if (paint_mod == ObjectMOD && event->buttons() == Qt::LeftButton) {
            if (object_item == nullptr) {
                if (ModelConfig::Instance()->getObject() != nullptr) {
                    QMessageBox::warning(
                        reinterpret_cast<QWidget *>(this->activeWindow()),
                        "info",
                        "Обьект управления может быть только один"
                    );
                } else {
                    // Создание обьекта управления
                    object_item = new ObjectItem();
                    object_item->setRectByTwoPoints(event->scenePos(), event->scenePos());

                    this->addItem(object_item);
                }
            } else {
                // Завершение создание обьекта управления
                // Создание магнитных точек(для крепления роботов и начала траектории)
                object_item->setMagnetPoints();
                ModelConfig::Instance()->setObject(object_item);
                emit addedItem(object_item);
                object_item = nullptr;
            }
        }
        else if (paint_mod == TrackMOD)
        {
            if (track_item == nullptr ) {
                if (ModelConfig::Instance()->getTrack() != nullptr) {
                    QMessageBox::warning(
                        reinterpret_cast<QWidget *>(this->activeWindow()),
                        "info",
                        "Траектория уже создана"
                    );
                } else if (event->buttons() == Qt::LeftButton) {
                    QPointF nodePos;
                    ObjectItem *ob_itm = nullptr;

                    // Ищем обьект управления на сцене и ставим начальную точку в центр этого обьекта
                    QList<QGraphicsItem *> under_items = this->items(event->scenePos());
                    if (!(under_items.isEmpty())) {
                        foreach(QGraphicsItem *item, under_items)
                        {
                            if(item && item->type() == ObjectItem::Type) {
                                ob_itm = dynamic_cast<ObjectItem*>(item);

                                nodePos = ob_itm->mapToScene(ob_itm->getMagnetPointPos(PaintPoint::CENTRAL));
                                break;
                            }
                        }
                    }

                    if (ob_itm) {
                        QPainterPath path(nodePos);
                        // Начинаем траекторию в nodePos
                        track_item = new TrackItem(path);

                        // В начальный момент вторая точка траектории совпадает с началом
                        path.lineTo(event->scenePos()+QPointF(0.01,0.01));
                        track_item->setPath(path);
                        this->addItem(track_item);
                    } else {
                        QMessageBox::warning(
                            reinterpret_cast<QWidget *>(this->activeWindow()),
                            "info",
                            "Траектория всегда должна начинаться с положения обьекта управления"
                        );
                    }
                }
            } else {
                if (event->buttons() == Qt::LeftButton) {
                    // Переход к установке следующей точки траектории
                    QPainterPath path = track_item->path();

                    // todo: Ограничение на "равность отрезков пути"
                    /*QVector2D vector = QVector2D(event->scenePos() - path.elementAt(path.elementCount()-2));
                    if (track_item->distance == 0) {
                        track_item->distance = vector.length();
                    }
                    vector.normalize();
                    vector *= track_item->distance;

                    vector = QVector2D(path.elementAt(path.elementCount()-2)) + vector;

                    path.setElementPositionAt(path.elementCount()-1, vector.x(), vector.y());
                    */
                    path.lineTo(event->scenePos()+QPointF(0.01,0.01));
                    track_item->setPath(path);

                } else if (event->buttons() == Qt::RightButton) {
                    QPainterPath path = track_item->path();

                    // todo: Ограничение на "равность отрезков пути"
                    QVector2D vector = QVector2D(event->scenePos() - path.elementAt(path.elementCount()-2));
                    /*if (track_item->distance == 0) {
                        track_item->distance = vector.length();
                    }
                    vector.normalize();
                    vector *= track_item->distance;*/

                    vector = QVector2D(path.elementAt(path.elementCount()-2)) + vector;

                    path.setElementPositionAt(path.elementCount()-1, vector.x(), vector.y());

                    track_item->setPath(path);

                    // Устанавливаем целевую точку траектории
                    track_item->setEndPoint();
                    ModelConfig::Instance()->setTrack(track_item);

                    // При нажатии ПКМ завершаем ввод
                    emit addedItem(track_item);
                    track_item = nullptr;
                }
            }
        }
    }

    QGraphicsScene::mousePressEvent(event);
}


void PaintScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(event->modifiers() & Qt::ControlModifier)) {
        if (paint_mod == PolygonMOD || paint_mod == ObstacleMOD) {
            if (polygon_item != nullptr) {
                // изменение положения последней точки полигона
                QPolygonF polygon = polygon_item->polygon();
                polygon.pop_back();

                //by default position
                QPointF nodePos = event->scenePos();

                //search magnet points and define pos
                fsetMagnetPointPos(event->scenePos(), &nodePos);

                // set node position
                polygon << nodePos;
                current_point->setPointPos(nodePos);
                polygon_item->setPolygon(polygon);
                polygon_item->update();
            }
        } else if (paint_mod == ObjectMOD) {
            if (object_item != nullptr) {
                object_item->setP2(event->scenePos());
                //object_item->update();
                //this->update();
                QSize size_ob = object_item->rect().size().toSize();
                emit changedStatus("Объект размера:"
                                   + QString::number(size_ob.width())
                                   + "x" + QString::number(size_ob.height()) + "см");
            }
        } else if (paint_mod == UnitMOD) {
            if (unit_item != nullptr) {
                // второе нажатие задает угол
                QPointF p_start = unit_item->sceneBoundingRect().center();
                QPointF p_end = event->scenePos();

                double angle = atan2(p_end.y()-p_start.y(),p_end.x()-p_start.x());
                angle = angle * 180/PI;

                setRotationItem(unit_item, p_start, angle - prev_angle);

                prev_angle = angle;
            }

        } else if (paint_mod == TrackMOD) {
            if (track_item != nullptr) {


                //by default position
                QPointF nodePos = event->scenePos();

                //search magnet points and define pos
                //fsetMagnetPointPos(event->scenePos(), &nodePos);

                QPainterPath path = track_item->path();
                path.setElementPositionAt(path.elementCount()-1, nodePos.x(), nodePos.y());
                track_item->setPath(path);
                track_item->update();
                this->update();
            } else {
                this->clearSelection();

                static QGraphicsItem *sel_item = nullptr;
                int s_count = 0;

                // todo: сделано слишком сложно, сделать проще

                // Подсвечиваем обьект при наведении на него мышкой перед началом заданием траектории
                QList<QGraphicsItem *> under_items = this->items(event->scenePos());
                if (!(under_items.isEmpty()))
                {
                    foreach(QGraphicsItem *item, under_items)
                    {
                        if(item && item->type() == ObjectItem::Type)
                        {
                            sel_item = item;
                            sel_item->setFlags(QGraphicsItem::ItemIsSelectable);
                            sel_item->setSelected(true);
                            s_count++;
                            break;
                        }
                    }
                }
                if (!s_count && sel_item && sel_item != nullptr)
                {
                    sel_item->setSelected(false);
                    sel_item->setFlags(sel_item->flags() & ~QGraphicsItem::ItemIsSelectable);
                }
            }
        }
    }

    QGraphicsScene::mouseMoveEvent(event);
}

void PaintScene::keyPressEvent(QKeyEvent *keyevent)
{
    if (keyevent->key()==Qt::Key_Escape)
    {
        qDebug() << "escape pressed";
        if (paint_mod != OffMOD)
        {
            //paint_mod = OffMOD;

            if (polygon_item) delete polygon_item;
            if (unit_item) delete unit_item;
            if (object_item) delete object_item;
            if (track_item) delete track_item;

            polygon_item = nullptr;
            unit_item = nullptr;
            object_item = nullptr;
            track_item = nullptr;

            current_point = nullptr;
            magnet_points.clear();
        }
    } else if (keyevent->key()==Qt::Key_Delete)
    {
        qDebug() << "delete pressed";
        if (paint_mod == EditMOD)
        {
            foreach(QGraphicsItem* item, this->selectedItems()) {
                item->setSelected(false);
                this->removeItem(item);
                delete item;
            }
        }
    }

    QGraphicsScene::keyPressEvent(keyevent);
}
/**
 * @brief PaintScene::fsetMagnetPointPos ищет список magnet_points в позиции pos
 * magnet_point = 0 - не найдено
 * magnet_point = PaintPoint* - найдено
 * QPointF* nodePos = magnet_point position - найдено
 * @param pos
 * @param nodePos
 */
void PaintScene::fsetMagnetPointPos(QPointF pos, QPointF *nodePos, const int ParentItemsType)
{
    magnet_points.clear();
    QList<QGraphicsItem *> under_items = this->items(pos);
    bool point_found = false;
    PaintPoint *magnet_point = nullptr;
    if (!(under_items.isEmpty()))
    {
        foreach(QGraphicsItem *item, under_items)
        {
            if(item && item->type() == PaintPoint::Type
                    && current_point != (magnet_point = dynamic_cast<PaintPoint *>(item))
                    && (ParentItemsType == -1 ||
                        (item->parentItem() && item->parentItem()->type() == ParentItemsType)))
            {
                if (!point_found) {
                    QPointF magnet_pos = magnet_point->scenePos();
                    *nodePos = magnet_pos;
                    point_found = true;
                }
                magnet_points.append(magnet_point);
            }
        }
    }
}

void PaintScene::setRotationItem(QGraphicsItem *item, QPointF center, qreal angle) {
    QTransform t;
    t.translate(center.x(), center.y());
    t.rotate(angle);
    t.translate(-center.x(), -center.y());
    item->setPos(t.map(item->pos())); // Магия
    item->setRotation(item->rotation() + angle);
    t.reset();
    item->update();
}

void PaintScene::removeItem(QGraphicsItem *item)
{
    QGraphicsScene::removeItem(item);
    emit removedItem(item);
}

void PaintScene::setPaintMod(paintMod mod)
{
    this->paint_mod = mod;
}

paintMod PaintScene::getPaintMod()
{
    return this->paint_mod;
}

