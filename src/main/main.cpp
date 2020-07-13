#include <QtWidgets/QApplication>
#include "controller/Acc.h"
#include "common/DarkStyle.h"
#include "common/RunGuard.h"
#include "common/Util.h"
#include "common/LogHandler.h"
#include "net/BusService.h"
#include <thread>
#include <QHostInfo>

int main(int argc, char *argv[])
{
    RunGuard guard("69619FA7-4944-4CCA-BF69-83323F34D32F");
    if (!guard.tryToRun()) {
        return 0;
    }

#ifndef DEBUG
    qInstallMessageHandler(myMessageOutput);
#endif

    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/images/Acc.ico"));
    qDebug() << "==============================================";
    qDebug() << Util::getSystemInfo();
    qDebug() << "Local host:" << Util::getLocalHost();
    qDebug() << "==============================================";

    QString family = Acc::instance()->getSettingModel()->fontFamily();
    bool bold = Acc::instance()->getSettingModel()->isBold();
    CDarkStyle::setFontFamily(family, bold);
    CDarkStyle::assign();

	QString host = Acc::instance()->getSettingModel()->host();
	if (!host.isEmpty()) {
		BusService::instance().setHost(host);
		BusService::instance().start();
	}

	a.setQuitOnLastWindowClosed(false);
	QObject::connect(&a, &QApplication::aboutToQuit, []{ 
		BusService::instance().stop();
		Acc::instance()->destory();
	});
	Acc::instance()->openWidget(WidgetID::MAIN);
	Acc::instance()->setWindowOpacity(WidgetID::MAIN, Acc::instance()->getSettingModel()->mainOpacity());
    //Acc::instance()->openWidget(WidgetID::TOOLS);

    qDebug() << "app running";
    qDebug() << "config dir:" << Util::getConfigDir();
	return a.exec();
}
