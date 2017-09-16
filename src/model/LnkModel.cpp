#include "LnkModel.h"
#include <QFileIconProvider>
#include <QDebug>
#include <QStandardPaths>
#include <QImageReader>
#include "common/Util.h"
#include "controller/Acc.h"
#include "common/FileVersionInfo.h"

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
	QStringList strList = Util::getAllLnk();
	for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
		info.setFile(*iter);
		QString target = info.isSymLink() ? info.symLinkTarget() : info.filePath();
		if (target.isEmpty()) {
			continue;
		}

		QSharedDataPointer<LnkData> p(new LnkData());
		// 链接名
		p->lnkName = info.fileName().remove(".lnk", Qt::CaseInsensitive);
		// 链接路径
		p->lnkPath = info.filePath();
		// 目录可执行程序路径
		p->targetPath = target;
		// 目标名字
		p->targetName = QFileInfo(p->targetPath).baseName();

		if (p->targetName.contains(p->lnkName, Qt::CaseInsensitive)) {
			CFileVersionInfo verinfo;
			if (verinfo.Create(p->targetPath.toStdWString().c_str())) {
				std::wstring desc = verinfo.GetFileDescription();
				if (!desc.empty()) {
					p->lnkName = QString::fromStdWString(desc);
				}
			}
		}

		// 过滤重复
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

void LnkModel::filter(const QString &text)
{
	const int MAX_RESULT = 50;
	pfilterdata_.clear();
	if (!text.isEmpty()) {
		for (auto iter = pdata_.begin(); iter != pdata_.end(); ++iter) {
			if (pfilterdata_.size() >= MAX_RESULT) {
				break;
			}

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
		qSort(pfilterdata_.begin(), pfilterdata_.end(), 
			[](const QSharedDataPointer<LnkData> &left, const QSharedDataPointer<LnkData> &right) -> bool {
			int leftHits = Acc::instance()->getHitsModel()->hits(left->lnkName, left->targetPath);
			int rightHits = Acc::instance()->getHitsModel()->hits(right->lnkName, right->targetPath);
			return leftHits > rightHits;
		});
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