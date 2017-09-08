#pragma once

#include <QObject>
#include <QMap>
#include "Constants.h"

class Acc : public QObject
{
	Q_OBJECT

public:
	static Acc* instance();
	void destory();
	void openWidget(const QString &id);

private:
	Acc();
	~Acc();

private:
	QMap<QString, QWidget*> widgets_;
};
