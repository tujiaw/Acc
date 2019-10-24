#include "Util.h"
#include <QStringList>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QWidget>
#include <QApplication>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCryptographicHash>
#include <QStandardPaths>

#pragma warning(disable:4091)
#include <ShlObj.h>
#pragma comment(lib, "Shell32.lib")

/*
* DWORD EnumerateFileInDirectory(LPSTR szPath)
* 功能：遍历目录下的文件和子目录，将显示文件和文件夹隐藏、加密的属性
*
* 参数：LPSTR szPath，为需遍历的路径
*
* 返回值：0代表执行完成，1代表发送错误
*/

void EnumerateFileInDirectory(const QString &dir, bool containsSubDir, QStringList &result)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hListFile;
	WCHAR szFilePath[MAX_PATH];

	// 构造代表子目录和文件夹路径的字符串，使用通配符"*"
	lstrcpy(szFilePath, dir.toStdWString().c_str());
	// 注释的代码可以用于查找所有以“.txt”结尾的文件
	// lstrcat(szFilePath, "\\*.txt");
	lstrcat(szFilePath, L"\\*");

	// 查找第一个文件/目录，获得查找句柄
	hListFile = FindFirstFile(szFilePath, &FindFileData);
	// 判断句柄
	if (hListFile == INVALID_HANDLE_VALUE) {
		qDebug() << "error:" << GetLastError();
	} else {
		do {
			if(lstrcmp(FindFileData.cFileName, TEXT(".")) == 0 || lstrcmp(FindFileData.cFileName, TEXT("..")) == 0) {
				continue;
			}
			
			// 判断文件属性，是否为加密文件或者文件夹
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
				continue;
			}
			// 判断文件属性，是否为隐藏文件或文件夹
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
				continue;
			}

			QString path = dir + "\\" + QString::fromStdWString(FindFileData.cFileName);
			// 判断文件属性，是否为目录
			if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				if (containsSubDir) {
					EnumerateFileInDirectory(path, containsSubDir, result);
				}
				continue;
			}
			// 读者可根据文件属性表中的内容自行添加、判断文件属性
			result.push_back(path);
		} while (FindNextFile(hListFile, &FindFileData));
	}
}

namespace Util
{

    QStringList getFiles(QString path, bool containsSubDir, int maxLimit, const QStringList &filterSuffix)
	{
        QStringList result;
        if (path.isEmpty()) {
            return result;
        }
        QDirIterator iter(path, QStringList(), QDir::Files, containsSubDir ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
        while (iter.hasNext()) {
            if (filterSuffix.isEmpty()) {
                result.push_back(iter.next());
            } else {
                QString suffix = iter.fileInfo().suffix();
                if (suffix.isEmpty()) {
                    result.push_back(iter.next());
                } else if (!filterSuffix.contains(suffix, Qt::CaseInsensitive)) {
                    result.push_back(iter.next());
                } else {
                    iter.next();
                }
            }

            if (maxLimit > 0 && result.size() >= maxLimit) {
                return result;
            }
        }
        return result;
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

    QString getWritebaleDir()
    {
        QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        if (!dir.exists()) {
            dir.mkpath(dir.absolutePath());
        }
        if (!dir.exists()) {
            dir = QDir(getRunDir());
        }
        return dir.absolutePath();
    }

	QString getConfigDir()
	{
        QDir dir(getWritebaleDir());
        if (!dir.exists("config")) {
            dir.mkdir("config");
		}
        return dir.absolutePath() + "/config";
	}

	QString getConfigPath()
	{
		return getConfigDir() + "/base.ini";
	}

    QString getIndexDir(const QString &name)
    {
        const QString indexDir = "indexfiles";
        QDir dir(getWritebaleDir());
        if (!dir.exists(indexDir)) {
            dir.mkdir(indexDir);
        }
        dir.cd(indexDir);
        if (name.isEmpty()) {
            return dir.absolutePath();
        } else {
            if (!dir.exists(name)) {
                dir.mkdir(name);
            }
            return dir.absolutePath() + "/" + name;
        }
    }

    bool removeDir(const QString &path, bool containSubDir)
    {
        QDir dir(path);
        QFileInfoList fileList = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
        for (int i = 0; i < fileList.size(); i++) {
            if (fileList[i].isDir() && containSubDir) {
                if (!removeDir(fileList[i].absoluteFilePath(), containSubDir)) {
                    return false;
                }
            } else {
                dir.remove(fileList[i].fileName());
            }
        }
        QString dirName = dir.dirName();
        if (dir.cdUp()) {
            return dir.rmdir(dirName);
        }
        return false;
    }

    QString getImagesDir()
    {
        QDir dir(getWritebaleDir());
        if (!dir.exists("images")) {
            dir.mkdir("images");
        }
        return dir.absolutePath() + "/images";
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
        } else {
            qDebug() << jError.errorString();
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

    std::string gbk2utf8(const std::string &gbkStr)
    {
        std::string outUtf8 = "";
        int n = MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, NULL, 0);
        if (n > 0) {
            WCHAR *str1 = new WCHAR[n];
            MultiByteToWideChar(CP_ACP, 0, gbkStr.c_str(), -1, str1, n);
            n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
            if (n > 0) {
                char *str2 = new char[n];
                WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
                outUtf8 = str2;
                delete[] str2;
            }
            delete[] str1;
        }
        return outUtf8;
    }


    std::string utf82gbk(const std::string &utf8Str)
    {
        std::string outGBK = "";
        int n = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, NULL, 0);
        if (n > 0) {
            WCHAR *str1 = new WCHAR[n];
            MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), -1, str1, n);
            n = WideCharToMultiByte(CP_ACP, 0, str1, -1, NULL, 0, NULL, NULL);
            if (n > 0) {
                char *str2 = new char[n];
                WideCharToMultiByte(CP_ACP, 0, str1, -1, str2, n, NULL, NULL);
                outGBK = str2;
                delete[] str2;
            }
            delete[] str1;
        }
        return outGBK;
    }

    void setWallpaper(const QString &imagePath)
    {
        qDebug() << "setWallpaper:" << imagePath;
        int start = imagePath.lastIndexOf(".");
        std::string format = imagePath.mid(start + 1).toUpper().toStdString();
        if (format.empty()) {
            return;
        }

        QString newPath = imagePath.mid(0, start + 1) + "bmp";
        QPixmap pixmap(imagePath, format.c_str());
        pixmap.save(newPath, "BMP");

        setWallpaperBMP(newPath);
    }

    void setWallpaperBMP(const QString &imagePath)
    {
        std::wstring wstr = imagePath.toStdWString();
        const wchar_t *p = wstr.c_str();
        if (!SystemParametersInfo(SPI_SETDESKWALLPAPER, 0, (void*)p, SPIF_UPDATEINIFILE + SPIF_SENDWININICHANGE)) {
            qDebug() << "set wallpaper failed, " << imagePath;
        }
    }

    QString md5(const QString &str)
    {
        QByteArray byteArray = QCryptographicHash::hash(str.toUtf8(), QCryptographicHash::Md5);
        return byteArray.toHex();
    }

    QString md5(const QByteArray &data)
    {
        QByteArray byteArray = QCryptographicHash::hash(data, QCryptographicHash::Md5);
        return byteArray.toHex();
    }

    QString getTimeInterval(quint64 msInterval)
    {
        int minutes = msInterval / (1000 * 60);
        // 服务端时间与客户端时间如果差距大可能为负的
        if (minutes <= 0) {
            return QStringLiteral("刚刚");
        } else if (minutes < 60) {
            return QString::number(minutes) + QStringLiteral("分钟前");
        }

        int hours = minutes / 60;
        if (hours < 24) {
            return QString::number(hours) + QStringLiteral("小时前");
        }

        int days = hours / 24;
        if (days < 30) {
            return QString::number(days) + QStringLiteral("天前");
        }

        int months = days / 30;
        if (months < 12) {
            return QString::number(months) + QStringLiteral("个月前");
        }

        int year = months / 12;
        return QString::number(year) + QStringLiteral("年前");
    }

    bool ImproveProcPriv()
    {
        HANDLE token;
        //提升权限
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token))
        {
            MessageBoxA(NULL, "Open process token error", "error", MB_ICONSTOP);
            return false;
        }
        TOKEN_PRIVILEGES tkp;
        tkp.PrivilegeCount = 1;
        ::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (!AdjustTokenPrivileges(token, FALSE, &tkp, sizeof(tkp), NULL, NULL))
        {
            MessageBoxA(NULL, "Adjust token privileges error", "error", MB_ICONSTOP);
            return false;
        }
        CloseHandle(token);
        return true;
    }
}
