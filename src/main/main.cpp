#include <QtWidgets/QApplication>
#include "controller/Acc.h"
#include "common/DarkStyle.h"
#include "common/RunGuard.h"
#include "common/Util.h"

int main(int argc, char *argv[])
{	
	RunGuard guard("69619FA7-4944-4CCA-BF69-83323F34D32F");
	if (!guard.tryToRun()) {
		return 0;
	}

	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/images/Acc.ico"));

	QString family = Acc::instance()->getSettingModel()->fontFamily();
	bool bold = Acc::instance()->getSettingModel()->isBold();
	CDarkStyle::setFontFamily(family, bold);
	CDarkStyle::assign();

	a.setQuitOnLastWindowClosed(false);
	QObject::connect(&a, &QApplication::aboutToQuit, []{ Acc::instance()->destory(); });
	Acc::instance()->openWidget(WidgetID::MAIN);
	Acc::instance()->setWindowOpacity(WidgetID::MAIN, Acc::instance()->getSettingModel()->mainOpacity());

	return a.exec();
}
