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
    explicit ModelConfig(QObject *parent = 0);
    static ModelConfig* Instance();

    /**
     * @brief величина шага за один такт отработки модели
     */
    double step;

    /**
     * @brief временной интервал между тактами отработки модели(мс)
     */
    int interval;
    double target_realtime_factor;

    /**
     * @brief максимальная скорость робота(которую он стремится достичь)
     */
    double vel_max;

    /**
     * @brief максимальное ускорение робота(которое он стремится достичь)
     */
    double accn_max;
    double acct_max;

    // Mutable values:
    double step_max;
    double step_min;
    int interval_max;
    int interval_min;

    PaintScene* sceneObject;

    void reset();

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

private:
    static ModelConfig* _instance;

    QList<UnitItem*> units;
    TrackItem *track_item;
    GroupPos startPos;
    ObjectItem *object_item;
    QSize sceneSize;
    double sceneBorderWidth;
};

#endif // MODELCONFIG_H
