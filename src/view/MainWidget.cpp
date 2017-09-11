#include "MainWidget.h"
#include <QtWidgets>
#include "component/qxtglobalshortcut/qxtglobalshortcut.h"
#include "LnkListView.h"
#include "model/LnkModel.h"
#include "LnkItemDelegate.h"
#include "common/Util.h"
#include "common/ForegroundWindowGuard.h"

const int TOP_HEIGHT = 70;
MainWidget::MainWidget(QWidget *parent)
	: QFrame(parent)
{
	tray_ = new SystemTray(this);
	connect(tray_, &SystemTray::sigSetting, this, &MainWidget::slotSetting);
	connect(tray_, &SystemTray::sigReload, this, &MainWidget::slotReload);
	connect(tray_, &QSystemTrayIcon::activated, this, &MainWidget::slotTrayActivated);
	tray_->show();

	QList<QKeySequence> keyList;
	keyList.append(QKeySequence(Qt::ALT + Qt::Key_Space));
	keyList.append(QKeySequence(Qt::CTRL + Qt::Key_Space));
	keyList.append(QKeySequence(Qt::SHIFT + Qt::Key_Space));
	mainShortcut_ = new QxtGlobalShortcut(this);
	connect(mainShortcut_, &QxtGlobalShortcut::activated, this, &MainWidget::slotMainShortcut);
	for (auto iter = keyList.begin(); iter != keyList.end(); ++iter) {
		if (mainShortcut_->setShortcut(*iter)) {
			break;
		}
	}

	QVBoxLayout *mLayout = new QVBoxLayout(this);
	mLayout->setContentsMargins(10, 10, 10, 10);
	mLayout->setSpacing(10);

	m_lineEdit = new QLineEdit(this);
	m_lineEdit->setObjectName("SearchLineEdit");
	connect(m_lineEdit, &QLineEdit::textChanged, this, &MainWidget::slotSearch);

	m_lnkListView = new LnkListView(this);
	m_lnkListView->setModel(new LnkModel(this));
	m_lnkListView->setItemDelegate(new LnkItemDelegate(this));
	m_lnkListView->hide();

	mLayout->addWidget(m_lineEdit);
	mLayout->addWidget(m_lnkListView, 1);

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
		else if (keyEvent->key() == Qt::Key_Escape) {
			this->parentWidget()->hide();
		}
	}
	else if (this == object && event->type() == QEvent::Show) {
		m_lineEdit->selectAll();
		m_lineEdit->setFocus();
	}

	return false;
}

void MainWidget::slotSetting()
{

}

void MainWidget::slotReload()
{
	LnkModel *model = static_cast<LnkModel*>(m_lnkListView->model());
	model->load();
}

void MainWidget::slotTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger) {
		slotMainShortcut();
	}
}

void MainWidget::slotMainShortcut()
{
	if (this->parentWidget()->isHidden()) {
		this->parentWidget()->show();
		this->parentWidget()->activateWindow();
		this->parentWidget()->raise();
		if (!this->parentWidget()->isActiveWindow()) {
			ForegroundWindowGuard guard;
			this->parentWidget()->activateWindow();
			this->parentWidget()->raise();
		}
	}
	else {
		this->parentWidget()->hide();
	}
}

void MainWidget::slotSearch(const QString &text)
{
	LnkModel *model = static_cast<LnkModel*>(m_lnkListView->model());
	LnkItemDelegate *delegate = static_cast<LnkItemDelegate*>(m_lnkListView->itemDelegate());
	model->filter(text.trimmed());
	m_lnkListView->setSelect(0);
	
	if (model->showCount() > 0) {
		if (m_lnkListView->isHidden()) {
			m_lnkListView->show();
		}
		this->parentWidget()->setFixedHeight(qMin(5, model->showCount()) * ROW_HEIGHT + TOP_HEIGHT + 12);
	}
	else {
		m_lnkListView->hide();
		this->parentWidget()->setFixedHeight(TOP_HEIGHT);
	}
	
}