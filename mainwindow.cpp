#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <QtGui>
#include <QString>
#include <QDebug>
#include <QMessageBox>

#include <ctime>

#include "unititem.h"
#include "paintlineitem.h"
#include "paintpolygonitem.h"
#include "objectitem.h"
#include "paintpoint.h"
#include "controlsysservice.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    syncModelTimer = 0;

    // Init Model
    modelConfig = ModelConfig::Instance();
    modelThread = 0;

    // Init Editor map
    EditorDialog = new ItemEditor(this);

    graphicScene = new PaintScene(this);
    graphicScene->EditorDialog = EditorDialog;
    graphicScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    modelConfig->sceneObject = graphicScene;

    ui->graphicsView->setScene(graphicScene);

    ui->graphicsView->setDragMode(QGraphicsView::NoDrag);
    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    ui->graphicsView->setViewportUpdateMode(QGraphicsView::SmartViewportUpdate);
    ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Делаем возможность выбора одновременно только одного инструмента
    toolsGroup = new QActionGroup(this);
    toolsGroup->addAction(ui->add_obstacle);
    toolsGroup->addAction(ui->add_polygon);
    toolsGroup->addAction(ui->add_unit);
    toolsGroup->addAction(ui->add_object);
    toolsGroup->addAction(ui->add_track);
    toolsGroup->addAction(ui->open_editor);
    toolsGroup->setExclusive(true);

    OptionsDialog = new ModelOptionsEditor(this);
    ModelViewer = new ModelAnalisisViewer(this);

    // Вешаем слот обновления статуса по сигналу от graphicScene
    connect(graphicScene, SIGNAL(changedStatus(QString)), this, SLOT(showStatus(QString)));
    connect(graphicScene, SIGNAL(addedItem(QGraphicsItem *)), this, SLOT(addedItemOnScene(QGraphicsItem *)));
    connect(graphicScene, SIGNAL(removedItem(QGraphicsItem *)), this, SLOT(removedItemOnScene(QGraphicsItem *)));
    connect(graphicScene, SIGNAL(sceneRectChanged(QRectF)), this, SLOT(graphicScene_sceneRectChanged(QRectF)));

    // События закрытия доп. окон
    connect(EditorDialog, SIGNAL(closed()), ui->open_editor, SLOT(toggle()));
    connect(OptionsDialog, SIGNAL(closed()), ui->open_options, SLOT(toggle()));
    connect(ModelViewer, SIGNAL(closed()), ui->open_model_viewer, SLOT(toggle()));

    // OptionsEditor
    connect(OptionsDialog, SIGNAL(sceneRectChanged(QRectF)), graphicScene, SLOT(sceneRectShow(QRectF)));

    // Устанавливаем размер graphicScene
    graphicScene->setSceneRect(modelConfig->getSceneBorderWidth(),
                               modelConfig->getSceneBorderWidth(),
                               modelConfig->getSceneSize().width(),
                               modelConfig->getSceneSize().height());

    // Инициализация соединения с системой группового управления
    MODEL_READY = false;
    controlSysService = new ControlSysService(this);
    controlSysService->init(); // todo: maybe async call need
    // Сигналы готовности системы управления к запуску
    connect(controlSysService, SIGNAL(cs_ready()), this, SLOT(model_ready()));
    connect(controlSysService, SIGNAL(cs_unready()), this, SLOT(model_unready()));
    // Сигналы для передачи и обновления начальных данных в системе управления
    /*connect(modelConfig, &ModelConfig::changed_start_pos,
            this, [=](GroupPos gpos){controlSysService->set_start_pos(gpos);});
    connect(modelConfig, &ModelConfig::changed_track_path,
            this, [=](QPainterPath path){controlSysService->set_track_path(path);});*/
}

void MainWindow::graphicScene_sceneRectChanged(QRectF rect) {
    // Устанавливаем размер graphicsView
    ui->graphicsView->setSceneRect(0,0,
                                   rect.width()+2*modelConfig->getSceneBorderWidth(),
                                   rect.height()+2*modelConfig->getSceneBorderWidth());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showStatus(QString status)
{
    ui->statusBar->showMessage(status, 2000);
}

void MainWindow::on_new_map_triggered()
{
    ui->statusBar->showMessage("Создание новой карты...", 2000);
}

void MainWindow::on_open_map_triggered()
{
    ui->statusBar->showMessage("Открытие карты...", 2000);
}

void MainWindow::on_save_map_triggered()
{
    ui->statusBar->showMessage("Сохранение карты...", 2000);
}

void MainWindow::on_add_obstacle_toggled(bool checked)
{
    graphicScene->setPaintMod(ObstacleMOD);
    ui->statusBar->showMessage("Режим рисования препятствия: "
                               + QString::number((int)checked), 2000);
}

void MainWindow::on_add_polygon_toggled(bool checked)
{
    graphicScene->setPaintMod(PolygonMOD);
    ui->statusBar->showMessage("Режим рисования полигона: "
                               + QString::number((int)checked), 2000);
}

void MainWindow::on_add_object_toggled(bool checked)
{
    graphicScene->setPaintMod(ObjectMOD);
    ui->statusBar->showMessage("Режим расстановки обьектов управления: "
                               + QString::number((int)checked), 2000);
}

void MainWindow::on_add_unit_toggled(bool checked)
{
    // Предварительная загрузка картинки юнита
    QPixmap(":/pictures/icons/unit.png");

    graphicScene->setPaintMod(UnitMOD);
    ui->statusBar->showMessage("Режим расстановки юнитов: "
                               + QString::number((int)checked), 2000);
}

void MainWindow::on_add_track_toggled(bool checked)
{
    graphicScene->setPaintMod(TrackMOD);
    ui->statusBar->showMessage("Режим задания траектории: "
                               + QString::number((int)checked), 2000);
}

void MainWindow::on_open_editor_toggled(bool checked)
{
    if (checked) {
        graphicScene->setPaintMod(EditMOD);
        ui->graphicsView->setDragMode(QGraphicsView::RubberBandDrag);

        foreach(QGraphicsItem *item, graphicScene->items())
        {
            if (PaintItem *item_ = dynamic_cast<PaintItem*>(item)) {
                if (item_->isEditable())
                {
                    item->setFlags(QGraphicsItem::ItemIsMovable
                                   | QGraphicsItem::ItemIsSelectable);
                    //item->setAcceptDrops(true);
                }
                item->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

                //item->setAcceptHoverEvents(true);

                connect(item_, SIGNAL(selectChanged(QGraphicsItem *)),
                        EditorDialog, SLOT(selectChangeItem(QGraphicsItem *)));
            }
            item->update();
        }
        graphicScene->showItemEditor();

    } else {
        graphicScene->setPaintMod(OffMOD);
        ui->graphicsView->setDragMode(QGraphicsView::NoDrag);

        foreach(QGraphicsItem *item, graphicScene->items())
        {
            if (PaintItem *item_ = dynamic_cast<PaintItem*>(item)) {
                item->setFlags(item->flags()
                               & ~QGraphicsItem::ItemIsMovable
                               & ~QGraphicsItem::ItemIsSelectable
                               & ~QGraphicsItem::ItemSendsGeometryChanges);

                //item->setAcceptDrops(false);
                //item->setAcceptHoverEvents(false);

                disconnect(item_, SIGNAL(selectChanged(QGraphicsItem *)),
                        EditorDialog, SLOT(selectChangeItem(QGraphicsItem *)));
            }
            item->update();
        }
        graphicScene->hideItemEditor();
    }


    ui->statusBar->showMessage("Режим редактирования элементов: "
                               + QString::number((int)checked), 2000);
}


void MainWindow::addedItemOnScene(QGraphicsItem *)
{

}

void MainWindow::removedItemOnScene(QGraphicsItem *)
{

}

void MainWindow::model_ready()
{
    if (!MODEL_READY) {
        ui->start_model->setEnabled(true);
        ui->pause_model->setEnabled(false);
        ui->stop_model->setEnabled(false);

        MODEL_READY = true;
    }
}
void MainWindow::model_unready()
{
    if (MODEL_READY) {
        ui->start_model->setEnabled(false);
        ui->pause_model->setEnabled(false);
        ui->stop_model->setEnabled(false);

        MODEL_READY = false;
    }
}
void MainWindow::on_start_model_triggered()
{

    if (modelConfig->getTrack() == 0
         || modelConfig->getObject() == 0
         || modelConfig->getUnits().isEmpty())
    {
        QMessageBox::warning(
            (QWidget *)(this),
            "info",
            "Для запуска модели необходимо добавить обьект, роботов и траекторию"
        );
        return;
    }


    ui->statusBar->showMessage("Отправляем начальное положение роботов и обьекта...");
    modelConfig->setStartPosition();

    controlSysService->set_config_data(modelConfig);
    controlSysService->set_start_pos(modelConfig->getStartPosition());
    ui->statusBar->showMessage("Отправляем траекторию движения обьекта...");
    controlSysService->set_track_path(modelConfig->getTrackPath());


    ui->statusBar->showMessage("Запуск...");
    connect(controlSysService, SIGNAL(cs_started()), this, SLOT(model_started()));
    connect(controlSysService, SIGNAL(cs_started_error(QString)), this, SLOT(model_started_error(QString)));
    connect(controlSysService, SIGNAL(cs_stopped()), this, SLOT(showStopSimulation()));
    controlSysService->start_request();

    ui->start_model->setEnabled(false);
    ui->pause_model->setEnabled(false);
    ui->stop_model->setEnabled(false);
}

void MainWindow::model_started()
{
    ui->statusBar->showMessage("Текущее время моделирования: 0");

    // Запуск синхронизации выходных данных системы управления и виртуальной сцены
    if (syncModelTimer == 0) {
        syncModelTimer = new QTimer(this);
        //connect(syncModelTimer, SIGNAL(timeout()), this, SLOT(syncModelAndView()));
        connect(syncModelTimer, &QTimer::timeout, [=](){
            // Отправляем запрос на получение положения роботов и обьекта(с привязкой ко времени в модели)
            controlSysService->get_pos_request();
        });
        // Перехватываем ответ на запрос с положениями
        connect(controlSysService, SIGNAL(cs_current_pos(GroupPos, QTime)),
                this,              SLOT(syncModelAndView(GroupPos, QTime)));

    }
    // Запуск опрашивателя модели
    syncModelTimer->setInterval(modelConfig->view_interval);
    syncModelTimer->start();


    // вывод результатов работы модели пользователю(графики)
    if (ui->open_model_viewer->isChecked()) {
        ModelViewer->startAnalisis(modelConfig->getUnits().length());
        connect(this, SIGNAL(changedGroupPos(GroupPos, QTime)),
                ModelViewer, SLOT(async_addState(GroupPos, QTime)),
                Qt::QueuedConnection);
    }

    // Настройка кнопок управления моделированием
    ui->start_model->setEnabled(false);
    ui->pause_model->setEnabled(true);
    ui->stop_model->setEnabled(true);
    connect(ui->pause_model, SIGNAL(triggered(bool)), this, SLOT(model_pause_trigger()));
    connect(ui->stop_model, SIGNAL(triggered(bool)), this, SLOT(model_stop_trigger()));
}

void MainWindow::model_pause_trigger()
{
    controlSysService->send_pause_request();
}
void MainWindow::model_stop_trigger()
{
    controlSysService->send_stop_request();
}
void MainWindow::model_started_error(QString error_str)
{
    ui->statusBar->showMessage(
        "Ошибка при запуске моделирования: " + error_str,
        10000
    );
    showStopSimulation();
}

void MainWindow::showStopSimulation()
{
    qDebug() << "MainWindow::showStopSimulation()";

    if (syncModelTimer != 0) {
        syncModelTimer->stop();
        //delete syncModelTimer;
        //syncModelTimer = 0;
    }

    // set initial position
    setPositionByGroupPos(
        // Список управляемых моделью роботов
        modelConfig->getUnits(),
        // Обьект обьекта управления
        modelConfig->getObject(),
        // Список координат каждого из роботов
        modelConfig->getStartPosition()
    );
    ui->statusBar->showMessage("Текущее время моделирования: 0");

    // Элементы управления
    ui->start_model->setEnabled(true);
    ui->pause_model->setEnabled(false);
    ui->pause_model->setChecked(false);
    ui->stop_model->setEnabled(false);

    // Обьекты не удаляются, поэтому надо принудительно.
    disconnect(ui->pause_model, SIGNAL(triggered(bool)), this, SLOT(model_pause_trigger()));
    disconnect(ui->stop_model, SIGNAL(triggered(bool)), this, SLOT(model_stop_trigger()));
    disconnect(ui->stop_model, SIGNAL(triggered(bool)), this, SLOT(showStopSimulation()));
    disconnect(this, SIGNAL(changedGroupPos(GroupPos, QTime)),
               ModelViewer, SLOT(async_addState(GroupPos, QTime)));

    disconnect(controlSysService, SIGNAL(cs_started()), this, SLOT(model_started()));
    disconnect(controlSysService, SIGNAL(cs_started_error(QString)), this, SLOT(model_started_error(QString)));
    disconnect(controlSysService, SIGNAL(cs_stopped()), this, SLOT(showStopSimulation()));

}

void MainWindow::syncModelAndView(GroupPos gpos, QTime time)
{
    if (!syncModelTimer->isActive())
        return;

    setPositionByGroupPos(
        // Список управляемых моделью роботов
        modelConfig->getUnits(),
        // Обьект обьекта управления
        modelConfig->getObject(),
        // Список координат каждого из роботов
        gpos
    );

    ui->statusBar->showMessage(
        "Текущее время моделирования: "
        + time.toString("hh:mm:ss.zzz")
    );



    QGraphicsEllipseItem *tp = new QGraphicsEllipseItem();
    double R = 3;
    tp->setRect(-R,-R,2*R,2*R);
    tp->setPos(gpos.object_pos.pos.x,gpos.object_pos.pos.y);
    tp->setZValue(10);
    graphicScene->addItem(tp);



    // Испускаем сигнал для анализа перемещения
    emit changedGroupPos(gpos, time);
}

void MainWindow::setPositionByGroupPos(
        QList<UnitItem *> units_items,
        ObjectItem* object_item,
        GroupPos positions
){

    // Отразим координаты роботов на сцене
    for(int i = 0; i < positions.robots_pos.size(); ++i) {
        setTransformation(units_items[i], positions.robots_pos[i].pos);
    }

    // Отразим координаты обьекта управления
    ItemPos object_pos = positions.object_pos.pos;
    setTransformation(object_item, object_pos);
}

void MainWindow::setTransformation(QGraphicsItem *item, ItemPos pos)
{
    QTransform t;

    // Определяем точку привязки трансформаций
    QPointF loc_center = item->scenePos();

    // Осуществляем перемещение робота
    if ( (pos.x-loc_center.x()) || (pos.y-loc_center.y()) ) {
        // матрица переноса
        item->setPos(pos.x, pos.y);
    }

    // Обновляем точку привязки трансформаций
    loc_center = item->scenePos();

    double alfa = pos.alfa * 180/M_PI;

    // Осуществляем поворот робота
    if ( (alfa - item->rotation()) ) {
        // матрица поворота
        t.translate(loc_center.x(), loc_center.y());
        t.rotate(alfa - item->rotation());
        t.translate(-loc_center.x(), -loc_center.y());
    }

    // Если результирующая матрица не единичная, то применим трансформации
    if (!t.isIdentity()) {
        // Применим полученное значение результирующей позиции
        //(увеличивает стабильность преобразования)
        item->setPos(t.map(item->pos()));
        // Повернем элемент
        item->setRotation(alfa);
        item->update();
        t.reset();
    }
}

void MainWindow::on_open_options_toggled(bool checked)
{
    if (checked) {
        OptionsDialog->show();
    } else {
        OptionsDialog->hide();
    }
}

void MainWindow::on_open_model_viewer_toggled(bool checked)
{
    if (checked) {
        ModelViewer->show();
    } else {
        ModelViewer->hide();
    }
}
