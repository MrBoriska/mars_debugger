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
    explicit ModelAnalisisViewer(QWidget *parent = 0);
    ~ModelAnalisisViewer();

    void startAnalisis(int units_count);
    void addState(int index, qreal x, qreal y, qreal angle, qint64 time = -1);

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

private:
    Ui::ModelAnalisisViewer *ui;

    qint64 start_time;
};

#endif // MODELANALISISVIEWER_H
