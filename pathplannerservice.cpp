#include "pathplannerservice.h"

#include <QDebug>
#include <QtGlobal>

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>

PathPlannerService::PathPlannerService(QObject *parent) : QObject(parent)
{
  self_socket = nullptr;
}

PathPlannerService::~PathPlannerService()
{
  // paranoya mode
  if (self_socket != nullptr) {
      self_socket->close();
  }
}

void PathPlannerService::init()
{
  self_socket = new QWebSocket();

  is_server = false;

  QString address = ModelConfig::Instance()->getModelAddress();
  address.prepend("ws://");

  qDebug() << "connecting...";
  qDebug() << address;

  // this is not blocking call
  self_socket->open(QUrl(address));
  connect(self_socket, SIGNAL(connected()),this, SLOT(pp_connected()));
  connect(self_socket, SIGNAL(disconnected()),this,SLOT(pp_disconnected_()));
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

void PathPlannerService::pp_connected()
{
  qDebug() << "connected";
  connect(self_socket, SIGNAL(textMessageReceived(QString)), this, SLOT(pp_readyRead(QString)));

}

void PathPlannerService::pp_disconnected_()
{
  qDebug() << "disconnected";
  if (self_socket) self_socket->deleteLater();
  self_socket = nullptr;
  emit pp_disconnected();
}


void PathPlannerService::pp_readyRead(QString strReply)
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
  else if (msg_type == "progress_event") handler_progress_event();
  else if (msg_type == "result_event") handler_result_event();
  else if (msg_type == "paused_event") handler_paused_event();
  else if (msg_type == "stopped_event") handler_stopped_event();
  else if (msg_type == "ready_event") handler_ready_event();
  else if (msg_type == "unready_event") handler_unready_event();

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
void PathPlannerService::start_request()
{
  QJsonObject msg;
  msg.insert("type","start_request");
  self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}
void PathPlannerService::handler_progress_event()
{
  emit pp_progress();
  //emit cs_started_error("Unknown error");
}

void PathPlannerService::handler_result_event()
{
  emit pp_result();
  //emit cs_started_error("Unknown error");
}

/* ---------------------------------------------------------------
* ПАУЗА
* ---------------------------------------------------------------
*/
void PathPlannerService::send_pause_request()
{
  QJsonObject msg;
  msg.insert("type","pause_request");
  self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

void PathPlannerService::handler_paused_event()
{
  emit pp_paused();
}

/* ---------------------------------------------------------------
* ОСТАНОВКА
* ---------------------------------------------------------------
*/
void PathPlannerService::send_stop_request()
{
  QJsonObject msg;
  msg.insert("type","stop_request");
  self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

void PathPlannerService::handler_stopped_event()
{
  emit pp_stopped();
}

/* ---------------------------------------------------------------
* Отправка входных данных системе управления
* ---------------------------------------------------------------
*/
void PathPlannerService::set_config_data(ModelConfig *config)
{
  QJsonObject msg;
  msg.insert("type", "set_config_data");

  config->sceneToJson(&msg);

  qDebug() << msg;

  self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}

// Вызывается в момент, когда СУ имеет достаточное количество входных данных, чтобы запуститься
void PathPlannerService::handler_ready_event()
{
  emit pp_ready();
}
void PathPlannerService::handler_unready_event()
{
  emit pp_unready();
}

/* ---------------------------------------------------------------
* Функции для запроса текущего положения роботов и груза
* ---------------------------------------------------------------
*/
void PathPlannerService::get_progress_request()
{
  QJsonObject msg;
  msg.insert("type","get_progress");
  self_socket->sendTextMessage(QJsonDocument(msg).toJson());
}
