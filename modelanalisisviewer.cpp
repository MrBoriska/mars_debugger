#include "modelanalisisviewer.h"
#include "ui_modelanalisisviewer.h"

#include <QDateTime>

#include <QFileDialog>

ModelAnalisisViewer::ModelAnalisisViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelAnalisisViewer)
{
    ui->setupUi(this);

    ui->Plot_1->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);
    ui->Plot_2->axisRect()->insetLayout()->setInsetAlignment(0, Qt::AlignTop|Qt::AlignLeft);

    // Настройки навигации по графику
    ui->Plot_1->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->Plot_2->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //ui->Angle->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    this->setWindowFlags(Qt::Window);

    // Настройка доступных к отображению типов графиков
    ui->typeSelectBox->clear();
    ui->typeSelectBox->insertItems(0, this->types_list);
    ui->typeSelectBox_2->clear();
    ui->typeSelectBox_2->insertItems(0, this->types_list);
    ui->axisSelectBox->clear();
    ui->axisSelectBox->insertItems(0, this->axis_list);
    ui->axisSelectBox_2->clear();
    ui->axisSelectBox_2->insertItems(0, this->axis_list);

    // Init vars foracceleration calculation
    prev_time = 0;
}

void ModelAnalisisViewer::closeEvent(QCloseEvent *event)
{
    emit closed();
    QDialog::closeEvent(event);
}

void ModelAnalisisViewer::setAxisNames(PlotType plot_type, QCustomPlot * plot)
{
    // Название осей
    plot->xAxis->setLabel("Time, t[ms]");
    switch (plot_type) {
        case V_LIN:
            plot->yAxis->setLabel("Linear velocity, [m/s]");
            break;
        case V_ANG:
            plot->yAxis->setLabel("Angular velocity, [m/s]");
            break;
        case ACC_LIN:
            plot->yAxis->setLabel("Linear acceleration, [m/s^2]");
            break;
        case ACC_ANG:
            plot->yAxis->setLabel("Angular acceleration, [rad/s^2]");
            break;
        case POS:
            plot->yAxis->setLabel("Position, [m]");
            break;
        case ERR_POS:
            plot->yAxis->setLabel("Position error, [m]");
            break;
        case ERR_VEL_X:
            plot->yAxis->setLabel("Linear velocity error, [m/s]");
            break;
        case ERR_VEL_Z:
            plot->yAxis->setLabel("Angular velocity error, [m/s]");
            break;
    }

}

void ModelAnalisisViewer::startAnalisis(int units_count)
{
    ui->Plot_1->clearGraphs();
    ui->Plot_2->clearGraphs();

    // Generate Axis names by selected data types
    this->setAxisNames(PlotType(ui->typeSelectBox->currentIndex()), ui->Plot_1);
    this->setAxisNames(PlotType(ui->typeSelectBox_2->currentIndex()), ui->Plot_2);

    // цвета с номерами 0 - 19 соответсвуют глобальным цветам Qt::GlobalColor
    // нам интересны только цвета в диапазоне 7-18
    int color = 7;
    QPen unit_pen;
    QString name_graph;

    for (int i=0;i<units_count;i++) {
        unit_pen = QPen(static_cast<Qt::GlobalColor>(color));
        name_graph = "Robot " + QString::number(i+1);

        if (i % 2)
            unit_pen.setStyle(Qt::DotLine);

        ui->Plot_1->addGraph()->setPen(unit_pen);
        ui->Plot_2->addGraph()->setPen(unit_pen);
        ui->Plot_1->graph(i)->setName(name_graph);
        ui->Plot_2->graph(i)->setName(name_graph);
        //ui->Xaxis->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);
        //ui->Yaxis->graph(i)->setScatterStyle(QCPScatterStyle::ssDisc);

        color += 2;
        if (color > 18) color = 7;
    }

    ui->Plot_1->legend->setVisible(true);
    ui->Plot_2->legend->setVisible(true);

    ui->Plot_1->yAxis->setRange(0, 0.00000001);
    ui->Plot_2->yAxis->setRange(0, 0.00000001);

    //ui->Xaxis->setInteraction(QCP::iSelectLegend, true);
    //ui->Yaxis->setInteraction(QCP::iSelectLegend, true);

    //connect(ui->Xaxis, SIGNAL(legendClick(QCPLegend *, QCPAbstractLegendItem *, QMouseEvent *)),
    //        this, SLOT(selectPlot(QCPLegend *, QCPAbstractLegendItem *, QMouseEvent *)));

    start_time = QDateTime::currentMSecsSinceEpoch();
}

void ModelAnalisisViewer::selectPlot(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    qDebug() << "i am called!";
}

void ModelAnalisisViewer::addState(int index, qreal x, qreal y, qint64 time)
{
    if (time < 0) {
        time = QDateTime::currentMSecsSinceEpoch() - start_time;
    }

    //time = qreal(time)/1000.0;

    ui->Plot_1->graph(index)->addData(time, double(x));
    ui->Plot_2->graph(index)->addData(time, double(y));

    ui->Plot_1->xAxis->setRange(0.0, time);
    ui->Plot_2->xAxis->setRange(0.0, time);

    if (ui->Plot_1->yAxis->range().upper < 1.2*x)
        ui->Plot_2->yAxis->setRange(0, 1.2*x);
    if (ui->Plot_1->yAxis->range().upper < 1.2*y)
        ui->Plot_2->yAxis->setRange(0, 1.2*y);

    ui->Plot_1->replot();
    ui->Plot_2->replot();
}


qreal ModelAnalisisViewer::getDataValueByType(GroupPos gpos, QTime time, PlotType plot_type, AxisType axis_type, int robot_id)
{
    qreal data_value = 0;

    RobotState st_st = ModelConfig::Instance()->getStartPosition().object_pos;

    switch (plot_type) {
        case V_LIN:
            switch(axis_type) {
                case X_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).vel.x;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;
        case V_ANG:
            switch(axis_type) {
                case Z_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).vel.w;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;
        case ACC_LIN:
            switch(axis_type) {
                case X_AXIS:
                    data_value = (gpos.robots_pos.at(robot_id).vel.x - prev_gpos.robots_pos.at(robot_id).vel.x);
                    data_value /= (time.msecsSinceStartOfDay()-prev_time)/1000.0;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;
        case ACC_ANG:
            switch(axis_type) {
                case Z_AXIS:
                    data_value = (gpos.robots_pos.at(robot_id).vel.w - prev_gpos.robots_pos.at(robot_id).vel.w);
                    data_value /= (time.msecsSinceStartOfDay()-prev_time)/1000.0;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;
        case POS:
            switch(axis_type) {
                case X_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).pos.x;
                    break;
                case Y_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).pos.y;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;
        case ERR_POS:
            switch(axis_type) {
                case X_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).pos_real.x - st_st.pos.x;
                    data_value -= gpos.robots_pos.at(robot_id).pos.x;
                    break;
                case Y_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).pos_real.y - st_st.pos.y;
                    data_value -= gpos.robots_pos.at(robot_id).pos.y;
                    break;
                default:
                    data_value = sqrt(
                        pow(gpos.robots_pos.at(robot_id).pos_real.x - st_st.pos.x - gpos.robots_pos.at(robot_id).pos.x, 2) +
                        pow(gpos.robots_pos.at(robot_id).pos_real.y - st_st.pos.y - gpos.robots_pos.at(robot_id).pos.y, 2)
                    );
                    break;
            }
            break;
        case ERR_VEL_X:
            switch(axis_type) {
                case X_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).vel_real.x;
                    data_value -= gpos.robots_pos.at(robot_id).vel.x;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;

        case ERR_VEL_Z:
            switch(axis_type) {
                case Z_AXIS:
                    data_value = gpos.robots_pos.at(robot_id).vel_real.w;
                    data_value -= gpos.robots_pos.at(robot_id).vel.w;
                    break;
                default:
                    data_value = 0;
                    break;
            }
            break;
    }

    return data_value;
}

// Slot calling by external signal
void ModelAnalisisViewer::async_addState(GroupPos gpos, QTime time)
{
    if (prev_time == 0) {
        prev_time = time.msecsSinceStartOfDay();
        prev_gpos = gpos;
    }
    int len = gpos.robots_pos.count();
    for (int i=0; i<len; i++) {
        this->addState(i,
           this->getDataValueByType(
               gpos,
               time,
               PlotType(ui->typeSelectBox->currentIndex()),
               AxisType(ui->axisSelectBox->currentIndex()),
               i
           ),
           this->getDataValueByType(
               gpos,
               time,
               PlotType(ui->typeSelectBox_2->currentIndex()),
               AxisType(ui->axisSelectBox_2->currentIndex()),
               i
           ),
           time.msecsSinceStartOfDay()
        );
    }
    prev_time = time.msecsSinceStartOfDay();
    prev_gpos = gpos;
}

ModelAnalisisViewer::~ModelAnalisisViewer()
{
    delete ui;
}


// Обработчики диалога сохранения графиков
void ModelAnalisisViewer::on_print_button_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save file", "", ".png");

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << file.errorString();
    } else {
        qDebug() << filename;
        ui->Plot_1->savePng(filename);
    }
}

void ModelAnalisisViewer::on_print_button_2_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save file", "", ".png");

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << file.errorString();
    } else {
        qDebug() << filename;
        ui->Plot_2->savePng(filename);
    }
}
