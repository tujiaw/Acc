#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <QString>
#include <QDir>
#include <QSize>

namespace WidgetID {
	const QString MAIN = "MAIN";
}

namespace ScanDir {
	const QString START_MENU = QDir::homePath() + "/AppData/Roaming/Microsoft/Windows/Start Menu/Programs";
}

const QSize LNK_ICON_SIZE(40, 40);
const int ROW_HEIGHT = 54;

#endif // CONSTANTS_H_
