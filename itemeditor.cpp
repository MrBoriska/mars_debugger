#include "itemeditor.h"
#include "ui_itemeditor.h"

#include "QDebug"
#include "QString"

#include "mainwindow.h"
#include "modelconfig.h"

ItemEditor::ItemEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemEditor)
{
    ui->setupUi(this);

    // В начальный момент времени не может быть выделенных элементов,
    // поэтому окно должно быть неактивно
    setEnabled(false);

    prev_angle = 0;

    // Меню материала
    ui->materialBox->addItem("ничего не выбрано");
    foreach(ItemMaterial mat, ModelConfig::Instance()->materials) {
        QPixmap ico(25, 25); ico.fill(mat.color);
        ui->materialBox->addItem(QIcon(ico), mat.title, mat.color);
    }
}

ItemEditor::~ItemEditor()
{
    delete ui;
}

void ItemEditor::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    emit closed();
}

void ItemEditor::on_dial_valueChanged(int angle)
{
    ui->groupBox->setTitle("Угол: "+QString::number(angle));

    // Если мы уже поменяли угол, то завершаем выполнение
    if (prev_angle == angle) return;

    bool is_qroup_center = ui->doGroupCenter->isChecked();

    QTransform t;
    if (is_qroup_center) {
        // вращение вокруг общего центра(вычисляется в updateCenterOfRotation() )
        t.translate(center.x(), center.y());
        t.rotate(angle - prev_angle);
        t.translate(-center.x(), -center.y());
        foreach(QGraphicsItem* item, selected_items) {
            item->setPos(t.map(item->pos())); // Магия
            item->setRotation(item->rotation() + angle - prev_angle);
            item->update();
            // дополнительно вызываем событие изменения позиции, для правильного завершения обработчиков
            if (item->type() == PaintPolygonItem::Type)
                (dynamic_cast<PaintPolygonItem*>(item))->pos_changed();
        }
    } else {
        // вращение каждого элемента вокруг своего центра отдельно
        QPointF loc_center;
        foreach(QGraphicsItem* item, selected_items) {
            loc_center = item->mapToScene(item->boundingRect()).boundingRect().center();
            t.translate(loc_center.x(), loc_center.y());
            t.rotate(angle - prev_angle);
            t.translate(-loc_center.x(), -loc_center.y());
            item->setPos(t.map(item->pos())); // Магия
            item->setRotation(item->rotation() + angle - prev_angle);
            t.reset();
            item->update();
            // дополнительно вызываем событие изменения позиции, для правильного завершения обработчиков
            if (item->type() == PaintPolygonItem::Type)
                (dynamic_cast<PaintPolygonItem*>(item))->pos_changed();
        }
    }



    prev_angle = angle;
}

// todo:: добавить ее вызов при изменении положения selected_items
void ItemEditor::updateCenterOfRotation()
{
    QRectF selected_rect;
    foreach(QGraphicsItem *item, selected_items) {
        selected_rect |= item->mapToScene(item->boundingRect()).boundingRect();
    }
    center = selected_rect.center();
}

// определяет необходимость выбора материала, а так-же его значение
void ItemEditor::updateMaterialSelect()
{
    int use_materials = 0;
    QColor common_color = QColor();
    QColor select_color = QColor();
    int equal_colors_count = 1;
    foreach(QGraphicsItem *item, selected_items) {
        if (item->type() == PaintPolygonItem::Type && !( (dynamic_cast<PaintPolygonItem*>(item))->isObstacle() )) {
            use_materials++;
            select_color = (dynamic_cast<PaintPolygonItem*>(item))->getMaterialColor();

            if (common_color == select_color) {
                equal_colors_count++;
            }
            common_color = select_color;
        }
    }

    if (use_materials) {
        ui->materialBox->setEnabled(true);

        int index = ui->materialBox->findData(common_color);
        if (index == -1) index = 0;
        if (equal_colors_count != use_materials) index = 0;
        ui->materialBox->blockSignals(true);
        ui->materialBox->setCurrentIndex(index);
        ui->materialBox->blockSignals(false);
    } else {
        ui->materialBox->setEnabled(false);
    }
}

void ItemEditor::selectChangeItem(QGraphicsItem * item)
{
    if (!item->isSelected()) {
        selected_items.append(item);
    } else {
        selected_items.removeOne(item);
    }

    updateCenterOfRotation();
    updateMaterialSelect();

    // и заодно обнулим редактор угла поворота
    prev_angle = 0;
    ui->dial->setValue(0);


    if (!selected_items.size() && isEnabled()) {
        setEnabled(false);
    } else if (selected_items.size() && !isEnabled()) {
        setEnabled(true);
    }

}

void ItemEditor::on_deleteButton_clicked()
{

    QList<QGraphicsItem *> main_items;
    foreach(QGraphicsItem* item, selected_items) {
        if (item->type() == PaintPoint::Type) {
            item->setSelected(false);
            item->scene()->removeItem(item);
            delete item;
        } else {
            main_items.append(item);
        }
    }

    foreach(QGraphicsItem* item, main_items) {
        item->setSelected(false);
        item->scene()->removeItem(item);
        delete item;
    }

}

void ItemEditor::on_materialBox_currentIndexChanged(int index)
{
    // todo: определить действия при выборе "ничего не выбрано"
    if (index < 1) return;

    foreach(QGraphicsItem *item, selected_items) {
        if (item->type() == PaintPolygonItem::Type) {
            (dynamic_cast<PaintPolygonItem*>(item))->setMaterialColor(
                ModelConfig::Instance()->materials.at(index-1).color
            );
        }
    }

}

QList<QGraphicsItem*> ItemEditor::getSelectedItems() {
    return selected_items;
}
