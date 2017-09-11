#include "Acc.h"
#include <QtWidgets>
#include "component/TitleWidget.h"
#include "component/FramelessWidget.h"
#include "view/MainWidget.h"
#include "view/SettingWidget.h"

Acc::Acc()
{
}

Acc::~Acc()
{
}

Acc* Acc::instance()
{
	static Acc s_inst;
	return &s_inst;
}

void Acc::destory()
{
	QMapIterator<QString, QWidget*> i(widgets_);
	while (i.hasNext()) {
		i.next();
		QWidget *widget = i.value();
		delete widget;
	}
	widgets_.clear();
}

void Acc::openWidget(const QString &id)
{
	if (widgets_.contains(id)) {
		widgets_[id]->showNormal();
	} else {
		FramelessWidget *widget = new FramelessWidget();
		if (id == WidgetID::MAIN) {
			MainWidget *content = new MainWidget(widget);
			widget->setContent(content);

			int width = 650;
			widget->resize(width, content->height());
			QRect mainRect = qApp->desktop()->screenGeometry(qApp->desktop()->primaryScreen());
			QPoint movePoin((mainRect.width() - width) / 2, mainRect.height() / 4);
			widget->move(movePoin);
		} else if (id == WidgetID::SETTING) {
			TitleWidget *title = new TitleWidget(widget);
			SettingWidget *content = new SettingWidget(widget);
			widget->setTitle(title);
			widget->setContent(content);
			widget->resize(600, 450);
		}

		widgets_[id] = widget;
		return widget->show();
	}
}
