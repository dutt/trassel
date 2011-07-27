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
		MessageClient(uint32 send_timeout = 0);

		uint8 getID() { return mID; }

		Message receiveMessage();
		Message sendMessage(BoolMsg& data,MessageClient* receiver, bool async = true, bool waitForReply = false);
		Message sendReply(Message previous, BoolMsg& data, bool async = true, bool waitForReply = false);
		Message sendMessage(StringMsg& data, MessageClient* receiver, bool async = true, bool waitForReply = false);
		Message sendReply(Message previous, StringMsg& data, bool async = true, bool waitForReply = false);
		Message sendMessage(DataMsg& data, MessageClient* receiver, bool async = true, bool waitForReply = false);
		Message sendReply(Message previous, DataMsg& data, bool async = true, bool waitForReply = false);
	};

	class Task : public MessageClient {
	public:
		void operator()();
	protected:
		virtual void handleMessage(Message msg) PURE;
		virtual void quit() { }
	};

	#define START_TASK(x) new boost::thread(x)

	class Group : public MessageClient {
	public:
		void attach(MessageClient& client);
		void detach(MessageClient& client);
	};
}

#endif //_MESSAGECLIENT_