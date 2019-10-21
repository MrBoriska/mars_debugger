#ifndef ITEMEDITOR_H
#define ITEMEDITOR_H

#include <QDialog>
#include <QtCore>
#include <QtGui>
#include <QGraphicsItem>

#include "paintlineitem.h"
#include "paintpolygonitem.h"

namespace Ui {
class ItemEditor;
}

/**
 * @brief The ItemEditor class
 */
class ItemEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ItemEditor(QWidget *parent = nullptr);
    ~ItemEditor();

    QList<QGraphicsItem *> getSelectedItems();

signals:
    void closed();

public slots:
    /**
     * @brief Принимает событие выбора элемента на сцене
     *        для последующего редактирования этого элемента
     */
    void selectChangeItem(QGraphicsItem *);

private slots:
    /**
     * @brief принимает событие изменения состояния виджета редактирования поворота элемента
     * @param value - значение заданного угла поворота элемента
     */
    void on_dial_valueChanged(int value);

    /**
     * @brief принимает событие нажатия кнопки удаления выбранных элементов
     */
    void on_deleteButton_clicked();

    /**
     * @brief принимает событие изменения материала для выбранных элементов
     * @param index - индекс выбранного материала
     */
    void on_materialBox_currentIndexChanged(int index);

    /**
     * @brief реагирует на событие о изменении центра вращения группы выбранных элементов
     */
    void updateCenterOfRotation();

protected:
    /**
     * @brief обновляет виджет выбора материала для выбранных элементов
     */
    void updateMaterialSelect();

private:
    Ui::ItemEditor *ui;

    // Выбранные элементы
    QList<QGraphicsItem *> selected_items;

    // Позиция центра вращения выбранных элементов
    QPointF center;

    // Переменная предыдущего угла, до изменения
    // необходима для вычисления разницы
    // между требуемым углом и уже имеющимся у элемента
    int prev_angle;

    void closeEvent(QCloseEvent *event);
};

#endif // ITEMEDITOR_H
