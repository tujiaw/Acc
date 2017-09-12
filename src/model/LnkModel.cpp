#include "LnkModel.h"
#include <QFileIconProvider>
#include <QDebug>
#include <QStandardPaths>
#include <QImageReader>
#include "common/Util.h"

LnkModel::LnkModel(QObject *parent)
	: QAbstractListModel(parent)
{
	load();
}

LnkModel::~LnkModel()
{
}

void LnkModel::load()
{
	QFileInfo info;
	for (int i = pdata_.size() - 1; i >= 0; i--) {
		info.setFile(pdata_[i]->lnkPath);
		if (!info.exists()) {
			pdata_.removeAt(i);
		}
	}

	QFileIconProvider iconProvider;
	QStringList strList = getAllLnk();
	for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
		info.setFile(*iter);
		QString target = info.isSymLink() ? info.symLinkTarget() : info.filePath();
		if (target.isEmpty()) {
			continue;
		}

		QSharedDataPointer<LnkData> p(new LnkData());
		// ������
		p->lnkName = info.fileName().remove(".lnk", Qt::CaseInsensitive);
		// ����·��
		p->lnkPath = info.filePath();
		// Ŀ¼��ִ�г���·��
		p->targetPath = target;
		// Ŀ������
		p->targetName = QFileInfo(p->targetPath).baseName();

		// �����ظ�
		QString key = p->key();
		if (isExist(key)) {
			continue;
		}

		QPair<QString, QString> pinyin = Util::getPinyinAndJianpin(p->lnkName.trimmed());
		p->pinyin = pinyin.first;
		p->jianpin = pinyin.second; 

		p->icon = iconProvider.icon(QFileInfo(target));
		if (p->icon.isNull()) {
			p->icon = iconProvider.icon(QFileIconProvider::File);
		}

		pdata_.append(p);
		if (p->pinyin.isEmpty() || p->jianpin.isEmpty()) {
			qDebug() << "hanzi2pinyin empty, text:" << p->lnkName;
		}
	}

	qDebug() << "lnk count:" << pdata_.size();
}

QStringList LnkModel::getAllLnk() const
{
	// ɨ���Ŀ¼
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

	// ��ӡ����ɨ��Ŀ¼
	QStringList logStr;
	logStr << desktopList << startMenuList << programDataDir;
	qDebug() << logStr;

	// ��ȡĿ¼�е�����
	QStringList result;
	for (auto iter = desktopList.begin(); iter != desktopList.end(); ++iter) {
		result.append(Util::getFiles(*iter, false)); // ���治�ݹ���Ŀ¼
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
			if (p->lnkName.contains(text, Qt::CaseInsensitive)) {
				pfilterdata_.append(*iter);
			}
			else if (p->targetName.contains(text, Qt::CaseInsensitive)) {
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

bool LnkModel::isExist(const QString &key)
{
	for (int i = 0; i < pdata_.size(); i++) {
		if (pdata_[i]->key() == key) {
			return true;
		}
	}
	return false;
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