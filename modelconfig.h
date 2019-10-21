#ifndef MODELCONFIG_H
#define MODELCONFIG_H

#include <QObject>
#include <QPainterPath>
#include "unititem.h"
#include "trackitem.h"
#include "objectitem.h"
#include "paintscene.h"

struct ItemPos {
  qreal x;
  qreal y;
  qreal alfa;
  qreal k; // коэфф. кривизны траектории
};

// Скорости робота
struct ItemVel {
    qreal x;
    qreal w;
};

// Вектор состояния обьекта переноса или робота
struct RobotState {
    ItemPos pos;
    ItemVel vel;
    ItemPos pos_real;
    ItemVel vel_real;
};

struct ItemMaterial {
    QColor color;
    QString title;
    qreal accn_max;
    qreal acct_max;
};

struct GroupPos {
    RobotState object_pos;
    QList<RobotState> robots_pos;
};

/**
 * @brief The ModelConfig class use Singleton pattern for saving config data
 */
class ModelConfig : public QObject
{
    Q_OBJECT
public:
    explicit ModelConfig(QObject *parent = nullptr);
    static ModelConfig* Instance();

    /**
     * @brief величина шага за один такт отработки модели
     */
    double step;

    /**
     * @brief временной интервал между тактами отработки модели(мс)
     */
    int interval;
    int view_interval;
    double target_realtime_factor;

    /**
     * @brief максимальная скорость робота(которую он стремится достичь)
     */
    double vel_max;

    /**
     * @brief Настройки регулятора движения по траектории
     */
    double trajectory_P;
    double trajectory_vP;
    double trajectory_w_thres_offset;
    double trajectory_w_thres;
    double trajectory_wI;

    PaintScene* sceneObject;

    void addUnit(UnitItem *unit);
    void delUnit(UnitItem *unit);
    QList<UnitItem*> getUnits();

    void setStartPosition();
    GroupPos getStartPosition();

    QList<ItemMaterial> materials;
    ItemMaterial defaultMaterial;

    void setTrack(TrackItem *item);
    TrackItem *getTrack();
    QPainterPath getTrackPath();

    void setObject(ObjectItem *object);
    ObjectItem *getObject();

    QSize getSceneSize();
    void setSceneSize(QSize size);
    double getSceneBorderWidth();

    ItemMaterial getItemMaterialByColor(QColor color);

    QString getModelAddress();
    void setModelAddress(QString model_address);

    void jsonToScene(QJsonObject jo);
    void sceneToJson(QJsonObject* jo);
    static QJsonObject gpos_to_jsonObject(GroupPos gpos);
    static QJsonObject state_to_jsonObject(RobotState state);
    static QJsonObject pos_to_jsonObject(ItemPos pos);
    static QJsonObject vel_to_jsonObject(ItemVel vel);
    static GroupPos jsonObject_to_gpos(QJsonObject jo);
    static RobotState jsonObject_to_state(QJsonObject jo);
    static ItemPos jsonObject_to_pos(QJsonObject jo);
    static ItemVel jsonObject_to_vel(QJsonObject jo);
    static QJsonArray tpath_to_jsonArray(QPainterPath tpath);
    static QJsonObject point_to_jsonObject(QPointF point);
    static QJsonObject material_to_jsonObject(ItemMaterial material);
    ItemMaterial jsonObject_to_material(QJsonObject jo);
    QPointF jsonObject_to_point(QJsonObject jpoint);

private:
    static ModelConfig* _instance;

    QList<UnitItem*> units;
    TrackItem *track_item;
    GroupPos startPos;
    ObjectItem *object_item;
    QSize sceneSize;
    double sceneBorderWidth;
    QString modelAddress;
};

#endif // MODELCONFIG_H
