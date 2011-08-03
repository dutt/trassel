#ifndef _MESSAGECLIENT_
#define _MESSAGECLIENT_

#include "messagequeue.h"
#include "boost/date_time/posix_time/posix_time_types.hpp"

namespace trassel {
	class MessageClient : public DirectedConsumer<Message, uint8>, public DirectedProducer<Message, uint8> {
		static uint8 lastID;
		uint8 mID;
		boost::posix_time::time_duration mSendTimeout;

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

	/*class ConcreteDirectedProducer : public DirectedProducer<Message, uint8> {
	public:
		ConcreteDirectedProducer(Channel<Message, uint8>* channel) : DirectedProducer(channel) {}
	};

	class ConcreteDirectedConsumer : public DirectedConsumer<Message, uint8> {
	};
*/

	class Task : public MessageClient {
	public:
		Task(Channel<Message, uint8>* channel) : MessageClient(channel) {}

		void operator()();
	protected:
		virtual void handleMessage(Message msg) =0;
		virtual void quit() { }
	};
}

#endif //_MESSAGECLIENT_