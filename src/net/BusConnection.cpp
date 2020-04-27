#include "BusConnection.h"
#include "qpid/messaging/Connection.h"
#include "qpid/messaging/Address.h"
#include "qpid/types/Uuid.h"
#include <iostream>
#include <assert.h>

std::function<void(const char*)> BusTrace::write_ = nullptr;

time_t GetCurrentTimeSec()
{
    return time(NULL);
}

std::string NewMessageId()
{
    qpid::types::Uuid uuid(true);
    return uuid.str();
}

BusConnection::BusConnection(const std::string &url)
    : _connection(new Connection(url))
    , _is_open(false)
{
    _connection->setOption("reconnect", true);
    _connection->setOption("heartbeat", 5);    
    Open(url);
}

BusConnection::~BusConnection()
{
    Close();
}

void BusConnection::AddQueueServer(const std::string &serverAddr, const ServerCallback &cb)
{
    assert(!serverAddr.empty());
    assert(!!cb);
    std::string serverAddress = serverAddr + "; {create:always, node: {type:queue}}";
    std::shared_ptr<std::thread> t(new std::thread(std::bind(&BusConnection::QueueServerRunning, this, serverAddress, cb)));
    _handlerThreadList.push_back(t);
}

void BusConnection::AddTopicServer(const std::string &serverAddr, const SubscribeCallback &cb)
{
    assert(!serverAddr.empty());
    assert(!!cb);
    std::string serverAddress = serverAddr + "; {create:always, node: {type:topic}}";
    std::shared_ptr<std::thread> t(new std::thread(std::bind(&BusConnection::TopicServerRunning, this, serverAddress, cb)));
    _handlerThreadList.push_back(t);
}

bool BusConnection::Open(const std::string& url)
{
	Debug("BusConnection::Open url:%s", url.c_str());
    try {
        _connection->open();
        _session = _connection->createSession();
        _asyncReceiver = _session.createReceiver("#.BusConnection.AsyncReceiver");
        _syncReceiver = _session.createReceiver("#.BusConnection.SyncReceiver");
    } catch (const std::exception& error) {
		Error("BusConnection::Open error:%s", error.what());
    }

    _is_open = _connection->isOpen();
    if (_is_open) {
        StartThread();
    }
    return _is_open;
}

void BusConnection::Close()
{
    _is_open = false;

    if (_receiveThread && _receiveThread->joinable()) {
        _receiveThread->join();
    }

    for (const auto &t : _handlerThreadList) {
        if (t->joinable()) {
            t->join();
        }
    }

    try {
        for (auto iter = _senderCache.begin(); iter != _senderCache.end(); ++iter) {
            if (iter->second.isValid()) {
                iter->second.close();
            }
        }

        if (_asyncReceiver) {
            _asyncReceiver.close();
        }
        if (_syncReceiver) {
            _syncReceiver.close();
        }
        if (_session) {
            _session.close();
        }
    } catch (const std::exception& error) {
		Error("BusConnection::Close, error:%s", error.what());
    }

    try {
        _connection->close();
    } catch (const std::exception &error) {
		Error("BusConnection::Close, connecton close error:%s", error.what());
    }
}

void BusConnection::StartThread()
{
    if (!_receiveThread) {
        _receiveThread.reset(new std::thread(std::bind(&BusConnection::ReceiveRunning, this)));
    }

    std::thread timeoutThread(std::bind(&BusConnection::TimeoutRunning, this));
    timeoutThread.detach();
}

void BusConnection::ReceiveRunning()
{
    while (_is_open) {
        try {
            QMsgPtr p(new Message());
            while (_asyncReceiver.fetch(*p.get(), Duration::SECOND)) {
                RequestInfo info;
                {
                    std::unique_lock<std::mutex> lock(_requestCacheMutex);
                    auto iter = _requestCache.find(p->getMessageId());
                    if (iter != _requestCache.end()) {
                        info = iter->second;
                        _requestCache.erase(iter);
                    }
                }
                if (info.cb && info.msg) {
                    info.cb(info.msg, p);
                }
                _session.acknowledge();
            }
        } catch (const std::exception& error) {
			Error("BusConnection::ReceiveRunning error:%s", error.what());
            break;
        }
    }
}

void BusConnection::TimeoutRunning()
{
    while (_is_open) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        if (_requestCache.empty()) {
            continue;
        }

        std::vector<RequestInfo> timeoutList;
        {
            time_t currentTime = GetCurrentTimeSec();
            std::unique_lock<std::mutex> lock(_requestCacheMutex);
            auto iter = _requestCache.begin();
            while (iter != _requestCache.end()) {
                const RequestInfo &info = iter->second;
                if (currentTime - info.time >= info.second) {
                    timeoutList.push_back(info);
                    iter = _requestCache.erase(iter);
                } else {
                    ++iter; 
                }
            }
        }

        if (!timeoutList.empty()) {
            for (auto iter = timeoutList.begin(); iter != timeoutList.end(); ++iter) {
                iter->cb(iter->msg, QMsgPtr(new Message()));
            }
        }
    }
}

void BusConnection::QueueServerRunning(const std::string &addr, const ServerCallback &cb)
{
    while (_is_open) {
        Session session = _connection->createSession();
        Receiver receiver = session.createReceiver(addr);
        try {
            while (_is_open) {
                Message msg;
                if (receiver.fetch(msg, Duration::SECOND)) {
                    Message reply;
                    cb(msg, reply);
                    session.acknowledge();
                    const Address &address = msg.getReplyTo();
                    if (address) {
                        reply.setMessageId(msg.getMessageId());
                        Sender sender = session.createSender(address);
                        sender.send(reply);
                        sender.close();
                    }
                }
            }
            session.sync();
        }
        catch (const std::exception &error) {
			Error("BusConnection::QueueServerRunning error:%s", error.what());
        }

        try {
            // Ϊ�˽��Ӧ��sender���б����ٺ��쳣���receiver.fetch�쳣
            session.close();
        }
        catch (const std::exception &error) {
			Error("BusConnection::QueueServerRunning error:%s", error.what());
        }
    }
}

void BusConnection::TopicServerRunning(const std::string &addr, const SubscribeCallback &cb)
{
    Session session = _connection->createSession();
    Receiver receiver = session.createReceiver(addr);
    while (_is_open) {
        try {
            Message msg;
            while (receiver.fetch(msg, Duration::SECOND)) {
                cb(msg);
                session.acknowledge();
            }
        }
        catch (const std::exception &error) {
			Error("BusConnection::QueueServerRunning error:%s", error.what());
        }
    }
}

bool BusConnection::PostMsg(const std::string &name, const QMsgPtr &msg, int second, const ResponseCallback &cb)
{
    bool result = false;
    Sender &sender = GetSender(name, "queue");
    if (sender) {
        msg->setMessageId(NewMessageId());
        msg->setReplyTo(_asyncReceiver.getAddress());
        try {
            {
                std::unique_lock<std::mutex> lock(_requestCacheMutex);
                RequestInfo info(second, msg, cb);
                _requestCache[msg->getMessageId()] = info;
            }
            sender.send(*msg.get());
            result = true;
        }
        catch (const std::exception& error) {
			Error("BusConnection::PostMsg error:%s", error.what());
        }
    }
    return result;
}

bool BusConnection::SendMsg(const std::string &name, const Message &requestMsg, Message &responseMsg, int milliseconds)
{
    bool result = false;
    Sender &sender = GetSender(name, "queue");
    if (sender) {
        Message &msg = const_cast<Message&>(requestMsg);
        msg.setMessageId(NewMessageId());
        msg.setReplyTo(_syncReceiver.getAddress());
        try {
            sender.send(msg);
            result = _syncReceiver.fetch(responseMsg, Duration(milliseconds));
            _session.acknowledge(responseMsg);
        }
        catch (const std::exception &error) {
			Error("BusConnection::SendMsg error:%s", error.what());
        }
    }
    return result;
}

bool BusConnection::PublishMsg(const std::string &topic, const Message &msg)
{
    bool result = false;
    Sender &sender = GetSender(topic, "topic");
    if (sender) {
        try {
            sender.send(msg);
            result = true;
        }
        catch (const std::exception &error) {
			Error("BusConnection::SendMsg error:%s", error.what());
        }
    }
    return result;
}

Sender& BusConnection::GetSender(const std::string &name, const std::string &nodeType)
{
    std::unique_lock<std::mutex> lock(_senderMutex);
    Sender &sender = _senderCache[name];
    if (sender) {
        return sender;
    } else {
        try {
            std::string address = name;
            if (nodeType == "queue") {
                //address += "; {create:always, delete:always, node: {type:queue, x-declare:{auto-delete:true}}}"; // �ǳ־�
                address += "; {create:always, node: {type:queue}}"; // �־�
            } else if (nodeType == "topic") {
                address += "; {create: always , node:{type:topic , x-declare:{type:fanout}}}";
            } else {
                std::cerr << "AddSender error, node type error:" << nodeType;
                return _emptySender;
            }

            Sender sender = _session.createSender(address);
            assert(sender.isValid());
            _senderCache[name] = sender;
            return _senderCache[name];
        }
        catch (const std::exception &error) {
			Error("AddSender error:%s", error.what());
        }
    }

    return _emptySender;
}
