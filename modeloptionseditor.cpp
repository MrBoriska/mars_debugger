#include "modeloptionseditor.h"
#include "ui_modeloptionseditor.h"

#include <cmath>

ModelOptionsEditor::ModelOptionsEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelOptionsEditor)
{
    ui->setupUi(this);

    modelConfig = ModelConfig::Instance();


    ui->sceneSize_w->setValue(modelConfig->getSceneSize().width());
    ui->sceneSize_h->setValue(modelConfig->getSceneSize().height());

    ui->modelAddress->setText(modelConfig->getModelAddress());

    ui->step_spin->setValue(modelConfig->step);
    ui->interval->setValue(modelConfig->view_interval); // ms -> sec

    ui->robot_vmax->setValue(modelConfig->robot_vmax);
    ui->robot_wmax->setValue(modelConfig->robot_wmax);
    ui->robot_amax->setValue(modelConfig->robot_amax);
    ui->trajectory_v_P->setValue(modelConfig->trajectory_v_P);
    ui->trajectory_v_I->setValue(modelConfig->trajectory_v_I);
    ui->trajectory_v_D->setValue(modelConfig->trajectory_v_D);
    ui->trajectory_w_P->setValue(modelConfig->trajectory_w_P);
    ui->trajectory_w_I->setValue(modelConfig->trajectory_w_I);
    ui->trajectory_w_D->setValue(modelConfig->trajectory_w_D);
    ui->trajectory_v_thres->setValue(modelConfig->trajectory_v_thres);
    ui->trajectory_w_thres->setValue(modelConfig->trajectory_w_thres);

    // todo: need develop algoritm for adding and removing materials table
    //ui->addMat->setEnabled(false);
    //ui->removeMat->setEnabled(false);

    // Create materials table from modelConfig->materials list
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(4);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList() << "Название" << "Цвет" << "Норм. уск." << "Танг. уск.");
    ui->tableWidget->setRowCount(modelConfig->materials.count());
    for(int i=0;i<modelConfig->materials.count();i++) {
        ItemMaterial mat = modelConfig->materials[i];

        QTableWidgetItem *title_itm = new QTableWidgetItem();
        title_itm->setText(mat.title);
        ui->tableWidget->setItem(i,0,title_itm);

        QTableWidgetItem *color_itm = new QTableWidgetItem();
        color_itm->setText(mat.color.name());
        color_itm->setBackgroundColor(mat.color);
        ui->tableWidget->setItem(i,1,color_itm);

        QTableWidgetItem *accn_itm = new QTableWidgetItem();
        accn_itm->setText(QString::number(mat.accn_max));
        ui->tableWidget->setItem(i,2,accn_itm);

        QTableWidgetItem *acct_itm = new QTableWidgetItem();
        acct_itm->setText(QString::number(mat.acct_max));
        ui->tableWidget->setItem(i,3,acct_itm);
    }
}

ModelOptionsEditor::~ModelOptionsEditor()
{
    delete ui;
}

void ModelOptionsEditor::closeEvent(QCloseEvent *event)
{
    QDialog::closeEvent(event);
    emit closed();
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

void ModelOptionsEditor::on_tableWidget_itemChanged(QTableWidgetItem *item)
{
    if (item->column() == 0) {
        // Название
        modelConfig->materials[item->row()].title = item->text();

    } else if (item->column() == 1) {
        // Цвет

        QColor color(item->text());
        if (!(color.isValid())) {
            return;
        }

        item->setBackgroundColor(color);

        foreach(QGraphicsItem *m_itm, modelConfig->sceneObject->items()) {
            if (m_itm->type() == PaintPolygonItem::Type) {
                PaintPolygonItem *mp_itm = dynamic_cast<PaintPolygonItem*>(m_itm);
                if (!(mp_itm->isObstacle()) && mp_itm->brush().color() == modelConfig->materials[item->row()].color) {
                    mp_itm->setBrush(QBrush(item->backgroundColor()));
                }
            }
        }
        modelConfig->materials[item->row()].color = item->backgroundColor();

    } else if (item->column() == 2) {
        // норм. ускорение
        modelConfig->materials[item->row()].accn_max = item->text().toDouble();

    } else if (item->column() == 3) {
        // тангенциальное ускорение
        modelConfig->materials[item->row()].acct_max = item->text().toDouble();
    }
}

void ModelOptionsEditor::on_addMat_clicked()
{
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
    ItemMaterial mat;
    modelConfig->materials.append(mat);
}

void ModelOptionsEditor::on_removeMat_clicked()
{
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
    modelConfig->materials.removeAt(ui->tableWidget->currentRow());
}

void ModelOptionsEditor::on_step_valueChanged(int value)
{
    ui->step_spin->setValue(double(value)/1000.0);
}

void ModelOptionsEditor::on_step_spin_valueChanged(double value)
{
    ui->step->setValue(int(1000*value));
    modelConfig->step = value;
}

void ModelOptionsEditor::on_interval_valueChanged(int value)
{
    ui->interval_spin->setValue(double(value)/1000.0); // ms -> sec
    modelConfig->view_interval = value;
}

void ModelOptionsEditor::on_interval_spin_valueChanged(double value)
{
    ui->interval->setValue(int(1000.0*value)); // sec -> ms
}

void ModelOptionsEditor::on_modelAddress_editingFinished()
{
    modelConfig->setModelAddress(ui->modelAddress->text());
}

void ModelOptionsEditor::on_robot_vmax_editingFinished()
{
    modelConfig->robot_vmax = ui->robot_vmax->value();
}

void ModelOptionsEditor::on_robot_wmax_editingFinished()
{
    modelConfig->robot_wmax = ui->robot_wmax->value();
}

void ModelOptionsEditor::on_robot_amax_editingFinished()
{
    modelConfig->robot_amax = ui->robot_amax->value();
}

void ModelOptionsEditor::on_trajectory_v_thres_editingFinished()
{
    modelConfig->trajectory_v_thres = ui->trajectory_v_thres->value();
}

void ModelOptionsEditor::on_trajectory_w_thres_editingFinished()
{
    modelConfig->trajectory_w_thres = ui->trajectory_w_thres->value();
}

void ModelOptionsEditor::on_trajectory_v_P_editingFinished()
{
    modelConfig->trajectory_v_P = ui->trajectory_v_P->value();
}

void ModelOptionsEditor::on_trajectory_v_I_editingFinished()
{
    modelConfig->trajectory_v_I = ui->trajectory_v_I->value();
}

void ModelOptionsEditor::on_trajectory_v_D_editingFinished()
{
    modelConfig->trajectory_v_D = ui->trajectory_v_D->value();
}

void ModelOptionsEditor::on_trajectory_w_P_editingFinished()
{
    modelConfig->trajectory_w_P = ui->trajectory_w_P->value();
}

void ModelOptionsEditor::on_trajectory_w_I_editingFinished()
{
    modelConfig->trajectory_w_I = ui->trajectory_w_I->value();
}

void ModelOptionsEditor::on_trajectory_w_D_editingFinished()
{
    modelConfig->trajectory_w_D = ui->trajectory_w_D->value();
}
