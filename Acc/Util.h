#pragma once

#include <QObject>

namespace Util {

	QStringList getFiles(QString path, bool containsSubDir = true);
	QString getPinyin(const QString &text);
	QPair<QString, QString> getPinyinAndJianpin(const QString &text);

}
