#include "messageclient.h"
using namespace trassel;

using namespace std;

using namespace boost;

uint8 MessageClient::lastID = 0;

MessageClient::MessageClient(Channel<Message, uint8>* channel, uint32 send_timeout)
	: DirectedConsumer(channel), DirectedProducer(channel), mID(lastID++), mSendTimeout(0, 0, send_timeout)
{ }

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

struct Message_Has_Reply {
	Message msg;
	Message_Has_Reply(Message sent_message) : msg(sent_message) {}
	bool operator()() const {
		return msg->next != 0;
	}
};
Message MessageClient::waitAsync(Message msg, bool waitForReply) {
	unique_lock<boost::mutex> lock(msg->mMutex);
	if(mSendTimeout.seconds() == 0)
		msg->waitCondition.wait(lock);
	else {
		if(!msg->waitCondition.timed_wait(lock, mSendTimeout, Message_Has_Reply(msg)))
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
	unique_lock<boost::mutex> lock(previous->mMutex);
	previous->next = msg;
	lock.unlock();
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

//
// Group

Group::Group(Channel<Message, uint8>* channel, GroupMode::GroupMode_t mode)
: Task(channel), mQuit(false), mMode(mode) {
	if(mode == GroupMode::FirstComeFirstServe)
		mClients[0] == std::queue<Message>();
}

void Group::attach(MessageClient* client) {
	if(mMode == GroupMode::Broadcast)
		mClients[client->getID()] = std::queue<Message>();
}

void Group::detach(MessageClient* client) {
	if(mMode == GroupMode::Broadcast) {
		ClientIt it = mClients.find(client->getID());
		if(it != mClients.end())
			mClients.erase(it);
	}
}

bool Group::isAttached(MessageClient* client) {
	return mClients.find(client->getID()) != mClients.end();
}

void Group::handleMessage(Message msg) {
	if(mMode == GroupMode::FirstComeFirstServe) {
		mClients[0].push(msg);
		mEmptyCondition.notify_one();
	}
	else if(mMode == GroupMode::Broadcast) {
		for(ClientIt it = mClients.begin(); it != mClients.end(); ++it)
			it->second.push(msg);
		mEmptyCondition.notify_all();
	}
}

void Group::quit() {
	lock mlock(mMutex);
	mQuit = true;
	mlock.unlock();
}

void Group::push(Message msg) {

}

Message Group::pop(uint8 id) {
	lock mlock(mMutex);
	if(mQuit) {
		mlock.unlock();
		return 0;
	}
	else {
		if(mMode == GroupMode::FirstComeFirstServe)
			return popInternal(0, mlock);
		else if(mMode == GroupMode::Broadcast)
			return popInternal(id, mlock);
		else {
			mlock.unlock();
			throw "Invalid group mode";
		}
	}
}

Message Group::popInternal(uint8 id, lock& waitLock) {
	std::queue<Message>& q = mClients[id];
	while(mClients[id].empty()) {
		mEmptyCondition.wait(waitLock);
	}
	Message ret = q.front();
	q.pop();
	waitLock.unlock();
	return ret;
}