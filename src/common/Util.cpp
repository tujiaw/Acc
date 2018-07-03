#include "Util.h"
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include "hanzi2pinyin.h"
#include <QWidget>
#include <QApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#pragma warning(disable:4091)
#include <ShlObj.h>
#pragma comment(lib, "Shell32.lib")

/*
* DWORD EnumerateFileInDirectory(LPSTR szPath)
* ���ܣ�����Ŀ¼�µ��ļ�����Ŀ¼������ʾ�ļ����ļ������ء����ܵ�����
*
* ������LPSTR szPath��Ϊ�������·��
*
* ����ֵ��0����ִ����ɣ�1�����ʹ���
*/

void EnumerateFileInDirectory(const QString &dir, bool containsSubDir, QStringList &result)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile;
	WCHAR szFilePath[MAX_PATH];

	// ���������Ŀ¼���ļ���·�����ַ�����ʹ��ͨ���"*"
	lstrcpy(szFilePath, dir.toStdWString().c_str());
	// ע�͵Ĵ���������ڲ��������ԡ�.txt����β���ļ�
	// lstrcat(szFilePath, "\\*.txt");
	lstrcat(szFilePath, L"\\*");

	// ���ҵ�һ���ļ�/Ŀ¼����ò��Ҿ��
	hListFile = FindFirstFile(szFilePath, &FindFileData);
	// �жϾ��
	if (hListFile == INVALID_HANDLE_VALUE) {
		qDebug() << "error:" << GetLastError();
	} else {
		do {
			if(lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 || lstrcmp(FindFileData.cFileName, TEXT("..")) == 0) {
				continue;
			}
			
			// �ж��ļ����ԣ��Ƿ�Ϊ�����ļ������ļ���
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
				continue;
			}
			// �ж��ļ����ԣ��Ƿ�Ϊ�����ļ����ļ���
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
				continue;
			}

			QString path = dir + "\\" + QString::fromStdWString(FindFileData.cFileName);
			// �ж��ļ����ԣ��Ƿ�ΪĿ¼
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (containsSubDir) {
					EnumerateFileInDirectory(path, containsSubDir, result);
				}
				continue;
			}
			// ���߿ɸ����ļ����Ա��е�����������ӡ��ж��ļ�����
			result.push_back(path);
		} while (FindNextFile(hListFile, &FindFileData));
	}
}

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
		static const QStringList s_dyz1 = QStringList()
			<< QStringLiteral("�� �� ��")
			<< QStringLiteral("�� ��")
			<< QStringLiteral("Ϻ ��");
		static const QStringList s_dyz2 = QStringList()
			<< "ai qi yi"
			<< "yin yue"
			<< "xia mi";
		Q_ASSERT(s_dyz1.size() == s_dyz2.size());

		QStringList strList = text.split("", QString::SkipEmptyParts);
		QString spaceText = strList.join(" ");
		for (int i = 0; i < s_dyz1.size(); i++) {
			if (spaceText.contains(s_dyz1[i])) {
				spaceText.replace(s_dyz1[i], s_dyz2[i]);
			}
		}

		strList = spaceText.split(" ", QString::SkipEmptyParts);
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

	bool shellExecute(const QString &path)
	{
        return shellExecute(path, "open");
	}

    bool shellExecute(const QString &path, const QString &operation)
    {
        if (path.isEmpty()) {
            return false;
        }

        HINSTANCE hinst = ShellExecute(NULL, operation.toStdWString().c_str(), path.toStdWString().c_str(), NULL, NULL, SW_SHOWNORMAL);
        LONG64 result = (LONG64)hinst;
        if (result <= 32) {
            qDebug() << "shellExecute failed, code:" << result;
            return false;
        }
        return true;
    }

	bool locateFile(const QString &dir)
	{
        if (dir.isEmpty()) {
            return false;
        }

		QString newDir = dir;
		QString cmd = QString("/select, \"%1\"").arg(newDir.replace("/", "\\"));
		qDebug() << cmd;

		HINSTANCE hinst = ::ShellExecute(NULL, L"open", L"explorer.exe", cmd.toStdWString().c_str(), NULL, SW_SHOW);
		LONG64 result = (LONG64)hinst;
		if (result <= 32) {
			qDebug() << "locateFile failed, code:" << result;
			return false;
		}
		return true;
	}

	void setForegroundWindow(QWidget *widget)
	{
		if (widget) {
			HWND hwnd = (HWND)widget->winId();
			::SetForegroundWindow(hwnd);
		}
	}

	void showWndTopMost(QWidget *widget)
	{
		if (widget) {
			HWND hwnd = (HWND)widget->winId();
			RECT rect;
			GetWindowRect(hwnd, &rect);
			SetWindowPos(hwnd, HWND_TOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);
		}
	}

	void cancelTopMost(QWidget *widget)
	{
		if (widget) {
			HWND hwnd = (HWND)widget->winId();
			RECT rect;
			GetWindowRect(hwnd, &rect);
			SetWindowPos(hwnd, HWND_NOTOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);
		}
	}

	QPixmap img(const QString &name)
	{
		QString path = ":/images/" + name;
		return QPixmap(path);
	}

	QString getRunDir()
	{
		return QApplication::applicationDirPath();
	}

	QString getConfigDir()
	{
		QDir configDir(getRunDir() + "/config");
		if (!configDir.exists()) {
			configDir.mkdir("config");
		}
		return configDir.absolutePath();
	}

	QString getConfigPath()
	{
		return getConfigDir() + "/base.ini";
	}

	QString getSystemDir(int csidl)
	{
		//CSIDL_COMMON_PROGRAMS CSIDL_PROGRAMS CSIDL_DESKTOP
		int csidls[] = { CSIDL_ADMINTOOLS, CSIDL_ALTSTARTUP, CSIDL_APPDATA, CSIDL_BITBUCKET, CSIDL_CDBURN_AREA,
			CSIDL_COMMON_ADMINTOOLS, CSIDL_COMMON_ALTSTARTUP, CSIDL_COMMON_APPDATA, CSIDL_COMMON_DESKTOPDIRECTORY,
			CSIDL_COMMON_DOCUMENTS, CSIDL_COMMON_FAVORITES, CSIDL_COMMON_MUSIC, CSIDL_COMMON_OEM_LINKS,
			CSIDL_COMMON_PICTURES, CSIDL_COMMON_PROGRAMS, CSIDL_COMMON_STARTMENU, CSIDL_COMMON_STARTUP,
			CSIDL_COMMON_TEMPLATES, CSIDL_COMMON_VIDEO, CSIDL_COMPUTERSNEARME, CSIDL_CONNECTIONS, CSIDL_CONTROLS,
			CSIDL_COOKIES, CSIDL_DESKTOP, CSIDL_DESKTOPDIRECTORY, CSIDL_DRIVES, CSIDL_FAVORITES, CSIDL_FONTS,
			CSIDL_HISTORY, CSIDL_INTERNET, CSIDL_INTERNET_CACHE, CSIDL_LOCAL_APPDATA, CSIDL_MYDOCUMENTS,
			CSIDL_MYMUSIC, CSIDL_MYPICTURES, CSIDL_MYVIDEO, CSIDL_NETHOOD, CSIDL_NETWORK, CSIDL_PERSONAL,
			CSIDL_PRINTERS, CSIDL_PRINTHOOD, CSIDL_PROFILE, CSIDL_PROGRAM_FILES, CSIDL_PROGRAM_FILESX86,
			CSIDL_PROGRAM_FILES_COMMON, CSIDL_PROGRAM_FILES_COMMONX86, CSIDL_PROGRAMS, CSIDL_RECENT, CSIDL_RESOURCES,
			CSIDL_RESOURCES_LOCALIZED, CSIDL_SENDTO, CSIDL_STARTMENU, CSIDL_STARTUP, CSIDL_SYSTEM, CSIDL_SYSTEMX86,
			CSIDL_TEMPLATES, CSIDL_WINDOWS, CSIDL_FLAG_DONT_UNEXPAND, CSIDL_FLAG_DONT_VERIFY, CSIDL_FLAG_NO_ALIAS,
			CSIDL_FLAG_PER_USER_INIT, CSIDL_FLAG_MASK
		};

		LPITEMIDLIST pidl;
		LPMALLOC pShellMalloc;
		wchar_t szDir[MAX_PATH] = { 0 };
		if (SUCCEEDED(SHGetMalloc(&pShellMalloc))) {
			if (SUCCEEDED(SHGetSpecialFolderLocation(NULL, csidl, &pidl))) {
				SHGetPathFromIDList(pidl, szDir);
				pShellMalloc->Free(pidl);
			}
			pShellMalloc->Release();
		}
		return QString::fromStdWString(szDir);
	}

	QStringList getAllLnk()
	{
		QStringList result;
		QString desktop = getSystemDir(CSIDL_DESKTOP);
		QString commonPrograms = getSystemDir(CSIDL_COMMON_PROGRAMS);
		QString programs = getSystemDir(CSIDL_PROGRAMS);
        QString commonDesktop = getSystemDir(CSIDL_COMMON_DESKTOPDIRECTORY);

		result.append(getFiles(desktop, false));
		result.append(getFiles(commonPrograms));
		result.append(getFiles(programs));
        result.append(getFiles(commonDesktop));

		//EnumerateFileInDirectory(desktop, false, result);
		//EnumerateFileInDirectory(commonPrograms, true, result);
		//EnumerateFileInDirectory(programs, true, result);

		return result;
	}
	QVariantMap json2map(const QByteArray &val)
	{
		QJsonParseError jError;
		QJsonDocument jDoc = QJsonDocument::fromJson(val, &jError);
		if (jError.error == QJsonParseError::NoError) {
			if (jDoc.isObject()) {
				QJsonObject jObj = jDoc.object();
				return jObj.toVariantMap();
			}
		}
		QVariantMap ret;
		return ret;
	}

	QString map2json(const QVariantMap &val)
	{
		QJsonObject jobj = QJsonObject::fromVariantMap(val);
		QJsonDocument jdoc(jobj);
		return QString(jdoc.toJson(QJsonDocument::Indented));
	}

	QVariantList json2list(const QByteArray &val)
	{
		QJsonParseError jError;
		QJsonDocument jDoc = QJsonDocument::fromJson(val, &jError);
		if (jError.error == QJsonParseError::NoError) {
			if (jDoc.isArray()) {
				QJsonArray jArr = jDoc.array();
				return jArr.toVariantList();
			}
		}
		QVariantList ret;
		return ret;
	}

	QString list2json(const QVariantList &val)
	{
		QJsonArray jArr = QJsonArray::fromVariantList(val);
		QJsonDocument jDoc(jArr);
		return QString(jDoc.toJson(QJsonDocument::Indented));
	}

    uint toKey(const QString & str) 
    {
        QKeySequence seq(str);
        uint keyCode;

        // We should only working with a single key here
        if (seq.count() == 1) {
            keyCode = seq[0];
        } else {
            // Should be here only if a modifier key (e.g. Ctrl, Alt) is pressed.

            // Add a non-modifier key "A" to the picture because QKeySequence
            // seems to need that to acknowledge the modifier. We know that A has
            // a keyCode of 65 (or 0x41 in hex)
            seq = QKeySequence(str + "+A");
            keyCode = seq[0] - 65;
        }

        return keyCode;
    }
}
