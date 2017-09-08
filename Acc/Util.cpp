#include "Util.h"
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

namespace Util {
	QStringList getFiles(QString path)
	{
		QStringList result;
		QDirIterator curit(path, QStringList(), QDir::Files);
		while (curit.hasNext()) {
			result.push_back(curit.next());
		}
		QDirIterator subit(path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
		while (subit.hasNext()) {
			result.push_back(subit.next());
		}
		return result;
	}

}
