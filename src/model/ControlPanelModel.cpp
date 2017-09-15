#include "ControlPanelModel.h"

ControlPanelModel::ControlPanelModel(QObject *parent)
	: QObject(parent)
{
	Item firewall;
	firewall.title = QStringLiteral("����ǽ");
	firewall.subtitle = QStringLiteral("ʹ��Windows����ǽ�������������ļ����");
	firewall.path = "Firewall.cpl";

	Item hdwwiz;
	hdwwiz.title = QStringLiteral("�豸������");
	hdwwiz.subtitle = QStringLiteral("�豸�������ṩ�����������װӲ����ͼ����ͼ");
	hdwwiz.path = "hdwwiz.cpl";

	Item intl;
	hdwwiz.title = QStringLiteral("���������");
	hdwwiz.subtitle = QStringLiteral("���������");
	hdwwiz.path = "intl.cpl";

	Item desk;
	hdwwiz.title = QStringLiteral("��Ļ�ֱ���");
	hdwwiz.subtitle = QStringLiteral("������ʾ�������");
	hdwwiz.path = "desk.cpl";

	Item appwiz;
	hdwwiz.title = QStringLiteral("����͹���");
	hdwwiz.subtitle = QStringLiteral("ж�ػ���ĳ���");
	hdwwiz.path = "appwiz.cpl";

	Item sysdm;
	hdwwiz.title = QStringLiteral("ϵͳ����");
	hdwwiz.subtitle = QStringLiteral("���ļ������ϵͳ��Ϣ");
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