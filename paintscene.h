#ifndef PAINTSCENE_H
#define PAINTSCENE_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>

#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QTimer>
#include <QDebug>
#include <QString>

#include "itemeditor.h"
#include "unititem.h"
#include "paintpolygonitem.h"
#include "objectitem.h"
#include "paintpoint.h"
#include "trackitem.h"

// Определяет список режимов создания нового элемента
enum paintMod {
    OffMOD=0,
    EditMOD=1,
    ObstacleMOD=2,
    PolygonMOD=3,
    ObjectMOD=4,
    UnitMOD=5,
    TrackMOD=6
};

class PaintScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit PaintScene(QObject *parent = nullptr);
    ~PaintScene();

    // Текущие значения стилей границ и заполнения элементов
    QPen pen;
    QBrush brush;

    // Окно редактирования элементов уже после их создания
    ItemEditor *EditorDialog;

    // Открытие/закрытие редактора элементов
    void showItemEditor();
    void hideItemEditor();

    // Задание режима редактирования
    void setPaintMod(paintMod mod);
    paintMod getPaintMod();

    // Устанавливает угол поворота для элемента сцены
    void setRotationItem(QGraphicsItem *item, QPointF center, qreal angle);

signals:
    // Для вывода сообщений в QStatusBar основного окна
    void changedStatus(QString);

    void addedItem(QGraphicsItem *);
    void removedItem(QGraphicsItem *);

public slots:
    void sceneRectShow(const QRectF &rect);

private:
    // Определяет режим создания нового элемента
    paintMod paint_mod;

    PaintLineItem *line_item;
    PaintPolygonItem *polygon_item;
    UnitItem *unit_item;
    double prev_angle;
    ObjectItem *object_item;
    TrackItem *track_item;
    PaintPoint *current_point;
    QList<PaintPoint *> magnet_points;

    PaintPolygonItem* basePolygon;
    PaintPolygonItem* baseBorderPolygon;

    // Осуществялет поиск magnet_point в указанной позиции
    // Запоминает его. А положение записывает в nodePos
    void fsetMagnetPointPos(QPointF pos, QPointF *nodePos, const int ParentItemsType = -1);

/*
 * Переопределенные методы из QGraphicsScene
 */
public:
    // For "emit removedItem()" signal
    void removeItem(QGraphicsItem *);
    bool isBasePolygon(PaintPolygonItem *p);
    void safeClear();
private:
    // Для рисования используем события мыши
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *);
    void keyPressEvent(QKeyEvent *);
};

#endif // PAINTSCENE_H
