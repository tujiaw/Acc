#include "MainWidget.h"
#include "qxtglobalshortcut/qxtglobalshortcut.h"
#include "LnkListView.h"
#include <QtWidgets>

MainWidget::MainWidget(QWidget *parent)
	: QFrame(parent)
{
	tray_ = new SystemTray(this);
	connect(tray_, &QSystemTrayIcon::activated, this, &MainWidget::slotTrayActivated);
	tray_->show();

	mainShortcut_ = new QxtGlobalShortcut(QKeySequence(Qt::CTRL + Qt::Key_Space), this);
	connect(mainShortcut_, &QxtGlobalShortcut::activated, this, &MainWidget::slotMainShortcut);

	QVBoxLayout *mLayout = new QVBoxLayout(this);
	mLayout->setContentsMargins(10, 10, 10, 10);
	mLayout->setSpacing(10);

	m_lineEdit = new QLineEdit(this);
	m_lineEdit->setFixedHeight(60);
	m_lnkListView = new LnkListView(this);
	mLayout->addWidget(m_lineEdit);
	mLayout->addWidget(m_lnkListView);

	qApp->installEventFilter(this);
}

MainWidget::~MainWidget()
{
	tray_->hide();
	tray_->deleteLater();
}

bool MainWidget::eventFilter(QObject *object, QEvent *event)
{
	if (QEvent::WindowDeactivate == event->type()) {
		this->parentWidget()->hide();
	}
	return false;
}

void MainWidget::slotTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger) {
		this->parentWidget()->showNormal();
		this->parentWidget()->raise();
	}
}

void MainWidget::slotMainShortcut()
{
	this->parentWidget()->showNormal();
	this->parentWidget()->activateWindow();
	this->parentWidget()->raise();
}
