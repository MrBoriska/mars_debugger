#include "modelconfig.h"

ModelConfig::ModelConfig(QObject *parent) : QObject(parent)
{
    step = 0.1;
    interval = 100; // ms
    view_interval = 200; // ms
    target_realtime_factor = 0.6;

    sceneSize = QSize(1000,700);
    sceneBorderWidth = 15;
    modelAddress = "localhost:9999";

    robot_vmax = 0.5;
    robot_wmax = 3.0;
    robot_amax = 0.2;
    trajectory_v_P = 100.0;
    trajectory_v_I = 0.0;
    trajectory_v_D = 0.0;
    trajectory_w_P = 100.0;
    trajectory_w_I = 0.15;
    trajectory_w_D = 0.0;
    trajectory_v_thres = 0.7;
    trajectory_w_thres = 3.0;

    track_item = nullptr;
    object_item = nullptr;

    ItemMaterial mat;

    mat.color = QColor("#b5e3ea");
    mat.title = "Лёд";
    mat.accn_max = 0.01;
    mat.acct_max = 0.01;
    materials.append(mat);

    mat.color = QColor("#5a3d32");
    mat.title = "Земля";
    mat.accn_max = 0.05;
    mat.acct_max = 0.02;
    materials.append(mat);

    mat.color = QColor("#707f89");
    mat.title = "Асфальт";
    mat.accn_max = 0.1;
    mat.acct_max = 0.05;
    materials.append(mat);
    defaultMaterial = mat;

    mat.color = QColor("#f2e27f");
    mat.title = "Песок";
    mat.accn_max = 0.02;
    mat.acct_max = 0.02;
    materials.append(mat);
}

ModelConfig* ModelConfig::_instance = nullptr;
ModelConfig* ModelConfig::Instance() {
  if(_instance == nullptr){
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
    state.vel.x = 0.0;
    state.vel.w = 0.0;

    foreach(UnitItem *unit, units) {
        curr_point = unit->sceneBoundingRect().center();
        pos.x = curr_point.x();
        pos.y = curr_point.y();
        pos.alfa = unit->rotation()*M_PI/180;
        state.pos = pos;
        startPos.robots_pos.append(state);
    }

    // Начальная позиция обьекта управления
    if (object_item != nullptr) {
        QPointF curr_object_pos = object_item->sceneBoundingRect().center();

        pos.x = curr_object_pos.x();
        pos.y = curr_object_pos.y();
        pos.alfa = object_item->rotation();
    } else {
        pos = startPos.robots_pos.at(0).pos;
    }

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

void ModelConfig::setModelAddress(QString model_address)
{
    this->modelAddress = model_address;
}

QString ModelConfig::getModelAddress()
{
    return this->modelAddress;
}


void ModelConfig::jsonToScene(QJsonObject jo)
{
    // Загрузка параметров
    this->interval = jo.value("interval").toInt();
    this->step = jo.value("step").toDouble();

    this->robot_vmax = jo.value("robot_vmax").toDouble();
    this->robot_wmax = jo.value("robot_wmax").toDouble();
    this->robot_amax = jo.value("robot_amax").toDouble();
    this->trajectory_v_P = jo.value("trajectory_v_P").toDouble();
    this->trajectory_v_I = jo.value("trajectory_v_I").toDouble();
    this->trajectory_v_D = jo.value("trajectory_v_D").toDouble();
    this->trajectory_w_P = jo.value("trajectory_w_P").toDouble();
    this->trajectory_w_I = jo.value("trajectory_w_I").toDouble();
    this->trajectory_w_D = jo.value("trajectory_w_D").toDouble();
    this->trajectory_v_thres = jo.value("trajectory_v_thres").toDouble();
    this->trajectory_w_thres = jo.value("trajectory_w_thres").toDouble();

    QSize size;
    size.setWidth(jo.value("scene_size").toObject().value("width").toInt());
    size.setHeight(jo.value("scene_size").toObject().value("height").toInt());
    this->setSceneSize(size);
    this->sceneObject->setSceneRect(this->getSceneBorderWidth(),
                                    this->getSceneBorderWidth(),
                                    this->getSceneSize().width(),
                                    this->getSceneSize().height());

    // Загрузка данных материалов
    this->materials.clear();
    QJsonArray materials = jo.value("materials").toArray();
    int length = materials.size();
    for(int i=0;i<length;i++) {
        QJsonObject jo_ = materials[i].toObject();
        this->materials.append(jsonObject_to_material(jo_));
    }
    this->defaultMaterial = this->getItemMaterialByColor(
        QColor(jo.value("default_mat_color").toString())
    );

    // Обновление сцены
    this->sceneObject->safeClear();
    this->sceneObject->sceneRectShow(this->sceneObject->sceneRect());

    // Загрузка элементов сцены
    QJsonArray map = jo.value("map").toArray();
    foreach(QJsonValue value, map) {
        QJsonObject jo_ = value.toObject();
        // Загрузка полигона(область, препятствие)
        if (jo_.value("type").toString() == "ground_polygon"
                || jo_.value("type").toString() == "obstacle_polygon") {
            QPolygonF polygon;
            QJsonArray points = jo_.value("points").toArray();
            int count = points.count();
            for(int i=0;i<count;i++) {
                polygon << jsonObject_to_point(points[i].toObject());
            }
            PaintPolygonItem* polygon_item = new PaintPolygonItem();
            polygon_item->setPolygon(polygon);
            polygon_item->setPos(jsonObject_to_point(jo_.value("pos").toObject()));
            polygon_item->isObstacle(jo_.value("is_obstacle").toBool());
            if (!(polygon_item->isObstacle()))
                polygon_item->setBrush(QBrush(QColor(jo_.value("material").toString())));
            polygon_item->editable = jo_.value("editable").toBool();
            polygon_item->setZValue(jo_.value("z_value").toDouble());

            this->sceneObject->addItem(polygon_item);
        }

        if (jo_.value("type").toString() == "object") {
            ObjectItem *object_item = new ObjectItem();
            object_item->setRectByTwoPoints(
                jsonObject_to_point(jo_.value("p1").toObject()),
                jsonObject_to_point(jo_.value("p2").toObject())
            );

            this->sceneObject->addItem(object_item);
            this->setObject(object_item);
        }
        if (jo_.value("type").toString() == "robot") {
            QPointF unitPos = jsonObject_to_point(jo_.value("pos").toObject());

            UnitItem *unit_item = new UnitItem(unitPos);
            QPointF p_start = unit_item->sceneBoundingRect().center();
            qreal angle = jo_.value("direction").toDouble();

            this->sceneObject->setRotationItem(unit_item, p_start, angle);
            this->sceneObject->addItem(unit_item);
            this->units.clear();
            this->addUnit(unit_item);
        }
        if (jo_.value("type").toString() == "trajectory") {
            QPainterPath tpath = jsonArray_to_tpath(jo_.value("tpath").toArray());
            track_item = new TrackItem(tpath);
            this->sceneObject->addItem(track_item);
            track_item->setEndPoint();
            this->setTrack(track_item);
        }
    }
}


void ModelConfig::sceneToJson(QJsonObject *jo)
{
    // Сериализация параметров
    jo->insert("interval", this->interval);
    jo->insert("step", this->step);
    jo->insert("target_realtime_factor", this->target_realtime_factor);
    jo->insert("robot_vmax", this->robot_vmax);
    jo->insert("robot_wmax", this->robot_wmax);
    jo->insert("robot_amax", this->robot_amax);
    jo->insert("trajectory_v_P", this->trajectory_v_P);
    jo->insert("trajectory_v_I", this->trajectory_v_I);
    jo->insert("trajectory_v_D", this->trajectory_v_D);
    jo->insert("trajectory_w_P", this->trajectory_w_P);
    jo->insert("trajectory_w_I", this->trajectory_w_I);
    jo->insert("trajectory_w_D", this->trajectory_w_D);
    jo->insert("trajectory_v_thres", this->trajectory_v_thres);
    jo->insert("trajectory_w_thres", this->trajectory_w_thres);

    // Сериализация таблицы грунтов
    QJsonArray materials_;
    foreach(ItemMaterial mat, this->materials) {
        materials_.append(material_to_jsonObject(mat));
    }
    jo->insert("materials",materials_);
    jo->insert("default_mat_color", this->defaultMaterial.color.name());

    // Сериализация рабочей области сцены
    QJsonObject jsize;
    jsize.insert("width", this->getSceneSize().width());
    jsize.insert("height", this->getSceneSize().height());
    jo->insert("scene_size", jsize);

    // Сериализация элементов сцены
    QJsonArray map;
    QList<QGraphicsItem *> items = this->sceneObject->items(Qt::AscendingOrder);
    foreach(QGraphicsItem* item, items) {
        QJsonObject jo_;
        // Сериализация полигона(области, препятствия)
        if (item->type() == PaintPolygonItem::Type) {
            PaintPolygonItem *polygon_item = dynamic_cast<PaintPolygonItem *>(item);

            // Пропускаем базовые элементы сцены,
            // они генерируются автоматически на основе размеров рабочей области
            if (this->sceneObject->isBasePolygon(polygon_item))
                continue;

            QJsonArray points;
            QPolygonF polygon = polygon_item->polygon();
            int count = polygon.count();
            for(int i=0;i<count;i++) {
                points.append(point_to_jsonObject(polygon.at(i)));
            }
            jo_.insert("points", points);
            jo_.insert("pos", point_to_jsonObject(polygon_item->scenePos()));
            jo_.insert("editable", polygon_item->isEditable());
            jo_.insert("is_obstacle", polygon_item->isObstacle());
            jo_.insert("z_value", polygon_item->zValue());
            if (!(polygon_item->isObstacle())) {
                jo_.insert("type","ground_polygon");
                jo_.insert("material", polygon_item->getMaterialColor().name());
            } else {
                jo_.insert("type", "obstacle_polygon");
            }
            map.append(jo_);
            continue;
        }

        // Сериализация обьекта управления
        if (item->type() == ObjectItem::Type) {
            ObjectItem *object_item = dynamic_cast<ObjectItem *>(item);

            jo_.insert("type","object");
            jo_.insert("p1", point_to_jsonObject(object_item->getP1()));
            jo_.insert("p2", point_to_jsonObject(object_item->getP2()));

            map.append(jo_);
            continue;
        }

        // Сериализация робота
        if (item->type() == UnitItem::Type) {
            UnitItem *unit_item = dynamic_cast<UnitItem *>(item);

            jo_.insert("type","robot");
            jo_.insert("pos", point_to_jsonObject(unit_item->scenePos()));
            jo_.insert("direction", item->rotation());

            map.append(jo_);
            continue;
        }

        // Сериализация начальной траектории
        if (item->type() == TrackItem::Type) {
            TrackItem *track_item = dynamic_cast<TrackItem *>(item);

            jo_.insert("type","trajectory");
            jo_.insert("pos", point_to_jsonObject(track_item->scenePos()));
            jo_.insert("tpath", tpath_to_jsonArray(track_item->path()));

            map.append(jo_);
            continue;
        }
    }
    jo->insert("map", map);
}

/* ---------------------------------------------------------------
 * Функции для преобразования из внутренних типов в json и обратно
 * ---------------------------------------------------------------
 */
QJsonArray ModelConfig::tpath_to_jsonArray(QPainterPath tpath)
{
    int length = tpath.elementCount();
    QJsonArray ja;

    for(int i=0;i<length;i++) {
        QJsonObject jo;
        QPainterPath::Element el = tpath.elementAt(i);
        if (el.isLineTo())
            jo.insert("type", "LineTo");
        else if (el.isCurveTo())
            jo.insert("type", "CurveTo");
        else if (el.isMoveTo())
            // type for start position
            jo.insert("type", "MoveTo");
        else {
            qDebug() << "Error! Unsupported element of track";
            continue;
        }

        QPointF point = el;
        jo.insert("point", point_to_jsonObject(point));

        ja.append(jo);
    }

    return ja;
}
QJsonObject ModelConfig::gpos_to_jsonObject(GroupPos gpos)
{
    QJsonObject jo;
    jo.insert("object_pos", state_to_jsonObject(gpos.object_pos));
    QJsonArray ja;
    foreach(RobotState pos, gpos.robots_pos) {
        ja.append(state_to_jsonObject(pos));
    }
    jo.insert("robots_pos", ja);

    return jo;
}

QJsonObject ModelConfig::state_to_jsonObject(RobotState state)
{
    QJsonObject jo;
    jo.insert("pos", pos_to_jsonObject(state.pos));
    jo.insert("vel", vel_to_jsonObject(state.vel));
    jo.insert("pos_real", pos_to_jsonObject(state.pos_real));
    jo.insert("vel_real", vel_to_jsonObject(state.vel_real));
    return jo;
}

QJsonObject ModelConfig::material_to_jsonObject(ItemMaterial material)
{
    QJsonObject jo;
    jo.insert("title", material.title);
    jo.insert("color", material.color.name());

    jo.insert("accn_max", material.accn_max);
    jo.insert("acct_max", material.acct_max);

    return jo;
}

QJsonObject ModelConfig::point_to_jsonObject(QPointF point)
{
    QJsonObject jpoint;
    jpoint.insert("x", point.x());
    jpoint.insert("y", point.y());
    return jpoint;
}

QJsonObject ModelConfig::pos_to_jsonObject(ItemPos pos)
{
    QJsonObject jo;
    jo.insert("alfa", pos.alfa);
    jo.insert("x", pos.x);
    jo.insert("y", pos.y);
    return jo;
}

QJsonObject ModelConfig::vel_to_jsonObject(ItemVel vel)
{
    QJsonObject jo;
    jo.insert("x",vel.x);
    jo.insert("w",vel.w);
    return jo;
}
QPainterPath ModelConfig::jsonArray_to_tpath(QJsonArray ja)
{
    QPainterPath tpath;
    int length = ja.size();
    for(int i=0;i<length;i++) {
        QJsonObject jo = ja[i].toObject();

        if (jo.value("type").toString() == "LineTo")
            tpath.lineTo(
                jo.value("point").toObject().value("x").toDouble(),
                jo.value("point").toObject().value("y").toDouble()
            );
        else if (jo.value("type").toString() == "MoveTo")
            tpath.moveTo(
                jo.value("point").toObject().value("x").toDouble(),
                jo.value("point").toObject().value("y").toDouble()
            );
        else {
            qDebug() << "Error! Unsupported element of track(QJsonArray)";
        }
    }
    return tpath;
}
GroupPos ModelConfig::jsonObject_to_gpos(QJsonObject jo)
{
    GroupPos gpos;
    gpos.object_pos = jsonObject_to_state(jo.value("object_pos").toObject());
    gpos.robots_pos = QList<RobotState>();
    foreach(QJsonValue value, jo.value("robots_pos").toArray()) {
        gpos.robots_pos.append(jsonObject_to_state(value.toObject()));
    }
    return gpos;
}

RobotState ModelConfig::jsonObject_to_state(QJsonObject jo)
{
    RobotState state;
    state.pos = jsonObject_to_pos(jo.value("pos").toObject());
    state.vel = jsonObject_to_vel(jo.value("vel").toObject());
    state.pos_real = jsonObject_to_pos(jo.value("pos_real").toObject());
    state.vel_real = jsonObject_to_vel(jo.value("vel_real").toObject());
    return state;
}

ItemPos ModelConfig::jsonObject_to_pos(QJsonObject jo)
{
    ItemPos pos;
    pos.alfa = jo.value("alfa").toDouble();
    pos.x = jo.value("x").toDouble();
    pos.y = jo.value("y").toDouble();
    pos.k = 0;
    return pos;
}

ItemVel ModelConfig::jsonObject_to_vel(QJsonObject jo)
{
    ItemVel vel;
    vel.x = jo.value("x").toDouble();
    vel.w = jo.value("w").toDouble();
    return vel;
}

ItemMaterial ModelConfig::jsonObject_to_material(QJsonObject jo)
{
    ItemMaterial material;
    material.title = jo.value("title").toString();
    material.color = QColor(jo.value("color").toString());

    material.accn_max = jo.value("accn_max").toDouble();
    material.acct_max = jo.value("acct_max").toDouble();

    return material;
}

QPointF ModelConfig::jsonObject_to_point(QJsonObject jpoint)
{
    return QPointF(
        jpoint.value("x").toDouble(),
        jpoint.value("y").toDouble()
    );
}

