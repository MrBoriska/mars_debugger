#ifndef MODELOPTIONSEDITOR_H
#define MODELOPTIONSEDITOR_H

#include <QDialog>
#include <QSettings>

#include <QTableWidgetItem>

#include "modelconfig.h"

namespace Ui {
class ModelOptionsEditor;
}

class ModelOptionsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ModelOptionsEditor(QWidget *parent = nullptr);
    ~ModelOptionsEditor();

private slots:

    void on_maxSpeed_valueChanged(int speed);

    void on_sceneSize_w_valueChanged(int scene_width);

    void on_sceneSize_h_valueChanged(int scene_height);

    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_addMat_clicked();

    void on_removeMat_clicked();

    void on_step_valueChanged(int value);

    void on_step_spin_valueChanged(double value);

    void on_interval_valueChanged(int value);

    void on_interval_spin_valueChanged(double value);

    void on_trajectory_P_spin_valueChanged(double arg1);

    void on_trajectory_w_thres_offset_valueChanged(double arg1);

    void on_trajectory_w_thres_valueChanged(double arg1);

    void on_trajectory_wI_valueChanged(double arg1);

    void on_trajectory_Pv_valueChanged(double arg1);

    void on_modelAddress_editingFinished();

signals:
    void sceneRectChanged(QRectF);
    void closed();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::ModelOptionsEditor *ui;
    ModelConfig *modelConfig;

    double k;
    double a;
    double k_slide;
};

#endif // MODELOPTIONSEDITOR_H
