#include "modelconfig.h"

ModelConfig::ModelConfig(QObject *parent) : QObject(parent)
{
    step = 0.5;
    interval = 10;//150
    target_realtime_factor = 1.0;
    vel_max = 0.5; // m/s
    accn_max = 0.01; // m/s^2
    acct_max = 0.1; // m/s^2


    step_max = 3;
    step_min = 0.1;
    interval_max = 10;
    interval_min = 4;

    sceneSize = QSize(1000,700);
    sceneBorderWidth = 15;

    track_item = 0;
    object_item = 0;

    ItemMaterial mat;

    mat.color = QColor("#b5e3ea");
    mat.title = "Лёд";
    mat.accn_max = 0.01;
    mat.acct_max = 0.01;
    materials.append(mat);

    mat.color = QColor("#5a3d32");
    mat.title = "Земля";
    mat.accn_max = 0.05;
    mat.acct_max = 0.2;
    materials.append(mat);

    mat.color = QColor("#707f89");
    mat.title = "Асфальт";
    mat.accn_max = 0.2;
    mat.acct_max = 0.2;
    materials.append(mat);
    defaultMaterial = mat;

    mat.color = QColor("#f2e27f");
    mat.title = "Песок";
    mat.accn_max = 0.02;
    mat.acct_max = 0.02;
    materials.append(mat);
}

ModelConfig* ModelConfig::_instance = 0;
ModelConfig* ModelConfig::Instance() {
  if(_instance == 0){
     _instance = new ModelConfig;
  }
  return _instance;
}

void ModelConfig::addUnit(UnitItem *unit)
{
    units.append(unit);
}

void ModelConfig::delUnit(UnitItem *unit)
{
    units.removeOne(unit);
}

QList<UnitItem*> ModelConfig::getUnits()
{
    return units;
}

void ModelConfig::setTrack(TrackItem *item)
{
    track_item = item;
}

TrackItem *ModelConfig::getTrack()
{
    return track_item;
}

 QPainterPath ModelConfig::getTrackPath()
 {
     QPainterPath path = track_item->path();
     QPainterPath path_;
     for (int i=0; i < path.elementCount();i++)
         path_.lineTo(path.elementAt(i)-QPointF(startPos.object_pos.pos.x,startPos.object_pos.pos.y));

     return path_;
 }

void ModelConfig::setObject(ObjectItem *object)
{
    object_item = object;
}

ObjectItem * ModelConfig::getObject()
{
    return object_item;
}

void ModelConfig::setStartPosition()
{
    // Начальная позиция роботов
    startPos.robots_pos.clear();
    QPointF curr_point;
    ItemPos pos;
    RobotState state;
    state.pos = pos;
    state.vel.x = 0;
    state.vel.w = 0;

    foreach(UnitItem *unit, units) {
        curr_point = unit->sceneBoundingRect().center();
        pos.x = curr_point.x();
        pos.y = curr_point.y();
        pos.alfa = unit->rotation()*M_PI/180;
        state.pos = pos;
        startPos.robots_pos.append(state);
    }

    // Начальная позиция обьекта управления
    QPointF curr_object_pos = object_item->sceneBoundingRect().center();

    pos.x = curr_object_pos.x();
    pos.y = curr_object_pos.y();
    pos.alfa = object_item->rotation();

    state.pos = pos;
    startPos.object_pos = state;

}

GroupPos ModelConfig::getStartPosition()
{
    return startPos;
}

void ModelConfig::setSceneSize(QSize size)
{
    sceneSize = size;
}

double ModelConfig::getSceneBorderWidth()
{
    return sceneBorderWidth;
}

QSize ModelConfig::getSceneSize()
{
    return sceneSize;
}

ItemMaterial ModelConfig::getItemMaterialByColor(QColor color)
{
    foreach (ItemMaterial mat, materials) {
        if (mat.color == color) {
            return mat;
        }
    }
    return defaultMaterial;
}

void ModelConfig::reset()
{

}
