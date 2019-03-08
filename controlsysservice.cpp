#include "controlsysservice.h"

#include <QDebug>
#include <QtGlobal>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

#include "modelconfig.h"


//This is run after a client connects, but before authentication takes place.
//You can do initializtion here, but do nothing that requires authentication.
ControlSysService::ControlSysService(QObject *parent) :
    QObject(parent)
{
    self_socket = 0;
}

ControlSysService::~ControlSysService()
{
    self_socket->close();
}

void ControlSysService::init()
{
    self_socket = new QWebSocket();

    is_server = false;

    qDebug() << "connecting...";

    // this is not blocking call
    //self_socket->connectToHost("127.0.0.1", 9999);
    self_socket->open(QUrl(QStringLiteral("ws://127.0.0.1:9999")));
    connect(self_socket, SIGNAL(connected()),this, SLOT(cs_connected()));
    connect(self_socket, SIGNAL(disconnected()),this,SLOT(cs_disconnected()));
    /*connect(self_socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this,         SLOT(connectionError(QAbstractSocket::SocketError))
    );*/

    // we need to wait...
    /*if(!self_socket->waitForConnected(5000))
    {
        qDebug() << "Error: " << self_socket->errorString();
        self_socket->deleteLater();
        self_socket = 0;
    }*/
}

void ControlSysService::cs_connected()
{
    qDebug() << "connected";
    connect(self_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(cs_readyRead(QString)));

}

void ControlSysService::cs_disconnected()
{
    qDebug() << "disconnected";
    if (self_socket) self_socket->deleteLater();
    self_socket = 0;
}


void ControlSysService::cs_readyRead(QString strReply)
{
    qDebug("readyRead: ");

    QString msg_type;

    // Read new message
    //QString strReply = (QString)(self_socket->readAll());

    // Parse JSON
    QJsonParseError parse_error;
    QJsonDocument jsonResponse = QJsonDocument::fromJson(strReply.toUtf8(), &parse_error);

    // Validate and send validation error
    if (jsonResponse.isNull()) {
        QJsonObject error_msg;
        error_msg.insert("type","error");
        error_msg.insert("info",parse_error.errorString());

        self_socket->sendTextMessage(QJsonDocument(error_msg).toJson());
        return;
    }

    // Using JSON
    QJsonObject jsonObject = jsonResponse.object();

    // Get type of message
    msg_type = jsonObject.value("type").toString();

    qDebug() << jsonObject;

    // Call handlers
    if (msg_type == "ping")
    {
        qDebug() << "Pong!";
        QJsonObject pong_msg;
        pong_msg.insert("type","pong");
        self_socket->sendTextMessage(QJsonDocument(pong_msg).toJson());
    }
    else if (msg_type == "pong") {}
    else if (msg_type == "error") {
        // todo
    }
    else if (msg_type == "started_event") handler_started_event();
    else if (msg_type == "paused_event") handler_paused_event();
    else if (msg_type == "stopped_event") handler_stopped_event();
    else if (msg_type == "ready_event") handler_ready_event();
    else if (msg_type == "unready_event") handler_unready_event();
    else if (msg_type == "positions") handler_positions(jsonObject);

    // Unknown message type
    else {
        QJsonObject error_msg;
        error_msg.insert("type","error");
        error_msg.insert("info","Unknown type (" + msg_type + ")");

        self_socket->sendTextMessage(QJsonDocument(error_msg).toJson());
    }
}

/* ---------------------------------------------------------------
 * ЗАПУСК
 * ---------------------------------------------------------------
 */
void ControlSysService::start_request()
{
    QJsonObject msg;
    msg.insert("type","start_request");
    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}
void ControlSysService::handler_started_event()
{
    emit cs_started();
    //emit cs_started_error("Unknown error");
}

/* ---------------------------------------------------------------
 * ПАУЗА
 * ---------------------------------------------------------------
 */
void ControlSysService::send_pause_request()
{
    QJsonObject msg;
    msg.insert("type","pause_request");
    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

void ControlSysService::handler_paused_event()
{
    emit cs_paused();
}

/* ---------------------------------------------------------------
 * ОСТАНОВКА
 * ---------------------------------------------------------------
 */
void ControlSysService::send_stop_request()
{
    QJsonObject msg;
    msg.insert("type","stop_request");
    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

void ControlSysService::handler_stopped_event()
{
    emit cs_stopped();
}

/* ---------------------------------------------------------------
 * Отправка входных данных системе управления
 * ---------------------------------------------------------------
 */
void ControlSysService::set_config_data(ModelConfig *config) {
    QJsonObject msg;
    msg.insert("type", "set_config_data");

    msg.insert("interval", config->interval);
    msg.insert("step", config->step);
    msg.insert("target_realtime_factor", config->target_realtime_factor);
    msg.insert("vel_max", config->vel_max);
    msg.insert("accn_max", config->accn_max);
    msg.insert("acct_max", config->acct_max);

    // Insertion materials list
    QJsonArray materials;
    foreach(ItemMaterial mat, config->materials) {
        materials.append(material_to_jsonObject(mat));
    }
    msg.insert("materials",materials);


    // Inserion map of scene

    QJsonObject jsize;
    jsize.insert("width", config->getSceneSize().width());
    jsize.insert("height", config->getSceneSize().height());
    msg.insert("scene_size", jsize);

    QJsonArray map;
    QList<QGraphicsItem *> items = config->sceneObject->items();
    foreach(QGraphicsItem* item, items) {
        QJsonObject jo;
        if (item->type() == PaintPolygonItem::Type) {
            jo.insert("type","ground_polygon");

            PaintPolygonItem *polygon_item = (PaintPolygonItem *)item;
            if (!(polygon_item->isObstacle())) {
                QJsonArray points;
                QPolygonF polygon = polygon_item->polygon();
                int count = polygon.count();
                for(int i=0;i<count;i++) {
                    points.append(point_to_jsonObject(polygon.at(i)));
                }
                jo.insert("points", points);
                jo.insert("pos", point_to_jsonObject(polygon_item->scenePos()));

                jo.insert("material", polygon_item->getMaterialColor().name());
                map.append(jo);
            }
        }
    }
    msg.insert("map", map);


    qDebug() << msg;

    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}


void ControlSysService::set_start_pos(GroupPos start_pos)
{
    QJsonObject msg = gpos_to_jsonObject(start_pos);
    msg.insert("type", "set_start_pos");

    qDebug() << msg;

    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

void ControlSysService::set_track_path(QPainterPath tpath)
{
    QJsonObject msg;
    msg.insert("type", "set_track_path");
    msg.insert("path", tpath_to_jsonArray(tpath));


    qDebug() << msg;

    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

// Вызывается в момент, когда СУ имеет достаточное количество входных данных, чтобы запуститься
void ControlSysService::handler_ready_event()
{
    emit cs_ready();
}
void ControlSysService::handler_unready_event()
{
    emit cs_unready();
}

/* ---------------------------------------------------------------
 * Функции для запроса текущего положения роботов и груза
 * ---------------------------------------------------------------
 */
void ControlSysService::get_pos_request()
{
    QJsonObject msg;
    msg.insert("type","get_pos");
    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}
void ControlSysService::handler_positions(QJsonObject jsonObject)
{
    emit cs_current_pos(
        jsonObject_to_gpos(jsonObject),
        QTime(0,0,0,0).addMSecs(jsonObject.value("time").toInt())
    );
}


/* ---------------------------------------------------------------
 * Функции для преобразования из внутренних типов в json и обратно
 * ---------------------------------------------------------------
 */
QJsonArray ControlSysService::tpath_to_jsonArray(QPainterPath tpath)
{
    int length = tpath.elementCount();
    QJsonArray ja;
    for(int i=1;i<length;i++) {
        QJsonObject jo;
        QPainterPath::Element el = tpath.elementAt(i);

        if (el.isLineTo())
            jo.insert("type", "LineTo");
        else if (el.isCurveTo())
            jo.insert("type", "CurveTo");
        else {
            // Error! Unsupported element of track
            return QJsonArray();
        }

        QPointF point = el;
        jo.insert("point", point_to_jsonObject(point));

        ja.append(jo);
    }

    return ja;
}
QJsonObject ControlSysService::gpos_to_jsonObject(GroupPos gpos)
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

QJsonObject ControlSysService::state_to_jsonObject(RobotState state)
{
    QJsonObject jo;
    jo.insert("pos", pos_to_jsonObject(state.pos));
    jo.insert("vel", vel_to_jsonObject(state.vel));
    return jo;
}

QJsonObject ControlSysService::material_to_jsonObject(ItemMaterial material)
{
    QJsonObject jo;
    jo.insert("title", material.title);
    jo.insert("color", material.color.name());

    jo.insert("accn_max", material.accn_max);
    jo.insert("acct_max", material.acct_max);

    return jo;
}

QJsonObject ControlSysService::point_to_jsonObject(QPointF point)
{
    QJsonObject jpoint;
    jpoint.insert("x", point.x());
    jpoint.insert("y", point.y());
    return jpoint;
}

QJsonObject ControlSysService::pos_to_jsonObject(ItemPos pos)
{
    QJsonObject jo;
    jo.insert("alfa", pos.alfa);
    jo.insert("x", pos.x);
    jo.insert("y", pos.y);
    return jo;
}

QJsonObject ControlSysService::vel_to_jsonObject(ItemVel vel)
{
    QJsonObject jo;
    jo.insert("x",vel.x);
    jo.insert("w",vel.w);
    return jo;
}

GroupPos ControlSysService::jsonObject_to_gpos(QJsonObject jo)
{
    GroupPos gpos;
    gpos.object_pos = jsonObject_to_state(jo.value("object_pos").toObject());
    gpos.robots_pos = QList<RobotState>();
    foreach(QJsonValue value, jo.value("robots_pos").toArray()) {
        gpos.robots_pos.append(jsonObject_to_state(value.toObject()));
    }
    return gpos;
}

RobotState ControlSysService::jsonObject_to_state(QJsonObject jo)
{
    RobotState state;
    state.pos = jsonObject_to_pos(jo.value("pos").toObject());
    state.vel = jsonObject_to_vel(jo.value("vel").toObject());
    return state;
}

ItemPos ControlSysService::jsonObject_to_pos(QJsonObject jo)
{
    ItemPos pos;
    pos.alfa = jo.value("alfa").toDouble();
    pos.x = jo.value("x").toDouble();
    pos.y = jo.value("y").toDouble();
    pos.k = 0;
    return pos;
}

ItemVel ControlSysService::jsonObject_to_vel(QJsonObject jo)
{
    ItemVel vel;
    vel.x = jo.value("x").toDouble();
    vel.w = jo.value("w").toDouble();
    return vel;
}



