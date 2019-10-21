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
