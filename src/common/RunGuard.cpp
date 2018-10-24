#include "RunGuard.h"

#include <QCryptographicHash>


namespace
{

	QString generateKeyHash(const QString& key, const QString& salt)
	{
		QByteArray data;

		data.append(key.toUtf8());
		data.append(salt.toUtf8());
		data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();

		return data;
	}

}

class MemLockGuard {
public:
    MemLockGuard(QSystemSemaphore &lock) : lock_(lock) { lock_.acquire(); }
    ~MemLockGuard() { lock_.release(); }

private:
    QSystemSemaphore &lock_;
};

RunGuard::RunGuard(const QString& key)
	: key(key)
	, memLockKey(generateKeyHash(key, "_memLockKey"))
	, sharedmemKey(generateKeyHash(key, "_sharedmemKey"))
	, sharedMem(sharedmemKey)
	, memLock(memLockKey, 1)
{
    MemLockGuard lock(memLock);
	{
		QSharedMemory fix(sharedmemKey);
		fix.attach();
	}
}

RunGuard::~RunGuard()
{
	release();
}

bool RunGuard::isAnotherRunning()
{
    if (sharedMem.isAttached()) {
        return false;
    }

    MemLockGuard lock(memLock);
    const bool isRunning = sharedMem.attach();
    if (isRunning) {
        sharedMem.detach();
    }

	return isRunning;
}

bool RunGuard::tryToRun()
{
    // Extra check
    if (isAnotherRunning()) {
        return false;
    }

    bool result = false;
    {
        MemLockGuard lock(memLock);
        result = sharedMem.create(sizeof(quint64));
    }
	
	if (!result) {
		release();
	}
    return result;
}

void RunGuard::release()
{
    MemLockGuard lock(memLock);
    if (sharedMem.isAttached()) {
        sharedMem.detach();
    }
}