#include "timer.h"
#include "threadpool.h"
#include "messagequeue.h"
#include <iostream>
using namespace std;

boost::mutex mMutex;
typedef boost::unique_lock<boost::mutex> Lock;
class TestConsumer : public MessageClient {
public:
	void operator()() {
		while(true) {
			Lock lock(mMutex);
			Message* msg = recieveMessage();
			cout <<(int)getID() <<" Message from " <<(int)msg->sender->getID() <<endl;
			switch(msg->type) {
				case MsgType::BoolMsgType:
					cout <<(int)getID() <<" Bool: " <<(msg->boolMsg.value?"true":"false") <<endl;
					if(!msg->boolMsg.value) {
						cout <<(int)getID() <<" waiting" <<endl;
						Timer::sleep(10000);
					}
					break;
				case MsgType::StringMsgType:
					cout <<(int)getID() <<" String: \"" <<msg->stringMsg.value <<"\"" <<endl;
					break;
			}
			delete msg;
			lock.unlock();
		}
	}
};

class TestProducer : public MessageClient {
	MessageClient* mReceiver;
public:
	TestProducer(MessageClient* receiver) : mReceiver(receiver) {}

	void operator()() {
		cout <<(int)getID() <<" Sending messages to " <<(int)mReceiver->getID() <<endl;
		BoolMsg bmsg;
		bmsg.value = true;
		cout <<(int)getID() <<" Sending true bool" <<endl;
		sendMessage(bmsg, mReceiver, false);
		bmsg.value = false;
		cout <<(int)getID() <<" Sending false bool" <<endl;
		sendMessage(bmsg, mReceiver);
		StringMsg smsg;
		smsg.value = "muffins";
		cout <<(int)getID() <<" Sending string" <<endl;
		sendMessage(smsg, mReceiver, false);
	}
};

int main(int argc, char** argv) {
	Channel<Message*>::setup();
	//int csize = 1;
	//int psize = 1;
	//TestConsumer* c = new TestConsumer[csize];
	//TestProducer* p = new TestProducer[psize];
	//boost::thread** cthreads = new boost::thread*[csize];
	//boost::thread** pthreads = new boost::thread*[psize];
	TestConsumer tc;
	TestProducer tp(&tc);
	boost::thread* cthread = new boost::thread(tc);
	boost::thread* pthread = new boost::thread(tp);
	//for(int a = 0; a < csize; ++a)
	//	cthreads[a] = new boost::thread(TestConsumer());
	//for(int a = 0; a < psize; ++a)
	//	pthreads[a] = new boost::thread(TestProducer());
	while(true) {
		Timer::sleep(1);
	}
}

