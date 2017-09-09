#include "LnkModel.h"
#include "Util.h"
#include "Constants.h"
#include <QFileIconProvider>
#include <QDebug>
#include <QStandardPaths>

LnkModel::LnkModel(QObject *parent)
	: QAbstractListModel(parent)
{
	QFileIconProvider iconProvider;
	QStringList strList = getAllLnk();
	QStringList keyList;
	QStringList xx;
	for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
		QFileInfo info(*iter);
		QString target;
		if (info.isSymLink()) {
			target = info.symLinkTarget();
		} else {
			continue;
		}

		QString filename = info.fileName();
		QString filepath = target.isEmpty() ? info.filePath() : target;

		QSharedDataPointer<LnkData> p(new LnkData());
		p->name = filename.remove(".lnk", Qt::CaseInsensitive);
		p->path = filepath;

		// 过滤重复
		QString key = p->name + "-" + p->path;
		if (keyList.contains(key)) {
			continue;
		}
		keyList.append(key);

		p->basename = QFileInfo(filepath).baseName();
		QPair<QString, QString> pinyin = Util::getPinyinAndJianpin(p->name.trimmed());
		p->pinyin = pinyin.first;
		p->jianpin = pinyin.second;
		p->pixmap = iconProvider.icon(QFileInfo(target)).pixmap(LNK_ICON_SIZE).scaled(LNK_ICON_SIZE);
		if (p->pixmap.isNull()) {
			p->pixmap = iconProvider.icon(info).pixmap(LNK_ICON_SIZE);
		}
		pdata_.append(p);
	}
	qDebug() << "lnk count:" << keyList.size();
}

LnkModel::~LnkModel()
{
}

QStringList LnkModel::getAllLnk() const
{
	// 扫描的目录
	QStringList desktopList = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
	QStringList startMenuList = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
	QString programDataDir = QDir::rootPath() + "/ProgramData/Microsoft/Windows/Start Menu/Programs";
	
	const QString programData = "ProgramData";
	QStringList tempList = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
	for (auto iter = tempList.begin(); iter != tempList.end(); ++iter) {
		int pos = iter->indexOf(programData, Qt::CaseInsensitive);
		if (pos > 0) {
			QString temp = iter->mid(0, pos + programData.size()) + "/Microsoft/Windows/Start Menu/Programs";
			if (QDir(temp).exists()) {
				programDataDir = temp;
				break;
			}
		}
	}

	// 打印所有扫描目录
	QStringList logStr;
	logStr << desktopList << startMenuList << programDataDir;
	qDebug() << logStr;

	// 获取目录中的链接
	QStringList result;
	for (auto iter = desktopList.begin(); iter != desktopList.end(); ++iter) {
		result.append(Util::getFiles(*iter, false)); // 桌面不递归子目录
	}
	for (auto iter = startMenuList.begin(); iter != startMenuList.end(); ++iter) {
		result.append(Util::getFiles(*iter));
	}
	result.append(Util::getFiles(programDataDir));

	return result;
}

void LnkModel::filter(const QString &text)
{
	pfilterdata_.clear();
	if (!text.isEmpty()) {
		for (auto iter = pdata_.begin(); iter != pdata_.end(); ++iter) {
			const LnkData *p = (*iter).data();
			if (p->name.contains(text, Qt::CaseInsensitive)) {
				pfilterdata_.append(*iter);
			}
			else if (p->basename.contains(text, Qt::CaseInsensitive)) {
				pfilterdata_.append(*iter);
			}
			else if (p->jianpin.contains(text, Qt::CaseInsensitive)) {
				pfilterdata_.append(*iter);
			}
			else if (p->pinyin.contains(text, Qt::CaseInsensitive)) {
				pfilterdata_.append(*iter);
			}
		}
	}
	int count = pfilterdata_.size();
	emit dataChanged(this->index(0, 0), this->index(qMax(count, 0), 0));
}

int LnkModel::totalCount() const
{
	return pdata_.size();
}

int LnkModel::showCount() const
{
	return pfilterdata_.size();
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