#include "Acc.h"
#include <QtWidgets>
#include "component/TitleWidget.h"
#include "component/FramelessWidget.h"
#include "view/MainWidget.h"
#include "view/ClipboardWidget.h"
#include "view/SettingWidget.h"
#include "common/Util.h"
#include "common/HttpRequest.h"

Acc::Acc()
	: lnkModel_(nullptr)
	, settingModel_(nullptr)
	, hitsModel_(nullptr)
{
    getHitsModel();
}

Acc::~Acc()
{
	destory();
}

Acc* Acc::instance()
{
	static Acc s_inst;
	return &s_inst;
}

void Acc::destory()
{
	for (auto iter = widgets_.begin(); iter != widgets_.end(); ++iter) {
		(*iter)->close();
	}
	widgets_.clear();

	if (lnkModel_) {
		lnkModel_->deleteLater();
		lnkModel_ = nullptr;
	}
	
	if (settingModel_) {
		settingModel_->deleteLater();
		settingModel_ = nullptr;
	}

	if (hitsModel_) {
		hitsModel_->deleteLater();
		hitsModel_ = nullptr;
	}
}

void Acc::openWidget(const QString &id)
{
	if (widgets_.contains(id)) {
		widgets_[id]->showNormal();
	} else {
		bool isShow = true;
		FramelessWidget *widget = new FramelessWidget();
		if (id == WidgetID::MAIN) {
			isShow = false;
			MainWidget *content = new MainWidget(widget);
			widget->setContent(content);

			int width = 650;
			widget->resize(650, content->height());
			QRect mainRect = qApp->desktop()->screenGeometry(qApp->desktop()->primaryScreen());
			QPoint movePoin((mainRect.width() - width) / 2, mainRect.height() / 4);
			widget->move(movePoin);
		} else if (id == WidgetID::SETTING) {
			TitleWidget *title = new TitleWidget(widget);
			title->setTitle(tr("Setting"));
			connect(title, &TitleWidget::sigClose, [this] { closeWidget(WidgetID::SETTING); });
			SettingWidget *content = new SettingWidget(widget);
			widget->setTitle(title);
			widget->setContent(content);
			widget->resize(650, 450);
        } else if (id == WidgetID::CLIPBOARD) {
            TitleWidget *title = new TitleWidget(widget);
            title->setTitle(tr("Clipboard"));
            title->setMinimizeVisible(true);
            Qt::WindowFlags flags = widget->windowFlags();
            flags |= Qt::WindowStaysOnTopHint;
            widget->setWindowFlags(flags);
            connect(title, &TitleWidget::sigClose, [this] { closeWidget(WidgetID::CLIPBOARD); });
            connect(title, &TitleWidget::sigMinimize, [this] { minimizeWidget(WidgetID::CLIPBOARD); });
            ClipboardWidget *content = new ClipboardWidget(widget);
            connect(content, &ClipboardWidget::sigHide, [this] { minimizeWidget(WidgetID::CLIPBOARD); });
            widget->setTitle(title);
            widget->setContent(content);
            widget->resize(400, 600);
        }

		widgets_[id] = widget;
		if (isShow) {
			widget->show();
		}
	}
}

void Acc::closeWidget(const QString &id)
{
	if (widgets_.contains(id)) {
		widgets_[id]->close();
		widgets_.remove(id);
	}
}

void Acc::hideWidget(const QString &id)
{
	if (widgets_.contains(id)) {
		widgets_[id]->hide();
	}
}

void Acc::minimizeWidget(const QString &id)
{
    if (widgets_.contains(id)) {
        widgets_[id]->showMinimized();
    }
}

void Acc::restoreWidget(const QString &id)
{
    if (widgets_.contains(id)) {
        widgets_[id]->showNormal();
    }
}

LnkModel* Acc::getLnkModel()
{
	if (!lnkModel_) {
		lnkModel_ = new LnkModel(this);
	}
	return lnkModel_;
}

SettingModel* Acc::getSettingModel()
{
	if (!settingModel_) {
		settingModel_ = new SettingModel(this);
	}
	return settingModel_;
}

HitsModel* Acc::getHitsModel()
{
	if (!hitsModel_) {
		hitsModel_ = new HitsModel(this);
	}
	return hitsModel_;
}

QWidget* Acc::getContentWidget(const QString &id)
{
	QWidget *result = nullptr;
	if (widgets_.contains(id)) {
		result = widgets_[id];
	}
	return result;
}

void Acc::setWindowOpacity(const QString &id, int level)
{
	if (widgets_.contains(id)) {
		widgets_[id]->setWindowOpacity(level * 0.1);
	}
}

void Acc::setBindWallpaper(int index)
{
    index = qMin(index, 10);
    index = qMax(index, 0);
    QString url = QString("https://bing.biturl.top/?index=%1").arg(index);

    auto getFileName = [](const QString &url) -> QString {
        QStringList strList = url.split(QRegExp("[?, &]"), QString::SkipEmptyParts);
        foreach(const QString &str, strList) {
            QStringList tempList = str.split('=', QString::SkipEmptyParts);
            if (tempList.size() > 1 && tempList[0].toLower() == "id") {
                return tempList[1];
            }
        }
        return "";
    };

    auto getLocalPath = [](QString &name) -> QString {
        QString bmpName = name;
        int pos = name.lastIndexOf('.');
        if (pos > 0) {
            bmpName = name.mid(0, pos) + ".BMP";
        }
        QDir dir(Util::getImagesDir());
        if (dir.exists(bmpName)) {
            return dir.absoluteFilePath(bmpName);
        }
        return "";
    };

    auto downloadImageResponse = [=](int err, const QString &url, const QByteArray &data) {
        if (err == 0) {
            QString name = getFileName(url);
            if (!name.isEmpty()) {
                QString filepath = Util::getImagesDir() + "/" + name;
                QFile f(filepath);
                f.open(QIODevice::WriteOnly);
                QDataStream out(&f);
                out.writeRawData(data.constData(), data.size());
                f.flush();
                Util::setWallpaper(filepath);
            }
        }
    };

    auto imageUrlResponse = [=](int err, const QString &url, const QByteArray &data) {
        if (err == 0) {
            QVariantMap vm = Util::json2map(data);
            QString imageUrl = vm["url"].toString();
            if (!imageUrl.isEmpty()) {
                QString name = getFileName(imageUrl);
                QString path = getLocalPath(name);
                if (!path.isEmpty()) {
                    Util::setWallpaperBMP(path);
                } else {
                    HttpRequestCenter::instance()->get(imageUrl, downloadImageResponse);
                }
            }
        }
        qDebug() << data;
    };

    HttpRequestCenter::instance()->get(url, imageUrlResponse);
}
