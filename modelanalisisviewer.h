#ifndef MODELANALISISVIEWER_H
#define MODELANALISISVIEWER_H

#include <QDialog>
#include <qcustomplot/qcustomplot.h>

#include "modelconfig.h"


namespace Ui {
class ModelAnalisisViewer;
}

class ModelAnalisisViewer : public QDialog
{
    Q_OBJECT

public:

    enum PlotType {
        V_LIN, V_ANG, ACC_LIN, ACC_ANG, POS, ERR_POS, ERR_VEL_X, ERR_VEL_Z
    };

    enum AxisType {
         NORM_AXIS, X_AXIS, Y_AXIS, Z_AXIS
    };

    const QStringList types_list = {
        "Линейная скорость",
        "Угловая скорость",
        "Линейное ускорение",
        "Положение",
        "Рассогласование по положению",
        "Рассогласование по линейной скорости",
        "Рассогласование по угловой скорости"
    };

    const QStringList axis_list = {
        " ", "Ось X", "Ось Y", "Ось Z"
    };

    explicit ModelAnalisisViewer(QWidget *parent = nullptr);
    ~ModelAnalisisViewer();

    void startAnalisis(int units_count);
    void addState(int index, qreal x, qreal y, qint64 time = -1);

signals:
    void closed();

public slots:
    void async_addState(GroupPos gpos, QTime time);

protected:
    void closeEvent(QCloseEvent *event);

protected slots:
    void selectPlot(QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event);

private slots:
    void on_print_button_clicked();

    void on_print_button_2_clicked();

private:
    Ui::ModelAnalisisViewer *ui;

    qint64 start_time;

    qint64 prev_time;
    GroupPos prev_gpos;

    void setAxisNames(PlotType plot_type, QCustomPlot *plot);
    qreal getDataValueByType(GroupPos gpos, QTime time, PlotType plot_type, AxisType axis_type, int robot_id);
};

#endif // MODELANALISISVIEWER_H
