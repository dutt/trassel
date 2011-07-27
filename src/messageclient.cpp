#include "messageclient.h"
using namespace trassel;

uint8 MessageClient::lastID = 0;

template< >
uint8 trassel::getID<Message, uint8>(Message msg) { return msg->receiver->getID(); }

MessageClient::MessageClient(uint32 send_timeout): mID(lastID++) {
	mSendTimeout.sec = send_timeout;
}

Message MessageClient::receiveMessage() {
	return consume(mID);
}

Message MessageClient::createMessage(MessageClient* receiver, MsgType::MsgTypeEnum type) {
	MessageS* msg = new MessageS();
	msg->sender = this;
	msg->receiver = receiver;
	msg->type = type;
	return Message(msg);
}

Message MessageClient::waitAsync(Message msg, bool waitForReply) {
	boost::unique_lock<boost::mutex> lock(msg->mMutex);
	if(mSendTimeout.sec == 0)
		msg->waitCondition.wait(lock);
	else {
		bool timeout = msg->waitCondition.timed_wait(lock,  mSendTimeout);
		if(!timeout)
			return 0;
	}
	if(waitForReply) {
		Message reply = receiveMessage();
		return reply;
	}
	else
		return 0;
}

Message MessageClient::sendMessage(BoolMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message msg = createMessage(receiver, MsgType::BoolMsgType);
	msg->boolMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message MessageClient::sendReply(Message previous, BoolMsg& data, bool async, bool waitForReply) {
	Message msg = createMessage(previous->sender, MsgType::BoolMsgType);
	previous->next = msg;
	msg->previous = previous;
	msg->boolMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message MessageClient::sendReply(Message previous, StringMsg& data, bool async, bool waitForReply) {
	Message msg = createMessage(previous->sender, MsgType::StringMsgType);
	previous->next = msg;
	msg->previous = previous;
	msg->stringMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message MessageClient::sendReply(Message previous, DataMsg& data, bool async, bool waitForReply) {
	Message msg = createMessage(previous->sender, MsgType::DataMsgType);
	previous->next = msg;
	msg->previous = previous;
	msg->dataMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message MessageClient::sendMessage(StringMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message msg = createMessage(receiver, MsgType::StringMsgType);
	msg->stringMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message MessageClient::sendMessage(DataMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message msg = createMessage(receiver, MsgType::DataMsgType);
	msg->dataMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

void Task::operator()() {
	while(true) {
		Message msg = receiveMessage();
		if(!msg) {
			quit();
			return;
		}
		handleMessage(msg);
	}
}
