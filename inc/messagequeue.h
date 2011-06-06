#ifndef _MESSAGEQUEUE_
#define _MESSAGEQUEUE_

#include <boost/thread.hpp>
#include "typedefs.h"
#include <queue>
#include "singelton.h"

//
// Non-directed messages
template<class T>
class Channel : public Singleton<Channel<T> > {
public:
	static void setup() {
		mInstance = new Channel();
	}
	void push(T data) {
		lock mlock(mMutex);
		mQueue.push(data);
		mlock.unlock();
		mEmptyCondition.notify_one();
	}
	T pop() {
		lock mlock(mMutex);
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
	uint32 value;
};

struct DataMsg {
	void* value;
	uint32 len;
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
struct MessageS;
typedef std::shared_ptr<MessageS> Message;

struct MessageS {
	void done() {
		if(async)
			waitCondition.notify_all();
	}
	~MessageS() {
		done();
	}
	MessageClient* sender;
	MessageClient* receiver;
	Message next; //next is the reply to this message
	Message previous; //this message is the reply to previous
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
// Directed message channel
template<class container_type, class id_type>
class DirectedProducer {
public:
	void produce(container_type data) {
		DirectedChannel<container_type, id_type>::getInstance()->push(data);
	}
};

template<class container_type, class id_type>
class DirectedConsumer {
public:
	container_type consume(id_type id) {
		return DirectedChannel<container_type, id_type>::getInstance()->pop(id);
	}
};

template<class container_type, class id_type>
id_type getID(container_type container) { throw std::exception("Need to create a specifier"); }

template<class container_type, class id_type>
class DirectedChannel : public Singleton<DirectedChannel<container_type, id_type> > {
public:
	static void setup() {
		mInstance = new DirectedChannel();
		mInstance->mQuit = false;
	}
	static void shutdown() {
		mInstance->close();
	}
	void push(container_type data) {
		lock mlock(mMutex);
		if(mQuit) {
			throw std::exception("Directed channel is shutting down");
			return;
		}
		mList.push_back(data);
		mlock.unlock();
		mEmptyCondition.notify_all();
	}
	container_type pop(id_type id) {
		lock mlock(mMutex);
		if(mQuit) {
			return 0;
		}
		else {
			bool found = false;
			Message ret;
			while(!found) {
				while(mList.empty() && !mQuit)
					mEmptyCondition.wait(mlock);
				if(mQuit)
					return 0;
				for(std::list<Message>::iterator it = mList.begin(); it != mList.end(); ++it) {
					if(getID<container_type, id_type>(*it) == id) {
						ret = *it;
						mList.erase(it);
						found = true;
						break;
					}
				}
			}
			return ret;
		}
		mlock.unlock();
	}
private:
	void close() {
		lock mlock(mMutex);
		mQuit = true;
		mEmptyCondition.notify_all();
	}
	bool mQuit;
	boost::condition_variable mEmptyCondition;
	typedef boost::unique_lock<boost::mutex> lock;
	boost::mutex mMutex;
	std::list<container_type> mList;
};

#endif //_MESSAGEQUEUE_