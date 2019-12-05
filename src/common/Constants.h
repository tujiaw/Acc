#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <QString>
#include <QDir>
#include <QSize>

namespace WidgetID {
	const QString MAIN = "MAIN";
	const QString SETTING = "SETTING";
    const QString CLIPBOARD = "CLIPBOARD";
    const QString TOOLS = "TOOLS";
}

namespace ScanDir {
	const QString START_MENU = QDir::homePath() + "/AppData/Roaming/Microsoft/Windows/Start Menu/Programs";
	const QString PROGRAM_DATA = QDir::rootPath() + "/ProgramData/Microsoft/Windows/Start Menu/Programs";
}

const QSize LNK_ICON_SIZE(40, 40);
const int ROW_HEIGHT = 54;
const int SEARCH_FONT_SIZE = 22;
const int INDEX_ROW_HEIGHT = 30;
const int MAX_OPACITY = 10;

#endif // CONSTANTS_H_
