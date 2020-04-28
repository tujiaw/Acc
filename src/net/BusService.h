#pragma once

#include <QObject>
#include "BusConnection.h"

class BusService : public QObject
{
	Q_OBJECT

public:
	static BusService& instance();
	bool start();
	void stop();

private:
	BusService(QObject *parent);
	~BusService();

	void onCommonMessage(const Message &msg);

private:
	std::unique_ptr<BusConnection> conn_;
};

