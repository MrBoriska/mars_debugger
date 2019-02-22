#ifndef MODELOPTIONSEDITOR_H
#define MODELOPTIONSEDITOR_H

#include <QDialog>
#include <QSettings>

#include "modelconfig.h"

namespace Ui {
class ModelOptionsEditor;
}

class ModelOptionsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ModelOptionsEditor(QWidget *parent = 0);
    ~ModelOptionsEditor();

private slots:
    void on_object_captures_count_valueChanged(int arg1);

    void on_maxSpeed_valueChanged(int speed);

    void on_sceneSize_w_valueChanged(int scene_width);

    void on_sceneSize_h_valueChanged(int scene_height);

signals:
    void sceneRectChanged(QRectF);
    void closed();

protected:
    void closeEvent(QCloseEvent *event);

private:
    Ui::ModelOptionsEditor *ui;
    QSettings config;
    ModelConfig *modelConfig;

    void calcSpeedParams();
    double k;
    double a;
    double k_slide;
};

#endif // MODELOPTIONSEDITOR_H
