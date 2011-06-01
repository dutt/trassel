#ifndef _MESSAGEQUEUE_
#define _MESSAGEQUEUE_

#include <boost/thread.hpp>
#include "typedefs.h"
#include <queue>
#include "singelton.h"

//
// Non-directed

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

//
// Messages

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
	MessageClient* receiver;
	Message* next; //next is the reply to this message
	Message* previous; //this message is the reply to previous
	MsgType::MsgTypeEnum type;
	union {
		BoolMsg boolMsg;
		StringMsg stringMsg;
		IntMsg intMsg;
		DataMsg dataMsg;
	};
	//async
	bool async;
	boost::condition_variable waitCondition;
	boost::mutex mMutex;
};

//
// Directed

class DirectedProducer {
public:
	void produce(Message* data);
};

class DirectedConsumer {
public:
	Message* consume(uint8 id);
};

class MessageClient : public DirectedConsumer, public DirectedProducer {
	static uint8 lastID;
	uint8 mID;
public:
	MessageClient() : mID(lastID++) {}

	uint8 getID() { return mID; }

	Message* receiveMessage();
	Message* sendMessage(BoolMsg& data,MessageClient* receiver, bool async = true, bool waitForReply = false);
	Message* sendReply(Message* previous, BoolMsg& data, bool async = true, bool waitForReply = false);
	void sendMessage(StringMsg& data, MessageClient* receiver, bool async = true);
	void sendMessage(DataMsg& data, MessageClient* receiver, bool async = true);
	
};

class DirectedChannel : public Singleton<DirectedChannel> {
public:
	static void setup() {
		mInstance = new DirectedChannel();
	}
	void push(Message* data) {
		boost::unique_lock<boost::mutex> mlock(mMutex);
		mList.push_back(data);
		mlock.unlock();
		mEmptyCondition.notify_one();
	}
	Message* pop(uint8 id) {
		boost::unique_lock<boost::mutex> mlock(mMutex);
		bool found = false;
		Message* ret;
		while(!found) {
			while(mList.empty())
				mEmptyCondition.wait(mlock);
			for(std::list<Message*>::iterator it = mList.begin(); it != mList.end(); ++it) {
				if((*it)->receiver->getID() == id) {
					ret = *it;
					mList.erase(it);
					found = true;
					break;
				}
			}
		}
		return ret;
		mlock.unlock();
	}
private:
	boost::condition_variable mEmptyCondition;
	typedef boost::unique_lock<boost::mutex> lock;
	boost::mutex mMutex;
	std::list<Message*> mList;
};

#endif //_MESSAGEQUEUE_