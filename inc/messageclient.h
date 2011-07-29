#ifndef _MESSAGECLIENT_
#define _MESSAGECLIENT_

#include "messagequeue.h"

namespace trassel {
	class MessageClient : public DirectedConsumer<Message, uint8>, public DirectedProducer<Message, uint8> {
		static uint8 lastID;
		uint8 mID;
		boost::xtime mSendTimeout;

		Message createMessage(MessageClient* receiver, MsgType::MsgTypeEnum type);
		Message waitAsync(Message msg, bool waitForReply);
	public:
		MessageClient(Channel<Message, uint8>* channel, uint32 send_timeout = 0);

		uint8 getID() { return mID; }

		Message receiveMessage();
		Message sendMessage(BoolMsg& data,MessageClient* receiver, bool async = true, bool waitForReply = false);
		Message sendReply(Message previous, BoolMsg& data, bool async = true, bool waitForReply = false);
		Message sendMessage(StringMsg& data, MessageClient* receiver, bool async = true, bool waitForReply = false);
		Message sendReply(Message previous, StringMsg& data, bool async = true, bool waitForReply = false);
		Message sendMessage(DataMsg& data, MessageClient* receiver, bool async = true, bool waitForReply = false);
		Message sendReply(Message previous, DataMsg& data, bool async = true, bool waitForReply = false);
	};

	class ConcreteDirectedChannel : public DirectedChannel<Message, uint8> {
	public:
		uint8 getID(Message msg) { return msg->receiver->getID(); }
	};

	class Task : public MessageClient {
	public:
		Task(Channel<Message, uint8>* channel) : MessageClient(channel) {}

		void operator()();
	protected:
		virtual void handleMessage(Message msg) PURE;
		virtual void quit() { }
	};

	//#define START_TASK(x) new boost::thread(x)

	namespace GroupMode {
		enum GroupMode_t {
			FirstComeFirstServe,
			Broadcast 
		};
	}

	//template< >
	//uint8 trassel::getID<Message, uint8>(Message msg) { return msg->receiver->getID(); }

	class Group : public Task, public ConcreteDirectedChannel {
	public:
		Group(const Group& other);
		Group(Channel<Message, uint8>* channel, GroupMode::GroupMode_t mode);

		void attach(MessageClient* client);
		void detach(MessageClient* client);
		bool isAttached(MessageClient* client);

		void handleMessage(Message msg);
		void quit();

		void push(Message msg);
		Message pop(uint8 id);

		GroupMode::GroupMode_t getMode() { return mMode; }
	private:
		Message popInternal(uint8 id, lock& waitLock);
		std::map<uint8, std::queue<Message> > mClients;
		typedef std::map<uint8, std::queue<Message> >::iterator ClientIt;
		GroupMode::GroupMode_t mMode;
		bool mQuit;
		boost::condition_variable mEmptyCondition;
		typedef boost::unique_lock<boost::mutex> lock;
		boost::mutex mMutex;
	};
}

#endif //_MESSAGECLIENT_