#include "group.h"

using namespace trassel;

Group::Group(Channel<Message, uint8>* channel, GroupMode::GroupMode_t mode)
	: Task(channel), mQuit(false), mMode(mode) {
		if(mode == GroupMode::FIFO)
			mClients[0] == std::queue<Message>();
}

Group::~Group() {
	quit();
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
	if(mMode == GroupMode::FIFO) {
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
	while(mClients.size() > 0) { //wait for all messages to be processed
		cout <<"loop1" <<endl;
		for(ClientIt it = mClients.begin(); it != mClients.end(); ++it) {
			cout <<"loop2" <<endl;
			if(it->second.size() == 0) {
				cout <<"Queue for " <<(int)it->first <<" was empty" <<endl;
				mClients.erase(it);
				it = mClients.begin();
			} else {
				cout <<"Queue for " <<(int)it->first <<" still has " <<it->second.size() <<" messages" <<endl;
			}
			cout <<"loop2.2" <<endl;
		}
		cout <<"loop1.2" <<endl;
	}
	cout <<"Queue cleaning is done" <<endl;
	lock mlock(mMutex);
	mQuit = true;
	mlock.unlock();
	cout <<"quit() is done" <<endl;
}

void Group::push(Message msg) {

}

Message Group::pop(uint8 id) {
	lock mlock(mMutex);
	if(mQuit) {
		mlock.unlock();
		return Message();
	}
	else {
		if(mMode == GroupMode::FIFO)
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
