#ifndef _MESSAGEQUEUE_
#define _MESSAGEQUEUE_

#include <boost/thread.hpp>
#include "typedefs.h"
#include <queue>
#include <tr1/memory>

#include <iostream>
using namespace std;

namespace trassel {

	template<class value_type, class id_type>
	class Channel {
	public:
		void virtual push(value_type) = 0;
		value_type virtual pop(id_type) = 0;
	};

	//
	// Non-directed messages
	template<class T>
	class NormalChannel : public Channel<T, void> {
	public:
		void push(T data) {
			lock mlock(mMutex);
			mQueue.push(data);
			mlock.unlock();
			mEmptyCondition.notify_one();
		}
		T pop(void) {
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
		Producer(Channel<T, void>* channel) : mChannel(channel) {}
		void produce(T data) {
			mChannel.push(data);
		}
		Channel<T, void>& getChannel() { return mChannel; }
	private:
		Channel<T, void>& mChannel;
	};

	template<class T>
	class Consumer {
	public:
		Consumer(Channel<T, void>* channel) : mChannel(channel) {}
		T consume() {
			return mChannel->pop();
		}
		Channel<T, void>& getChannel() { return mChannel; }
	private:
		Channel<T, void>& mChannel;
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
		void setValue(const char* text) {
			len = strlen(text)+1;
			value = new char[len];
			strcpy(value, text);
		}
	};

	struct IntMsg {
		uint32 value;
		static const uint32 len = sizeof(uint32);
	};

	struct DataMsg {
		void* value;
		uint32 len;
	};

	namespace MsgType {
        enum MsgTypeEnum {
			BoolMsgType,
			StringMsgType,
			IntMsgType,
			DataMsgType,
		};
	}

	class MessageClient;
	struct MessageS;
	typedef std::tr1::shared_ptr<MessageS> Message;

	struct MessageS {
		MessageS() : isDone(false), sender(0), receiver(0), next(), previous(), async(false) {}
		void done() {
			isDone = true;
			if(async)
				waitCondition.notify_all();
		}
		~MessageS() {
			done();
		}
		bool isDone;
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

	typedef boost::unique_lock<boost::mutex> lock;
    
	//
	// Directed message channel
	template<class container_type = Message, class id_type = uint8>
	class DirectedChannel : public Channel<container_type, id_type> {
	public:
		DirectedChannel() {
			mQuit = false;
		}
		~DirectedChannel() {
			close();
		}
		void close() {
			//cout <<"closing" <<endl;
			if(mQuit)
				return;
			lock mlock(mMutex);
			mQuit = true;
			mEmptyCondition.notify_all();
		}
		void push(container_type data) {
			lock mlock(mMutex);
			if(mQuit) {
				throw std::runtime_error("Directed channel is shutting down");
				return;
			}
			mList.push_back(data);
			//cout <<mList.size() <<" items in queue" <<endl;
			mlock.unlock();
			mEmptyCondition.notify_all();
		}
		
		id_type virtual getID(container_type) = 0;

		container_type pop(id_type id) {
			lock mlock(mMutex);
			if(mQuit) {
				//cout <<"mQuit is true" <<endl;
				return Message();
			}
			else {
				//cout <<"mQuit false" <<endl;
				bool found = false;
				Message ret;
				while(!found) {
					while(mList.empty() && !mQuit)
						mEmptyCondition.wait(mlock);
					if(mQuit)
					{
						//cout <<"mQuit2 is true" <<endl;
						return Message();
					}
					for(std::list<Message>::iterator it = mList.begin(); it != mList.end(); ++it) {
						if(getID(*it) == id) {
							ret = *it;
							mList.erase(it);
							found = true;
							break;
						}
					}
					mlock.unlock();
					mlock.lock();
				}
				return ret;
			}
		}
	private:
		bool mQuit;
		boost::condition_variable mEmptyCondition;
		boost::mutex mMutex;
		std::list<container_type> mList;
	};

	template<class container_type = Message, class id_type = uint8>
	class DirectedProducer {
	public:
		DirectedProducer(Channel<container_type, id_type>* channel) : mChannel(channel) {}
		void produce(container_type data) {
			mChannel->push(data);
		}
		Channel<container_type, id_type>& getChannel() { return mChannel; }
	private:
		Channel<container_type, id_type>* mChannel;
	};

	template<class container_type = Message, class id_type = uint8>
	class DirectedConsumer {
	public:
		DirectedConsumer(Channel<container_type, id_type>* channel) : mChannel(channel) {}
		container_type consume(id_type id) {
			return mChannel->pop(id);
		}
		Channel<container_type, id_type>& getChannel() { return mChannel; }
	private:
		Channel<container_type, id_type>* mChannel;
	};
}

#endif //_MESSAGEQUEUE_