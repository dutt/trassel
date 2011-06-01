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

Message* MessageClient::sendMessage(BoolMsg& data, MessageClient* receiver, bool async, bool waitForReply) {
	Message* msg = new Message();
	msg->previous = msg->next = 0;
	msg->sender = this;
	msg->receiver = receiver;
	msg->type = MsgType::BoolMsgType;
	msg->boolMsg = data;
	produce(msg);
	if(!async) {
		boost::unique_lock<boost::mutex> lock(msg->mMutex);
		msg->waitCondition.wait(lock);
		if(waitForReply) {
			Message* reply = receiveMessage();
			return reply;
		}
	}
	return 0;
}

Message* MessageClient::sendReply(Message* previous, BoolMsg& data, bool async, bool waitForReply) {
	Message* msg = new Message();
	previous->next = msg;
	msg->previous = previous;
	msg->next = 0;
	msg->sender = this;
	msg->receiver = previous->sender;
	msg->boolMsg = data;
	msg->type = MsgType::BoolMsgType;
	produce(msg);
	if(!async) {
		boost::unique_lock<boost::mutex> lock(msg->mMutex);
		msg->waitCondition.wait(lock);
		if(waitForReply) {
			Message* reply = receiveMessage();
			reply->previous = msg;
			return reply;
		}
	}
}

void MessageClient::sendMessage(StringMsg& data, MessageClient* receiver, bool async) {
	Message* msg = new Message();
	msg->sender = this;
	msg->receiver = receiver;
	msg->type = MsgType::StringMsgType;
	msg->stringMsg = data;
	produce(msg);
}

void MessageClient::sendMessage(DataMsg& data, MessageClient* receiver, bool async) {
	Message* msg = new Message();
	msg->sender = this;
	msg->receiver = receiver;
	msg->type = MsgType::DataMsgType;
	msg->dataMsg = data;
	produce(msg);
}
