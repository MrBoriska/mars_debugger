#include "modeloptionseditor.h"
#include "ui_modeloptionseditor.h"

#include <cmath>

ModelOptionsEditor::ModelOptionsEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelOptionsEditor)
{
    ui->setupUi(this);

    modelConfig = ModelConfig::Instance();

    // set default values
    this->config.setValue("object/captures_count", ui->object_captures_count->value());

    ui->maxSpeed->setMaximum(1000);
    ui->maxSpeed->setMinimum(1);
    calcSpeedParams();
    ui->maxSpeed->setValue(modelConfig->step/(modelConfig->interval*k_slide));

    ui->sceneSize_w->setValue(modelConfig->getSceneSize().width());
    ui->sceneSize_h->setValue(modelConfig->getSceneSize().height());
}

ModelOptionsEditor::~ModelOptionsEditor()
{
    delete ui;
}

void ModelOptionsEditor::closeEvent(QCloseEvent *event)
{
    emit closed();
    QDialog::closeEvent(event);
}

void ModelOptionsEditor::calcSpeedParams()
{
    double s_max = modelConfig->step_max;
    double s_min = modelConfig->step_min;
    int i_max = modelConfig->interval_max;
    int i_min = modelConfig->interval_min;

    double speed_max = s_max/i_min;
    double speed_min = s_min/i_max;

    // step = k * interval + a, вычисление зависимости по двум конечным точкам
    k = (s_max - s_min)/(i_min - i_max);
    a = s_max - i_min*k;

    k_slide = (speed_max - speed_min)/(ui->maxSpeed->maximum()-ui->maxSpeed->minimum());
}

void ModelOptionsEditor::on_object_captures_count_valueChanged(int value)
{
    this->config.setValue("object/captures_count", value);
}

void ModelOptionsEditor::on_maxSpeed_valueChanged(int speed)
{
    // Устанавливаем соответсвия между значениями из виджета
    // и настоящим диапазоном изменения скорости
    double v = speed * k_slide;

    modelConfig->interval = floor(-(a/(k-v))+0.5);
    //modelConfig->step = v*a/(v-k);

    // корректировка шага
    modelConfig->step = v * modelConfig->interval;
}

void ModelOptionsEditor::on_sceneSize_w_valueChanged(int scene_width)
{
    modelConfig->setSceneSize(QSize(scene_width, modelConfig->getSceneSize().height()));
    emit sceneRectChanged(QRectF(modelConfig->getSceneBorderWidth(),
                                 modelConfig->getSceneBorderWidth(),
                                 modelConfig->getSceneSize().width(),
                                 modelConfig->getSceneSize().height()));
}

void ModelOptionsEditor::on_sceneSize_h_valueChanged(int scene_height)
{
    modelConfig->setSceneSize(QSize(modelConfig->getSceneSize().width(), scene_height));
    emit sceneRectChanged(QRectF(modelConfig->getSceneBorderWidth(),
                                 modelConfig->getSceneBorderWidth(),
                                 modelConfig->getSceneSize().width(),
                                 modelConfig->getSceneSize().height()));
}
