#include "BusService.h"
#include <QDebug>
#include <QHostInfo>
#include "Util.h"
#include "WinTool.h"

const std::string ACC_TOPIC_COMMON = "acc_common";
const std::string ACC_QUEUE_ECHO = "acc_queue_echo";
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

const std::string& BusService::id()
{
	static std::string s_id;
	static std::once_flag once;
	std::call_once(once, []() {
		s_id = Uuid();
	});
	return s_id;
}

void BusService::setHost(const QString &host)
{
	QHostInfo hostInfo = QHostInfo::fromName(host);
	QList<QHostAddress> addrList = hostInfo.addresses();
	if (addrList.size() > 0) {
		address_ = addrList[0].toString();
	}
}

bool BusService::start()
{
	if (address_.isEmpty()) {
		return false;
	}
	conn_.reset(new BusConnection(address_.toStdString()));
	conn_->AddQueueServer(ACC_QUEUE_ECHO, std::bind(&BusService::onEchoMessage, this, std::placeholders::_1, std::placeholders::_2));
	conn_->AddTopicServer(ACC_TOPIC_COMMON, std::bind(&BusService::onCommonMessage, this, std::placeholders::_1));
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
		qpid::types::Variant::Map data;
		data["id"] = id();
		data["system"] = Util::getSystemInfo().toStdString();
		data["host"] = Util::getLocalHost().toStdString();
		data["pid"] = (uint32_t)WinTool::GetCurrentPid();
		data["name"] = "Acc";
		MessagePtr rsp(new Message());
		rsp->setContentObject(data);
		conn_->PostMsg(ACC_REPORT, rsp, 3, [](const MessagePtr &req, const MessagePtr &rsp) {
			qDebug() << "report finished!";
		});
	}
	else if (subject == "lock") {
		std::string targetId = msg.getContent();
		if (targetId.empty() || targetId == BusService::id()) {
			qDebug() << "will lock system, target id:" << QString::fromStdString(targetId);
			WinTool::LockSystem();
		}
	}
}

void BusService::onEchoMessage(const Message &req, Message &rsp)
{
	rsp.setContent(req.getContent());
}
