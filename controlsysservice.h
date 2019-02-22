#ifndef CONTROLSYSSERVICE_H
#define CONTROLSYSSERVICE_H

#include <QObject>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebSockets/qwebsocket.h>
#include <QDebug>

#include "modelconfig.h"

class ControlSysService : public QObject
{
    Q_OBJECT
public:
    explicit ControlSysService(QObject *parent = 0);
    ~ControlSysService();




signals:
    void cs_started();
    void cs_started_error(const QString);
    void cs_ready();
    void cs_unready();
    void cs_paused();
    void cs_stopped();
    void cs_current_pos(GroupPos, QTime);

public slots:
    void init();
    void start_request();
    void send_pause_request();
    void send_stop_request();
    void set_start_pos(GroupPos start_pos);
    void set_track_path(QPainterPath tpath);
    void get_pos_request();

private slots:
    void cs_connected();
    void cs_disconnected();
    void cs_readyRead(QString strReply);

private:
    QWebSocket *self_socket;
    QWebSocket *cs_socket;

    bool is_server;

    void handler_started_event();
    void handler_paused_event();
    void handler_stopped_event();
    void handler_ready_event();
    void handler_unready_event();
    void handler_positions(QJsonObject jsonObject);

    QJsonObject gpos_to_jsonObject(GroupPos gpos);
    QJsonObject state_to_jsonObject(RobotState state);
    QJsonObject pos_to_jsonObject(ItemPos pos);
    QJsonObject vel_to_jsonObject(ItemVel vel);
    GroupPos jsonObject_to_gpos(QJsonObject jo);
    RobotState jsonObject_to_state(QJsonObject jo);
    ItemPos jsonObject_to_pos(QJsonObject jo);
    ItemVel jsonObject_to_vel(QJsonObject jo);
    QJsonArray tpath_to_jsonArray(QPainterPath tpath);
};

#endif // CONTROLSYSSERVICE_H
