#include "LnkModel.h"
#include "Util.h"
#include "Constants.h"
#include <QFileIconProvider>
#include <QDebug>

LnkModel::LnkModel(QObject *parent)
	: QAbstractListModel(parent)
{
	QFileIconProvider iconProvider;
	QStringList strList = Util::getFiles(ScanDir::START_MENU);
	for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
		QFileInfo info(*iter);
		QString target;
		if (info.isSymLink()) {
			target = info.symLinkTarget();
		}
		QString filename = info.fileName();
		QString filepath = info.filePath();
		QVariantMap vm;
		vm["name"] = filename;
		vm["path"] = target.isEmpty() ? filepath : target;
		if (!target.isEmpty()) {
			vm["icon"] = iconProvider.icon(QFileInfo(target)).pixmap(LNK_ICON_SIZE).scaled(LNK_ICON_SIZE);
		}
		else {
			vm["icon"] = iconProvider.icon(info).pixmap(LNK_ICON_SIZE);
		}
		
		data_.append(vm);
	}
}

LnkModel::~LnkModel()
{
}

int LnkModel::rowCount(const QModelIndex &parent) const
{
	return data_.count();
}

QVariant LnkModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		return data_[index.row()];
	}
	return QVariant();
}