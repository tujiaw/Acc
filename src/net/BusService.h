#pragma once

#include <QObject>
#include "BusConnection.h"

class BusService : public QObject
{
	Q_OBJECT

public:
	static BusService& instance();
	static const std::string& id();
	void setHost(const QString &host);
	bool start();
	void stop();

private:
	BusService(QObject *parent);
	~BusService();

	void onCommonMessage(const Message &msg);
	void onEchoMessage(const Message &req, Message &rsp);

private:
	std::unique_ptr<BusConnection> conn_;
	QString address_;
};

