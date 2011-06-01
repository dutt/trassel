#include "messagequeue.h"

uint8 MessageClient::lastID = 0;

void DirectedProducer::produce(Message* data) {
	DirectedChannel::getInstance()->push(data);
}

Message* DirectedConsumer::consume(uint8 id) {
	return DirectedChannel::getInstance()->pop(id);
}


Message* MessageClient::receiveMessage() {
	return consume(mID);
}

Message* MessageClient::createMessage(MessageClient* receiver, MsgType::MsgTypeEnum type) {
	Message* msg = new Message();
	msg->previous = msg->next = 0;
	msg->sender = this;
	msg->receiver = receiver;
	msg->type = type;
	return msg;
}
Message* MessageClient::waitAsync(Message* msg, bool waitForReply) {
	boost::unique_lock<boost::mutex> lock(msg->mMutex);
	msg->waitCondition.wait(lock);
	if(waitForReply) {
		Message* reply = receiveMessage();
		return reply;
	}
	else
		return 0;
}
Message* MessageClient::sendMessage(BoolMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message* msg = createMessage(receiver, MsgType::BoolMsgType);
	msg->boolMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message* MessageClient::sendReply(Message* previous, BoolMsg& data, bool async, bool waitForReply) {
	Message* msg = createMessage(previous->sender, MsgType::BoolMsgType);
	previous->next = msg;
	msg->previous = previous;
	msg->boolMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message* MessageClient::sendMessage(StringMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message* msg = createMessage(receiver, MsgType::StringMsgType);
	msg->stringMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}

Message* MessageClient::sendMessage(DataMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message* msg = createMessage(receiver, MsgType::DataMsgType);
	msg->dataMsg = data;
	produce(msg);
	if(!async) {
		return waitAsync(msg, waitForReply);
	}
	return 0;
}
