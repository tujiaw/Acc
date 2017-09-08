#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <QString>
#include <QDir>

namespace WidgetID {
	const QString MAIN = "MAIN";
}

namespace ScanDir {
	const QString START_MENU = QDir::homePath() + "/AppData/Roaming/Microsoft/Windows/Start Menu/Programs";
}

#endif // CONSTANTS_H_
