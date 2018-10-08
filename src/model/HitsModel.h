#pragma once

#include <QObject>
#include <QVariantList>

enum HitType {
    T_LNK,
    T_DIR,
    T_FILE
};

struct HitData {
    HitType type = T_LNK;
    int hits = 0;
    QString lasttime;
    QString title;
    QString subtitle;
};

class QTimer;
class HitsModel : public QObject
{
	Q_OBJECT
public:
	HitsModel(QObject *parent = 0);
	~HitsModel();

	void increase(HitType type, const QString &title, const QString &subtitle);
    int hits(HitType type, const QString &title, const QString &subtitle);

private:
    void init();
    void save();
    void delaySave();

private:
    QList<HitData> data_;
    QTimer *timer_;
};