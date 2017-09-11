#include "SettingWidget.h"
#include <QAction>
#include <QKeyEvent>
#include <QDebug>

SettingWidget::SettingWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	connect(ui.pbFirst, &QPushButton::clicked, this, &SettingWidget::slotFirst);
	connect(ui.pbSecond, &QPushButton::clicked, this, &SettingWidget::slotSecond);
	ui.listWidget->addItem(tr("Hot Key"));
}

SettingWidget::~SettingWidget()
{
}

void SettingWidget::slotFirst()
{
	ui.leFirst->clear();
}

void SettingWidget::slotSecond()
{

}

void SettingWidget::keyPressEvent(QKeyEvent *e)
{
	QKeySequence key(e->key());
	qDebug() << e->text();
	qDebug() << key.toString();
	QWidget::keyPressEvent(e);
}