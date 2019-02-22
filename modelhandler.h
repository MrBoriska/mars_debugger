#ifndef MODELHANDLER_H
#define MODELHANDLER_H

#include <QObject>
#include <QTimer>

class ModelHandler : public QObject
{
    Q_OBJECT
public:
    explicit ModelHandler(QObject *parent = 0);

private:
    QTimer *timer;
};

#endif // MODELHANDLER_H
