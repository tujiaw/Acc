#include "BusService.h"
#include <QDebug>
#include "Util.h"

const std::string BUS_HOST = "118.24.4.114";
const std::string ACC_COMMON = "acc_common";
const std::string ACC_REPORT = "acc_report";

BusService::BusService(QObject *parent)
	: QObject(parent)
{
}

BusService::~BusService()
{
}

BusService& BusService::instance()
{
	static BusService s_inst(nullptr);
	return s_inst;
}

bool BusService::start()
{
	conn_.reset(new BusConnection(BUS_HOST));
	conn_->AddTopicServer(ACC_COMMON, std::bind(&BusService::onCommonMessage, this, std::placeholders::_1));
	return conn_->isOpen();
}

void BusService::stop()
{
	if (conn_) {
		conn_->Close();
	}
}

void BusService::onCommonMessage(const Message &msg)
{
	std::string subject = msg.getSubject();
	qDebug() << "on common msg subject:" << QString::fromStdString(subject);
	qDebug() << "content:" << QString::fromStdString(msg.getContent());
	if (subject == "report") {
		QStringList infoList;
		infoList << ("[system]" + Util::getSystemInfo());
		infoList << ("[host]" + Util::getLocalHost());
		MessagePtr rsp(new Message());
		rsp->setContent(infoList.join("\n").toStdString());
		conn_->PostMsg(ACC_REPORT, rsp, 3, [](const MessagePtr &req, const MessagePtr &rsp) {
			qDebug() << "report finished!";
		});
	}
}