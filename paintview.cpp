#include "paintview.h"

PaintView::PaintView(QWidget *parent) : QGraphicsView(parent)
{
    //ui->graphicsView->verticalScrollBar()->blockSignals(true);
    //ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //ui->graphicsView->horizontalScrollBar()->blockSignals(true);
    //ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
}

PaintView::~PaintView()
{

}

void PaintView::wheelEvent ( QWheelEvent * event )
{
    if(event->modifiers() & Qt::ControlModifier) {
        double scaleFactor = 1.05;
        if(event->delta() > 0) {
            scale(scaleFactor,scaleFactor);
        } else {
            scale(1/scaleFactor,1/scaleFactor);
        }
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void PaintView::mousePressEvent(QMouseEvent * e)
{
   if (e->button() == Qt::LeftButton &&
       e->modifiers() & Qt::ControlModifier)
   {
        setDragMode(QGraphicsView::ScrollHandDrag);
        QMouseEvent fake(e->type(), e->pos(), Qt::LeftButton, Qt::LeftButton, e->modifiers());
        QGraphicsView::mousePressEvent(&fake);
   }
   else QGraphicsView::mousePressEvent(e);
}

void PaintView::mouseReleaseEvent(QMouseEvent * e)
{
   if (e->button() == Qt::LeftButton &&
       e->modifiers() & Qt::ControlModifier)
   {
        setDragMode(QGraphicsView::NoDrag);
        QMouseEvent fake(e->type(), e->pos(), Qt::LeftButton, Qt::LeftButton, e->modifiers());
        QGraphicsView::mouseReleaseEvent(&fake);
   }
   else QGraphicsView::mouseReleaseEvent(e);
}

