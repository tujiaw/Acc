#include "Acc.h"
#include <QtWidgets>
#include "FramelessWidget.h"
#include "MainWidget.h"

Acc::Acc() : QObject(nullptr)
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
			widget->setFixedWidth(600);
		}

		widgets_[id] = widget;
		return widget->show();
	}
}
