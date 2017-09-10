#ifndef FOREGROUNDWINDOWGUARD_H_
#define FOREGROUNDWINDOWGUARD_H_

class ForegroundWindowGuard
{
public:
	ForegroundWindowGuard();
	~ForegroundWindowGuard();

private:
	unsigned long  m_lockTimeOut;
};

#endif // FOREGROUNDWINDOWGUARD_H_
