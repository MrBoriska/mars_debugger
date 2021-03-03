#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QActionGroup>

#include "paintscene.h"
#include "itemeditor.h"
#include "modelconfig.h"
#include "modeloptionseditor.h"
#include "modelanalisisviewer.h"

#include "controlsysservice.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    // Показывает сообщение в QStatusBar
    void showStatus(QString status);

signals:
    void changedGroupPos(GroupPos, QTime);

private slots:
    // Кнопки для работы с файлами карт
    void on_new_map_triggered();
    void on_open_map_triggered();
    void on_save_map_triggered();

    // Кнопки для добавления и редактирования элементов на карте
    void on_add_unit_toggled(bool checked);
    void on_add_obstacle_toggled(bool checked);
    void on_add_polygon_toggled(bool checked);
    void on_add_object_toggled(bool checked);
    void on_open_editor_toggled(bool checked);
    void on_add_track_toggled(bool checked);

    // Открытие окна настроек приложения и модели
    void on_open_options_toggled(bool checked);
    // Открытия окна с анализом модели
    void on_open_model_viewer_toggled(bool checked);
    // Управление запуском модели
    void on_start_model_triggered();

    // Обработчики событий добавления и удаления элементов с карты
    void addedItemOnScene(QGraphicsItem *);
    void removedItemOnScene(QGraphicsItem *);

    // Методы для сихронизации между моделью и отображением
    void on_reconnect_triggered();
    void syncModelAndView(GroupPos gpos, QTime time);
    //void showFinishSimulation();
    void showStopSimulation();
    void model_disconnect();
    void model_ready();
    void model_unready();
    void model_started();
    void model_started_error(QString error_str);
    void model_stop_trigger();
    void model_pause_trigger();

    void graphicScene_sceneRectChanged(QRectF rect);

    void on_plan_path_triggered();

    void on_reconnect_planner_triggered();
    
private:
    Ui::MainWindow *ui;
    PaintScene *graphicScene;
    QActionGroup *toolsGroup;
    ItemEditor *EditorDialog;
    ModelOptionsEditor *OptionsDialog;
    ModelAnalisisViewer *ModelViewer;
    QThread *modelThread;
    ModelConfig *modelConfig;
    QTimer *syncModelTimer;

    // Сетевой сервис для коммуникации с системой управленичя или ее моделью
    ControlSysService *controlSysService;
    bool MODEL_READY;

    void setTransformation(QGraphicsItem *item, ItemPos pos);
    void setPositionByGroupPos(QList<UnitItem *> units_items, ObjectItem *object_item, GroupPos positions);

    QList<QGraphicsEllipseItem *> real_trajectory_items;
    QList<QGraphicsEllipseItem *> target_trajectory_items;

};

#endif // MAINWINDOW_H
