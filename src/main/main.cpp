#include <QtWidgets/QApplication>
#include "controller/Acc.h"
#include "common/DarkStyle.h"
#include "common/RunGuard.h"

bool isDoubleEqual(double d1, double d2, double EPSINON = 0.000001)
{
	return abs(d1 - d2) < EPSINON;
}


int main(int argc, char *argv[])
{
	RunGuard guard("69619FA7-4944-4CCA-BF69-83323F34D32F");
	if (!guard.tryToRun()) {
		return 0;
	}

	QApplication a(argc, argv);
	a.setWindowIcon(QIcon(":/images/Acc.ico"));
	CDarkStyle::assign();
	a.setQuitOnLastWindowClosed(false);
	QObject::connect(&a, &QApplication::aboutToQuit, []{ Acc::instance()->destory(); });
	Acc::instance()->openWidget(WidgetID::MAIN);
	return a.exec();
}