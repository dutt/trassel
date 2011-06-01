#include "timer.h"
#include "threadpool.h"
#include "messagequeue.h"
#include <iostream>
using namespace std;

struct container {
	container() : i(0), c(0) { }
	container(int _i, char _c) {
		i = _i;
		c = _c;
	}
	int i;
	char c;
};

class TestConsumer : public MessageClient {
public:
	void operator()() {
		while(true) {
			Message* msg = receiveMessage();
			cout <<(int)getID() <<": Message from " <<(int)msg->sender->getID() <<endl;
			switch(msg->type) {
				case MsgType::BoolMsgType:
					cout <<(int)getID() <<": Bool: " <<(msg->boolMsg.value?"true":"false") <<endl;
					if(!msg->boolMsg.value) {
						cout <<(int)getID() <<": Processing bool" <<endl;
						for(int i = 0; i < 3; ++i) {
							cout <<".";
							Timer::sleep(3000);
						}
						cout <<"done" <<endl;
					}
					else {
						BoolMsg data;
						data.value = false;
						cout  <<(int)getID() <<": Sending reply" <<endl;
						sendReply(msg, data);
					}
					break;
				case MsgType::StringMsgType:
					cout <<(int)getID() <<": String: \"" <<msg->stringMsg.value <<"\"" <<endl;
					break;
				case MsgType::DataMsgType:
					cout <<(int)getID() <<": Data: ";
					container c;
					memcpy(&c, msg->dataMsg.data, msg->dataMsg.length);
					cout <<"i = " <<c.i <<", c = " <<c.c <<endl;
					break;
			}
			delete msg; //we're done with it
		}
	}
};

class TestProducer : public MessageClient {
	MessageClient* mReceiver;
public:
	TestProducer(MessageClient* receiver) : mReceiver(receiver) {}

	void operator()() {
		cout <<(int)getID() <<": Sending messages to " <<(int)mReceiver->getID() <<endl;
		BoolMsg bmsg;
		bmsg.value = true;
		cout <<(int)getID() <<": Sending true bool" <<endl;
		Message* reply = sendMessage(bmsg, mReceiver, false, true);
		if(reply != 0) {
			cout  <<(int)getID() <<": Got reply to first msg: " <<reply->boolMsg.value <<endl;
		}

		bmsg.value = false;
		cout <<(int)getID() <<": Sending false bool" <<endl;
		sendMessage(bmsg, mReceiver);

		StringMsg smsg;
		smsg.value = "muffins";
		cout <<(int)getID() <<": Sending string" <<endl;
		sendMessage(smsg, mReceiver, false);
		
		DataMsg dmsg;
		dmsg.data = new uint8[sizeof(container)];
		dmsg.length = sizeof(container);
		container c(12, 'D');
		memcpy(dmsg.data, &c, sizeof(container));
		cout <<(int)getID() <<": Sending data" <<endl;
		sendMessage(dmsg, mReceiver);
	}
};

int main(int argc, char** argv) {
	//Channel<Message*>::setup();
	DirectedChannel::setup();
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

