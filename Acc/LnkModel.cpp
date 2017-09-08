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
		QString filepath = target.isEmpty() ? info.filePath() : target;

		QSharedDataPointer<LnkData> p(new LnkData());
		p->name = filename.remove(".lnk", Qt::CaseInsensitive);
		p->basename = QFileInfo(filepath).baseName();
		p->path = filepath;
		if (!target.isEmpty()) {
			p->pixmap = iconProvider.icon(QFileInfo(target)).pixmap(LNK_ICON_SIZE).scaled(LNK_ICON_SIZE);
		}
		else {
			p->pixmap = iconProvider.icon(info).pixmap(LNK_ICON_SIZE);
		}
		pdata_.append(p);
	}
}

LnkModel::~LnkModel()
{
}

void LnkModel::filter(const QString &text)
{
	pfilterdata_.clear();
	for (auto iter = pdata_.begin(); iter != pdata_.end(); ++iter) {
		const LnkData *p = (*iter).data();
		if (p->name.contains(text, Qt::CaseInsensitive)) {
			pfilterdata_.append(*iter);
		}
		else if (p->basename.contains(text, Qt::CaseInsensitive)) {
			pfilterdata_.append(*iter);
		}
	}
	int count = pfilterdata_.size();
	emit dataChanged(this->index(0, 0), this->index(qMax(count, 0), 0));
}

int LnkModel::rowCount(const QModelIndex &parent) const
{
	return pfilterdata_.count();
}

QVariant LnkModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if (role == Qt::DisplayRole && row < pfilterdata_.size()) {
		return pfilterdata_[row]->toVariant();
	}
	return QVariant();
}