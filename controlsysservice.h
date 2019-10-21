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
    explicit ControlSysService(QObject *parent = nullptr);
    ~ControlSysService();

signals:
    /**
     * @brief сигнал о событии потери соединения с моделью
     */
    void cs_disconnected();

    /**
     * @brief сигнал о событии начала работы модели
     *        старт процесса моделирования.
     */
    void cs_started();

    /**
     * @brief сигнал о событии ошибки при запуске моделирования
     * @param QString - информация об ошибке
     */
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
    void set_config_data(ModelConfig *config);
    void get_pos_request();

private slots:
    void cs_connected();
    void cs_disconnected_();
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
    void handler_get_groundType(QJsonObject jsonObject);
};

#endif // CONTROLSYSSERVICE_H
