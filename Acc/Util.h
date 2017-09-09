#pragma once

#include <QObject>

namespace Util {
	QStringList getFiles(QString path, bool containsSubDir = true);
}
