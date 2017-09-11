#include "SettingWidget.h"
#include <QAction>

SettingWidget::SettingWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	QAction *acHotKey = new QAction(tr("Hot Key"), this);
	ui.listWidget->addAction(acHotKey);
}

SettingWidget::~SettingWidget()
{
}
