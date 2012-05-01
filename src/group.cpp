#include "group.h"
#include "timer.h"

using namespace trassel;

Group::Group(Channel<Message, uint8>* channel, GroupMode::GroupMode_t mode)
	: Task(channel), mMode(mode), mQuit(false) {
		if(mode == GroupMode::FIFO)
			mClients[0] = std::queue<Message>();
}

Group::~Group() {
	quit();
}

void Group::attach(MessageClient* client) {
	if(mMode == GroupMode::Broadcast) {
        mClients[client->getID()] = std::queue<Message>();
        cout <<"attach" <<endl;
    }
		
}

void Group::detach(MessageClient* client) {
	if(mMode == GroupMode::Broadcast) {
		ClientIt it = mClients.find(client->getID());
		if(it != mClients.end()) {
            mClients.erase(it);
            cout <<"detach" <<endl;
        }
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
    lock mlock(mMutex);
	mQuit = true;
	mlock.unlock();
    mEmptyCondition.notify_all();
	while(mClients.size() > 0) { //wait for all messages to be processed
		//cout <<"size a " <<mClients.size() <<endl;
        if(mClients.begin()->second.size() == 0) {
            //cout <<"Queue for " <<(int)mClients.begin()->first <<" was empty" <<endl;
            //cout <<"size1 " <<mClients.size() <<endl;
            mClients.erase(mClients.begin());
            //cout <<"size2 " <<mClients.size() <<endl;
        } else {
            //cout <<"Queue for " <<(int)mClients.begin()->first <<" still has " <<mClients.begin()->second.size() <<" messages" <<endl;
        }
        Timer::sleep(10);
        mEmptyCondition.notify_all();
        //cout <<"size b " <<mClients.size() <<endl;
	}
	//cout <<"Queue cleaning is done" <<endl;
	//cout <<"quit() is done" <<endl;
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
    cout <<"popInternal(" <<(uint16)id <<") enter" <<endl;
    if(mQuit || mClients.find(id) == mClients.end()) {
    	waitLock.unlock();
    	cout <<"popInternal(" <<(uint16)id <<") exit early" <<endl;
    	return Message();
    }
    
	std::queue<Message>& q = mClients[id];
	while(mClients[id].empty()) {
		cout <<"popInternal(" <<(uint16)id <<") waiting" <<endl;
		mEmptyCondition.wait(waitLock);
        if(mQuit) {
            waitLock.unlock();
            cout <<"popInternal(" <<(uint16)id <<") exit 2" <<endl;
            return Message();
        }
	}
	Message ret = q.front();
	q.pop();
	waitLock.unlock();
    cout <<"popInternal(" <<(uint16)id <<") exit msg" <<endl;
	return ret;
}
