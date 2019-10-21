#ifndef PAINTITEM_H
#define PAINTITEM_H


#include <QObject>
#include <QtGui>
#include <QtCore>
#include <QGraphicsItem>
#include <QVariant>

/**
 * @brief Base class of all paintable items on scene
 */
// TODO: inherits from QAbstractGraphicsShapeItem in future
class PaintItem : public QObject
{
    Q_OBJECT
public:
    enum { Type = QVariant::UserType + 10 };

    explicit PaintItem(QObject *parent = nullptr);

    int type() const { return Type; }

    virtual bool isEditable() {return true;}

signals:
    void selectChanged(QGraphicsItem *item = nullptr);
    void positionChanged(QGraphicsItem *item = nullptr);

};

#endif // PAINTITEM_H
