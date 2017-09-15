#include "ControlPanelModel.h"

ControlPanelModel::ControlPanelModel(QObject *parent)
	: QObject(parent)
{
	Item firewall;
	firewall.title = QStringLiteral("防火墙");
	firewall.subtitle = QStringLiteral("使用Windows防火墙来帮助保护您的计算机");
	firewall.path = "Firewall.cpl";

	Item hdwwiz;
	hdwwiz.title = QStringLiteral("设备管理器");
	hdwwiz.subtitle = QStringLiteral("设备管理器提供计算机上所安装硬件的图形视图");
	hdwwiz.path = "hdwwiz.cpl";

	Item intl;
	hdwwiz.title = QStringLiteral("区域和语言");
	hdwwiz.subtitle = QStringLiteral("区域和语言");
	hdwwiz.path = "intl.cpl";

	Item desk;
	hdwwiz.title = QStringLiteral("屏幕分辨率");
	hdwwiz.subtitle = QStringLiteral("更改显示器的外观");
	hdwwiz.path = "desk.cpl";

	Item appwiz;
	hdwwiz.title = QStringLiteral("程序和功能");
	hdwwiz.subtitle = QStringLiteral("卸载或更改程序");
	hdwwiz.path = "appwiz.cpl";

	Item sysdm;
	hdwwiz.title = QStringLiteral("系统属性");
	hdwwiz.subtitle = QStringLiteral("更改计算机的系统信息");
	hdwwiz.path = "sysdm.cpl";
}

void ControlPanelModel::addItem(const ControlPanelModel::Item &item)
{
	data_.append(item);
}

const QList<ControlPanelModel::Item>& ControlPanelModel::items() const
{
	return data_;
}