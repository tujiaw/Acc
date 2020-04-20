#include "MainWidget.h"
#include <QtWidgets>
#include "component/qxtglobalshortcut/qxtglobalshortcut.h"
#include "LnkListView.h"
#include "model/LnkModel.h"
#include "LnkItemDelegate.h"
#include "common/Util.h"
#include "common/ForegroundWindowGuard.h"
#include "common/HttpRequest.h"
#include "controller/Acc.h"

const int TOP_HEIGHT = 70;
const QChar OPEN_URL_PREFIX = '<';
const QChar SEARCH_ENGINE_PREFIX = '>';

MainWidget::MainWidget(QWidget *parent)
    : QFrame(parent)
{
	tray_ = new SystemTray(this);
	connect(tray_, &SystemTray::sigReload, this, &MainWidget::slotReload);
	connect(tray_, &QSystemTrayIcon::activated, this, &MainWidget::slotTrayActivated);
	tray_->show();

	m_searchTimer = new QTimer(this);
	m_searchTimer->setInterval(200);
	connect(m_searchTimer, &QTimer::timeout, this, &MainWidget::slotSearchTimer);
    
    m_wallpaperTimer = new QTimer(this);
    m_wallpaperTimer->setInterval(3600 * 1000);
    connect(m_wallpaperTimer, &QTimer::timeout, this, &MainWidget::slotWallpaper);
    m_wallpaperTimer->start();

	mainShortcut_ = new QxtGlobalShortcut(this);
	connect(mainShortcut_, &QxtGlobalShortcut::activated, this, &MainWidget::slotMainShortcut);
	connect(Acc::instance(), &Acc::sigSetMainShortcut, this, &MainWidget::slotMainShortcutChanged);
	connect(Acc::instance(), &Acc::sigClearResult, this, &MainWidget::slotClearResult);

	// 从配置文件获取热键，如果不存在或注册失败则依次使用默认热键
	QString mainShortcutText = Acc::instance()->getSettingModel()->mainShortcutText();
	slotMainShortcutChanged(mainShortcutText);

	QVBoxLayout *mLayout = new QVBoxLayout(this);
	mLayout->setContentsMargins(10, 10, 10, 10);
	mLayout->setSpacing(10);

	m_lineEdit = new QLineEdit(this);
	m_lineEdit->setObjectName("SearchLineEdit");
	connect(m_lineEdit, &QLineEdit::textChanged, this, &MainWidget::slotTextChanged);

    m_headLabel = new QLabel(this);
    m_headLabel->setFixedHeight(15);
    m_headLabel->setHidden(true);

	m_lnkListView = new LnkListView(this);
	m_lnkListView->setModel(Acc::instance()->getLnkModel());
	m_lnkListView->setItemDelegate(new LnkItemDelegate(this));
	m_lnkListView->hide();

	mLayout->addWidget(m_lineEdit);
    mLayout->addWidget(m_headLabel);
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
			slotReturnPressed();
			return true;
		} else if (keyEvent->key() == Qt::Key_Escape) {
			this->parentWidget()->hide();
		} else if (keyEvent->modifiers() == Qt::ControlModifier) {
			if (keyEvent->key() == Qt::Key_N) {
				m_lnkListView->selectNext();
                return true;
			} else if (keyEvent->key() == Qt::Key_P) {
				m_lnkListView->selectPrev();
                return true;
			}
        } else if (object == m_lineEdit) {
            if (keyEvent->key() == Qt::Key_Down) {
                m_lnkListView->selectNext();
            } else if (keyEvent->key() == Qt::Key_Up) {
                m_lnkListView->selectPrev();
            }
        }
	} else if (this == object && event->type() == QEvent::Show) {
		m_lineEdit->selectAll();
		m_lineEdit->setFocus();
    }

	return false;
}

void MainWidget::slotReload()
{
	LnkModel *model = static_cast<LnkModel*>(m_lnkListView->model());
	model->load();

    auto indexList = Acc::instance()->getSettingModel()->getIndexList();
    for (auto iter = indexList.begin(); iter != indexList.end(); ++iter) {
        Acc::instance()->getLnkModel()->load(*iter);
    }
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

void MainWidget::slotMainShortcutChanged(const QString &textKey)
{
	if (textKey.isEmpty()) {
		QStringList keyList = QStringList() << "Alt+Space" << "Ctrl+Space" << "Shift+Space";
		for (auto iter = keyList.begin(); iter != keyList.end(); ++iter) {
			if (mainShortcut_->setShortcut(QKeySequence(*iter))) {
				Acc::instance()->getSettingModel()->setMainShortcutText(*iter);
				break;
			}
		}
		return;
	}

	QKeySequence newKey(textKey);
	QKeySequence oldKey = mainShortcut_->shortcut();
	if (!mainShortcut_->setShortcut(newKey)) {
		QMessageBox::warning(nullptr, tr("Warning"), tr("Set main shortcut failed!"));
		mainShortcut_->setShortcut(oldKey);
	} else {
		Acc::instance()->getSettingModel()->setMainShortcutText(textKey);
	}
}

void MainWidget::slotClearResult()
{
	m_lineEdit->clear();
}

void MainWidget::slotSearchTimer()
{
	m_searchTimer->stop();
	QString text = m_lineEdit->text().trimmed();
    LnkModel *model = qobject_cast<LnkModel*>(m_lnkListView->model());
    if (model) {
        model->filter(text.trimmed());
        m_headLabel->setText(model->head());
        m_headLabel->setHidden(model->head().isEmpty());
        m_lnkListView->setSelect(0);

        if (model->showCount() > 0) {
            if (m_lnkListView->isHidden()) {
                m_lnkListView->show();
            }
            int maxResult = Acc::instance()->getSettingModel()->maxResult();
            this->parentWidget()->setFixedHeight(qMin(maxResult, model->showCount()) * ROW_HEIGHT + TOP_HEIGHT + 12);
        } else {
            m_lnkListView->hide();
            this->parentWidget()->setFixedHeight(TOP_HEIGHT);
        }
    }
}

void MainWidget::slotTextChanged(const QString &text)
{
	LnkItemDelegate::setSearchText(text.trimmed());
	if (getPrefixAndText().first.isEmpty()) {
		m_searchTimer->stop();
		m_searchTimer->start();
	} else {
		m_lnkListView->hide();
		this->parentWidget()->setFixedHeight(TOP_HEIGHT);
	}
}

void MainWidget::slotReturnPressed()
{
	QPair<QString, QString> search = getPrefixAndText();
	if (search.first == OPEN_URL_PREFIX && Acc::instance()->getSettingModel()->enableOpenUrl()) {
		Util::shellExecute(getUrl());
	} else if (search.first == SEARCH_ENGINE_PREFIX && Acc::instance()->getSettingModel()->searchEngine().first) {
		QString searchEngine = Acc::instance()->getSettingModel()->searchEngine().second.toLower();
		if (!searchEngine.isEmpty()) {
			QString searchUrl;
			if (searchEngine.contains("baidu")) {
				searchUrl = "https://www.baidu.com/s?wd=%1";
			} else if (searchEngine.contains("bing")) {
				searchUrl = "http://cn.bing.com/search?q=%1";
			} else if (searchEngine.contains("google")) {
				searchUrl = "https://www.google.com/search?q=%1";
			}
			if (!searchUrl.isEmpty() && !search.second.isEmpty()) {
				searchUrl = searchUrl.arg(QString(QUrl::toPercentEncoding(search.second)));
				Util::shellExecute(searchUrl);
			}
		}
    } else if (m_lnkListView->isVisible()) {
		m_lnkListView->openIndex(m_lnkListView->currentIndex());
	} else {
        if (Util::shellExecute(search.second)) {
            if (search.second.indexOf(QRegExp("[a-z|A-Z]:")) == 0) {
                QFileInfo f(search.second);
                if (f.isFile()) {
                    Acc::instance()->getHitsModel()->increase(T_FILE, f.fileName(), f.absoluteFilePath());
                } else if (f.isDir()) {
                    Acc::instance()->getHitsModel()->increase(T_DIR, f.fileName(), f.absoluteFilePath());
                }
            }
        }
	}
}

void MainWidget::slotWallpaper()
{
    auto data = Acc::instance()->getSettingModel()->bindWallpaperUrl();
    if (data.first) {
        Acc::instance()->setBindWallpaper(data.second);
    }
}

QString MainWidget::getUrl() const
{
	QString url = m_lineEdit->text().trimmed();
	if (url.size() > 0 && url[0] == OPEN_URL_PREFIX) {
		url = url.mid(1).trimmed().toLower();
		QStringList headList = QStringList() << "http://" << "https://";
		for (auto iter = headList.begin(); iter != headList.end(); ++iter) {
			if (url.indexOf(*iter) == 0) {
				return url;
			}
		}
		return headList[0] + url;
	}
	return "";
}

QPair<QString, QString> MainWidget::getPrefixAndText() const
{
	QPair<QString, QString> result;
	QString text = m_lineEdit->text().trimmed();
	if (text.size() > 0) {
		if (text[0] == OPEN_URL_PREFIX || text[0] == SEARCH_ENGINE_PREFIX) {
			result.first = text[0];
			text = text.mid(1);
		}
	}
	result.second = text;
	return result;
}