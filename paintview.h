#ifndef PAINTVIEW_H
#define PAINTVIEW_H

#include <QObject>
#include <QtGui>
#include <QtCore>
#include <QGraphicsView>

class PaintView : public QGraphicsView
{
    Q_OBJECT
public:
    explicit PaintView(QWidget *parent = nullptr);
    ~PaintView();

signals:

public slots:

protected:
    // Перехват скроллинга для организации масштабирования по Ctrl+Scroll
    void wheelEvent(QWheelEvent *event);
    // Перехват событий для организации перемещения сцены по Ctrl+ЛКМ
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
};

#endif // PAINTVIEW_H
