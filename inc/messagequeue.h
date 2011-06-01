#ifndef _MESSAGEQUEUE_
#define _MESSAGEQUEUE_

#include <boost/thread.hpp>
#include "typedefs.h"
#include <queue>
#include "singelton.h"

template<class T>
class Channel : public Singleton<Channel<T> > {
public:
	static void setup() {
		mInstance = new Channel();
	}
	void push(T data) {
		boost::unique_lock<boost::mutex> mlock(mMutex);
		mQueue.push(data);
		mlock.unlock();
		mEmptyCondition.notify_one();
	}
	T pop() {
		boost::unique_lock<boost::mutex> mlock(mMutex);
		if(mQueue.empty()) {
			mEmptyCondition.wait(mlock);
		}
		T ret = mQueue.front();
		mQueue.pop();
		mlock.unlock();
		return ret;
	}
private:
	boost::condition_variable mEmptyCondition;
	typedef boost::unique_lock<boost::mutex> lock;
	boost::mutex mMutex;
	std::queue<T> mQueue;
};

template<class T>
class Producer {
public:
	void produce(T data) {
		Channel<T>::getInstance()->push(data);
	}
};

template<class T>
class Consumer {
public:
	T consume() {
		return Channel<T>::getInstance()->pop();
	}
};

struct Message;

/*template <class T>
class ConsumeWorker {
public:
	static void exit() {
		boost::unique_lock<boost::mutex> exitLock(mExitMutex);
		mDoExit = true;
	}

	bool getExit() {
		bool retr;
		boost::unique_lock<boost::mutex> exitLock(mExitMutex);
		retr = mDoExit;
		exitLock.release();
		return retr;
	}
	void operator()() {
		while(getExit()) {
			Message msg = Channel::getInstance()->pop();
			msg.reciever->handleMessage(msg);
		}
	}
private:
	boost::mutex mExitMutex;
	bool mDoExit;
};*/

struct BoolMsg {
	bool value;
	static const uint32 len = sizeof(bool);
};

struct StringMsg {
	char* value;
	uint32 len;
};

struct IntMsg {
	int value;
};

struct DataMsg {
	void* data;
	int length;
};

namespace MsgType {
	typedef enum MsgTypeEnum {
		BoolMsgType,
		StringMsgType,
		IntMsgType,
		DataMsgType,
	};
}

class MessageClient;

struct Message {
	void done() {
		if(async)
			waitCondition.notify_all();
	}
	~Message() {
		done();
	}
	MessageClient* sender;
	MessageClient* reciever;
	MsgType::MsgTypeEnum type;
	bool async;
	boost::condition_variable waitCondition;
	boost::mutex mMutex;
	union {
		BoolMsg boolMsg;
		StringMsg stringMsg;
		IntMsg intMsg;
		DataMsg dataMsg;
	};
};

class MessageClient : public Consumer<Message*>, public Producer<Message*> {
	static uint8 lastID;
	uint8 mID;
public:
	MessageClient() : mID(lastID++) {}

	uint8 getID() { return mID; }

	Message* recieveMessage() {
		return consume();
	}
	void sendMessage(BoolMsg& data,MessageClient* reciever, bool async = true) {
		if(async) {
			Message* msg = new Message();
			msg->sender = this;
			msg->reciever = reciever;
			msg->type = MsgType::BoolMsgType;
			msg->boolMsg = data;
			produce(msg);
			boost::unique_lock<boost::mutex> lock(msg->mMutex);
			msg->waitCondition.wait(lock);
		}
		else {
			Message* msg = new Message();
			msg->sender = this;
			msg->reciever = reciever;
			msg->type = MsgType::BoolMsgType;
			msg->boolMsg = data;
			produce(msg);
		}
	}

	void sendMessage(StringMsg& data, MessageClient* reciever, bool async = true) {
		Message* msg = new Message();
		msg->sender = this;
		msg->reciever = reciever;
		msg->type = MsgType::StringMsgType;
		msg->stringMsg = data;
		produce(msg);
	}
	void sendMessage(DataMsg& data, MessageClient* reciever, bool async = true) {
		Message* msg = new Message();
		msg->sender = this;
		msg->reciever = reciever;
		msg->type = MsgType::DataMsgType;
		msg->dataMsg = data;
		produce(msg);
	}
};

#endif //_MESSAGEQUEUE_