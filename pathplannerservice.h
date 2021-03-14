#ifndef PATHPLANNERSERVICE_H
#define PATHPLANNERSERVICE_H

#include <QObject>
#include <QtWebSockets/qwebsocketserver.h>
#include <QtWebSockets/qwebsocket.h>
#include <QDebug>

#include "modelconfig.h"

class PathPlannerService : public QObject
{
    Q_OBJECT
public:
    explicit PathPlannerService(QObject *parent = nullptr);
    ~PathPlannerService();
signals:
    /**
     * @brief сигнал о событии потери соединения с моделью
     */
    void pp_disconnected();

    /**
     * @brief сигнал о событии начала работы модели
     *        старт процесса моделирования.
     */
    void pp_started();

    /**
     * @brief сигнал о событии ошибки при запуске моделирования
     * @param QString - информация об ошибке
     */
    void pp_started_error(const QString);


    void pp_ready();
    void pp_unready();
    void pp_paused();
    void pp_stopped();
    void pp_current_pos(GroupPos, QTime);

public slots:
    void init();
    void start_request();
    void send_pause_request();
    void send_stop_request();
    void set_config_data(ModelConfig *config);
    void get_pos_request();

private slots:
    void pp_connected();
    void pp_disconnected_();
    void pp_readyRead(QString strReply);

private:
    QWebSocket *self_socket;
    QWebSocket *cs_socket;

    bool is_server;

    void handler_started_event();
    void handler_paused_event();
    void handler_stopped_event();
    void handler_ready_event();
    void handler_unready_event();
};

};

#endif // PATHPLANNERSERVICE_H
