#pragma once

#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <functional>
#include <thread>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <queue>

#include "qpid/messaging/Session.h"
#include "qpid/messaging/Message.h"
#include "qpid/messaging/Sender.h"
#include "qpid/messaging/Receiver.h"

using namespace qpid::messaging;
using MessagePtr = std::shared_ptr<Message>;

time_t GetCurrentTimeSec();
std::string NewMessageId();


#define Debug BusTrace(__FILE__, __LINE__, "[Debug]")
#define Error BusTrace(__FILE__, __LINE__, "[Error]")
class BusTrace
{
public:
	BusTrace(const char *path, int line, const char *logLevel) : path_(path), line_(line), logLevel_(logLevel) {}
	static void setWrite(const std::function<void(const char*)> &cb) { write_ = cb; }
	void __cdecl operator()(const char *fmt, ...) const
	{
		va_list ptr;
		va_start(ptr, fmt);
		char buffer[4096] = { 0 };
		std::string filename(path_);
		int ipos = filename.find_last_of('\\');
		filename = filename.substr(ipos + 1, filename.size() - ipos + 1);
		int iret = sprintf_s(buffer, sizeof(buffer), "[%s, %d] %s", filename.c_str(), line_, logLevel_);
		if (-1 != iret)
		{
			vsnprintf_s(buffer + iret, sizeof(buffer) - iret - 1, _TRUNCATE, fmt, ptr);
			if (write_) {
				write_(buffer);
			} else {
				std::cout << buffer << std::endl;
			}
		}
		va_end(ptr);
	}

private:
	const char *path_;
	const int line_;
	const char *logLevel_;
	static std::function<void(const char*)> write_;
};

class BusConnection
{
public:
    using ResponseCallback = std::function<void(const MessagePtr &sendMsg, const MessagePtr &resMsg)>;
    using ServerCallback = std::function<void(const Message &msg, Message &reply)>;
    using SubscribeCallback = std::function<void(const Message &msg)>;

    struct RequestInfo {
        RequestInfo() : second(1) , time(GetCurrentTimeSec()) , msg(nullptr), cb(nullptr) {}
        RequestInfo(int s, const MessagePtr &m, const ResponseCallback &c) : second(s), time(GetCurrentTimeSec()), msg(m), cb(c) {}
        int second; time_t time; MessagePtr msg; ResponseCallback cb;
    };

    BusConnection(const std::string &url);
    BusConnection(const BusConnection &) = delete;
    void operator=(const BusConnection &) = delete;
    ~BusConnection();

	bool isOpen() const { return _is_open; }
    void AddQueueServer(const std::string &serverAddr, const ServerCallback &cb);
    void AddTopicServer(const std::string &serverAddr, const SubscribeCallback &cb);
    bool PostMsg(const std::string &name, const MessagePtr &msg, int second, const ResponseCallback &cb);
    bool SendMsg(const std::string &name, const Message &requestMsg, Message &responseMsg, int milliseconds = 1000);
    bool PublishMsg(const std::string &topic, const Message &msg);
	void Close();

private:
    bool Open(const std::string& url);
    void StartThread();
    void ReceiveRunning();
    void TimeoutRunning();
    void QueueServerRunning(const std::string &addr, const ServerCallback &cb);
    void TopicServerRunning(const std::string &addr, const SubscribeCallback &cb);

    Sender& GetSender(const std::string &name, const std::string &nodeType);

private:
    std::unique_ptr<qpid::messaging::Connection> _connection;
    bool _is_open;
    Session _session;
    Receiver _asyncReceiver;
    Receiver _syncReceiver;

    std::unique_ptr<std::thread> _receiveThread;
    std::vector<std::shared_ptr<std::thread>> _handlerThreadList;

    std::mutex _requestCacheMutex;
    std::unordered_map<std::string, RequestInfo> _requestCache;

    mutable std::mutex _senderMutex;
    std::unordered_map<std::string, Sender> _senderCache;
    Sender _emptySender;
};

