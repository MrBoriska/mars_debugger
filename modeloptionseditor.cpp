#include "modeloptionseditor.h"
#include "ui_modeloptionseditor.h"

#include <cmath>

ModelOptionsEditor::ModelOptionsEditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelOptionsEditor)
{
    ui->setupUi(this);

    modelConfig = ModelConfig::Instance();

    ui->maxSpeed->setMaximum(1000);
    ui->maxSpeed->setMinimum(1);

    ui->maxSpeed->setValue(int(modelConfig->vel_max * 1000.0));

    ui->sceneSize_w->setValue(modelConfig->getSceneSize().width());
    ui->sceneSize_h->setValue(modelConfig->getSceneSize().height());

    // todo: need develop algoritm for adding and removing materials table
    ui->addMat->setEnabled(false);
    ui->removeMat->setEnabled(false);

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
    emit closed();
    QDialog::closeEvent(event);
}

void ModelOptionsEditor::on_maxSpeed_valueChanged(int speed)
{
    // Устанавливаем соответсвия между значениями из виджета
    // и настоящим диапазоном изменения скорости
    double v = double(speed) / 1000.0;

    // корректировка шага
    modelConfig->vel_max = v;
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
    // todo: update config materials list and validate!
    ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);
}

void ModelOptionsEditor::on_removeMat_clicked()
{
    // todo: update config materials list
    ui->tableWidget->removeRow(ui->tableWidget->currentRow());
}
