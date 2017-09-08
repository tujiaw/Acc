#include "MainWidget.h"
#include "qxtglobalshortcut/qxtglobalshortcut.h"
#include "LnkListView.h"
#include "LnkModel.h"
#include "LnkItemDelegate.h"
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
	m_lineEdit->setFixedHeight(50);
	m_lnkListView = new LnkListView(this);
	m_lnkListView->setModel(new LnkModel(this));
	m_lnkListView->setItemDelegate(new LnkItemDelegate(this));

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
		return false;
	}

	if (event->type() == QEvent::KeyPress) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
			m_lnkListView->openIndex(m_lnkListView->currentIndex());
			return true;
		} else if (object == m_lineEdit) {
			if (keyEvent->key() == Qt::Key_Down) {
				m_lnkListView->selectNext();
			} else if (keyEvent->key() == Qt::Key_Up) {
				m_lnkListView->selectPrev();
			}
		}
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
