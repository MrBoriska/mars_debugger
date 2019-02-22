#include "modelanalisisviewer.h"
#include "ui_modelanalisisviewer.h"

#include <QDateTime>

#ifndef QT_NO_PRINTER
#include <QPrinter>
#include <QPrintDialog>
#endif

ModelAnalisisViewer::ModelAnalisisViewer(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModelAnalisisViewer)
{
    ui->setupUi(this);

    // Название осей
    ui->Xaxis->xAxis->setLabel("Время, t[с]");
    ui->Xaxis->yAxis->setLabel("X Axis");
    ui->Yaxis->xAxis->setLabel("Время, t[с]");
    ui->Yaxis->yAxis->setLabel("Y Axis");
    ui->Angle->xAxis->setLabel("Время, t[с]");
    ui->Angle->yAxis->setLabel("Angle");

    // Настройки навигации по графику
    ui->Xaxis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->Yaxis->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    ui->Angle->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

    this->setWindowFlags(Qt::Window);
}

void ModelAnalisisViewer::closeEvent(QCloseEvent *event)
{
    emit closed();
    QDialog::closeEvent(event);
}

void ModelAnalisisViewer::startAnalisis(int units_count)
{
    ui->Xaxis->clearGraphs();
    ui->Yaxis->clearGraphs();
    ui->Angle->clearGraphs();

    // цвета с номерами 0 - 19 соответсвуют глобальным цветам Qt::GlobalColor
    // нам интересны только цвета в диапазоне 7-18
    int color = 7;
    QPen unit_pen;
    QString name_graph;

    for (int i=0;i<units_count;i++) {
        unit_pen = QPen((Qt::GlobalColor)color);
        name_graph = "Unit " + QString::number(i);

        ui->Xaxis->addGraph()->setPen(unit_pen);
        ui->Yaxis->addGraph()->setPen(unit_pen);
        ui->Angle->addGraph()->setPen(unit_pen);
        ui->Xaxis->graph(i)->setName(name_graph);
        ui->Yaxis->graph(i)->setName(name_graph);
        ui->Angle->graph(i)->setName(name_graph);


        color++;
        if (color > 18) color = 7;
    }

    ui->Xaxis->legend->setVisible(true);
    ui->Yaxis->legend->setVisible(true);
    ui->Angle->legend->setVisible(true);
    //ui->Xaxis->setInteraction(QCP::iSelectLegend, true);
    //ui->Yaxis->setInteraction(QCP::iSelectLegend, true);
    //ui->Angle->setInteraction(QCP::iSelectLegend, true);


    //connect(ui->Xaxis, SIGNAL(legendClick(QCPLegend *, QCPAbstractLegendItem *, QMouseEvent *)),
    //        this, SLOT(selectPlot(QCPLegend *, QCPAbstractLegendItem *, QMouseEvent *)));

    start_time = QDateTime::currentMSecsSinceEpoch();
}

void ModelAnalisisViewer::selectPlot(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event)
{
    qDebug() << "i am called!";
}

void ModelAnalisisViewer::addState(int index, qreal x, qreal y, qreal angle, qint64 time)
{
    if (time < 0) {
        time = QDateTime::currentMSecsSinceEpoch() - start_time;
    }

    //time /= 1000;

    ui->Xaxis->graph(index)->addData(time, (double)x);
    ui->Yaxis->graph(index)->addData(time, (double)y);
    ui->Angle->graph(index)->addData(time, (double)angle);

    ui->Xaxis->xAxis->setRange(0, time);
    ui->Yaxis->xAxis->setRange(0, time);
    ui->Angle->xAxis->setRange(0, time);

    ui->Xaxis->yAxis->setRange(0, 700);
    ui->Yaxis->yAxis->setRange(0, 500);
    ui->Angle->yAxis->setRange(-360, 360);

    ui->Xaxis->replot();
    ui->Yaxis->replot();
    ui->Angle->replot();
}

void ModelAnalisisViewer::async_addState(GroupPos gpos, QTime time)
{
    int len = gpos.robots_pos.count();
    for(int i=0; i<len; i++) {
        ItemPos pos = gpos.robots_pos.at(i).pos;
        this->addState(i, pos.x, pos.y, pos.alfa, time.msec());
    }
}

ModelAnalisisViewer::~ModelAnalisisViewer()
{
    delete ui;
}

void ModelAnalisisViewer::on_print_button_clicked()
{
#if !defined(QT_NO_PRINTER) && !defined(QT_NO_PRINTDIALOG)
    QPrinter printer;
    // use paintRequest(QPainter*) signal
    //QPrintPreviewDialog dialog(&printer, this);
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        this->render(&painter);// todo: необходимо что-то умнее, чем просто окошко копировать
    }
#endif
}
