#include "Util.h"
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include "hanzi2pinyin.h"

namespace Util
{

	QStringList getFiles(QString path, bool containsSubDir)
	{
		QStringList result;
		QDirIterator curit(path, QStringList(), QDir::Files);
		while (curit.hasNext()) {
			result.push_back(curit.next());
		}

		if (containsSubDir) {
			QDirIterator subit(path, QStringList(), QDir::Files, QDirIterator::Subdirectories);
			while (subit.hasNext()) {
				result.push_back(subit.next());
			}
		}

		return result;
	}

	QString getPinyin(const QString &text)
	{
		std::wstring result = L"";
		std::wstring wstr = text.toStdWString();
		for (auto iter = wstr.begin(); iter != wstr.end(); ++iter) {
			result += retrievePinyin(*iter);
		}
		return QString::fromStdWString(result);
	}

	QPair<QString, QString> getPinyinAndJianpin(const QString &text)
	{
		static const QStringList s_dyz1 = QStringList() << QStringLiteral("°®ÆæÒÕ");
		static const QStringList s_dyz2 = QStringList() << QStringLiteral("Ææ");
		static const QStringList s_dyz3 = QStringList() << "qi";

		int index = -1;
		for (int i = 0; i < s_dyz1.size(); i++) {
			if (text.contains(s_dyz1[i])) {
				index = i;
				break;
			}
		}

		QStringList strList;
		if (index < 0) {
			strList = text.split("", QString::SkipEmptyParts);
		} else {
			for (auto iter = text.begin(); iter != text.end(); ++iter) {
				if (*iter == s_dyz2[index]) {
					strList.append(s_dyz3[index]);
				} else {
					strList.append(*iter);
				}
			}
		}

		QString pinyin, jianpin;
		for (auto iter = strList.begin(); iter != strList.end(); ++iter) {
			QString tmp = getPinyin(*iter);
			if (tmp.size() > 0) {
				pinyin += tmp;
				jianpin += tmp[0];
			}
		}
		return qMakePair(pinyin, jianpin);
	}
}