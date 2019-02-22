#include "modelhandler.h"

ModelHandler::ModelHandler(QObject *parent) : QObject(parent)
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(syncModelAndView()));
    timer->setInterval(1000);
    timer->start();
}

ModelHandler::~ModelHandler()
{
    delete timer;
}

void ModelWorker::syncModelAndView()
{
    qDebug() << "ModelWorker::syncModelAndView()";

}

