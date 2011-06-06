#ifndef _MESSAGECLIENT_
#define _MESSAGECLIENT_

#include "messagequeue.h"

class MessageClient : public DirectedConsumer<Message, uint8>, public DirectedProducer<Message, uint8> {
	static uint8 lastID;
	uint8 mID;

	Message createMessage(MessageClient* receiver, MsgType::MsgTypeEnum type);
	Message waitAsync(Message msg, bool waitForReply);
public:
	MessageClient() : mID(lastID++) {}

	uint8 getID() { return mID; }

	Message receiveMessage();
	Message sendMessage(BoolMsg& data,MessageClient* receiver, bool async = true, bool waitForReply = false);
	Message sendReply(Message previous, BoolMsg& data, bool async = true, bool waitForReply = false);
	Message sendMessage(StringMsg& data, MessageClient* receiver, bool async = true, bool waitForReply = false);
	Message sendMessage(DataMsg& data, MessageClient* receiver, bool async = true, bool waitForReply = false);
};

#endif //_MESSAGECLIENT_