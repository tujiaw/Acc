#include "Acc.h"
#include <QtWidgets/QApplication>
#include "DarkStyle.h"
#include "FramelessWidget.h"
#include "Util.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	QStringList strList = Util::getFiles(ScanDir::START_MENU);
	qDebug() << strList;
	// ÉèÖÃÖ÷Ìâ
	CDarkStyle::assign();

	// If this property is true, the applications quits when the last visible primary window (i.e. window with no parent) is closed.
	a.setQuitOnLastWindowClosed(false);

	QObject::connect(&a, &QApplication::aboutToQuit, []{ Acc::instance()->destory(); });

	// qDebug() << ScanDir::START_MENU;
	Acc::instance()->openWidget(WidgetID::MAIN);

	return a.exec();
}
