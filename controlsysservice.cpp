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
    self_socket = nullptr;
}

ControlSysService::~ControlSysService()
{
    // paranoya mode
    if (self_socket != nullptr) {
        self_socket->close();
    }
}

void ControlSysService::init()
{
    self_socket = new QWebSocket();

    is_server = false;

    QString address = ModelConfig::Instance()->getModelAddress();
    address.prepend("ws://");

    qDebug() << "connecting...";
    qDebug() << address;

    // this is not blocking call
    self_socket->open(QUrl(address));
    connect(self_socket, SIGNAL(connected()),this, SLOT(cs_connected()));
    connect(self_socket, SIGNAL(disconnected()),this,SLOT(cs_disconnected_()));
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

void ControlSysService::cs_disconnected_()
{
    qDebug() << "disconnected";
    if (self_socket) self_socket->deleteLater();
    self_socket = nullptr;
    emit cs_disconnected();
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
        error_msg.insert("type", "error");
        error_msg.insert("info", parse_error.errorString());

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
void ControlSysService::set_config_data(ModelConfig *config)
{
    QJsonObject msg;
    msg.insert("type", "set_config_data");

    config->sceneToJson(&msg);

    qDebug() << msg;

    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}


void ControlSysService::set_start_pos(GroupPos start_pos)
{
    QJsonObject msg = ModelConfig::gpos_to_jsonObject(start_pos);
    msg.insert("type", "set_start_pos");

    qDebug() << msg;

    self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

void ControlSysService::set_track_path(QPainterPath tpath)
{
    QJsonObject msg;
    msg.insert("type", "set_track_path");
    msg.insert("path", ModelConfig::tpath_to_jsonArray(tpath));


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
        ModelConfig::jsonObject_to_gpos(jsonObject),
        QTime(0,0,0,0).addMSecs(jsonObject.value("time").toInt())
    );
}



