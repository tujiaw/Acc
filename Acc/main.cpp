#include "Acc.h"
#include <QtWidgets/QApplication>
#include "DarkStyle.h"
#include "FramelessWidget.h"
#include "Util.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);

	a.setWindowIcon(QIcon(":/images/Acc.ico"));
	CDarkStyle::assign();
	a.setQuitOnLastWindowClosed(false);
	QObject::connect(&a, &QApplication::aboutToQuit, []{ Acc::instance()->destory(); });
	Acc::instance()->openWidget(WidgetID::MAIN);

	return a.exec();
}
