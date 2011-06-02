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
			if(msg == 0) {
				cout <<(int)getID() <<": Shutting down" <<endl;
				return;
			}
			cout <<(int)getID() <<": Message from " <<(int)msg->sender->getID() <<endl;
			switch(msg->type) {
				case MsgType::BoolMsgType:
					cout <<(int)getID() <<": Bool: " <<(msg->boolMsg.value?"true":"false") <<endl;
					if(!msg->boolMsg.value) {
						cout <<(int)getID() <<": Processing bool";
						for(int i = 0; i < 2; ++i) {
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
					cout <<(int)getID() <<": Processing string";
					for(int i = 0; i < 2; ++i) {
						cout <<".";
						Timer::sleep(3000);
					}
					cout <<"done" <<endl;
					cout <<(int)getID() <<": String: \"" <<msg->stringMsg.value <<"\"" <<endl;
					break;
				case MsgType::DataMsgType:
					cout <<(int)getID() <<": Data: ";
					container c;
					memcpy(&c, msg->dataMsg.value, msg->dataMsg.len);
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
		try {
			cout <<(int)getID() <<": Sending messages to " <<(int)mReceiver->getID() <<endl;
			BoolMsg bmsg;
			bmsg.value = true;
			cout <<(int)getID() <<": Sending true bool" <<endl;
			Message* reply = sendMessage(bmsg, mReceiver, false, true);
			if(reply != 0) {
				cout  <<(int)getID() <<": Got reply to first msg: " <<reply->boolMsg.value <<endl;
			}
			delete reply;
		} catch(std::exception ex) {
			cout <<"Failed to send true bool" <<endl;
		}

		try {
			BoolMsg bmsg;
			bmsg.value = false;
			cout <<(int)getID() <<": Sending false bool" <<endl;
			sendMessage(bmsg, mReceiver);
		} catch(std::exception ex) {
			cout <<"Failed to send false bool" <<endl;
		}
		
		try {
			StringMsg smsg;
			smsg.value = "muffins";
			cout <<(int)getID() <<": Sending string" <<endl;
			sendMessage(smsg, mReceiver, false);
		} catch(std::exception ex) {
			cout <<"Failed to send string msg" <<endl;
		}
		
		try {
			DataMsg dmsg;
			dmsg.value = new uint8[sizeof(container)];
			dmsg.len = sizeof(container);
			container c(12, 'D');
			memcpy(dmsg.value, &c, sizeof(container));
			cout <<(int)getID() <<": Sending data" <<endl;
			sendMessage(dmsg, mReceiver);
		} catch(std::exception ex) {
			cout <<"Failed to send data msg" <<endl;
		}
	}
};

int main(int argc, char** argv) {
	DirectedChannel::setup();
	TestConsumer tc;
	TestProducer tp(&tc);
	boost::thread* cthread = new boost::thread(tc);
	boost::thread* pthread = new boost::thread(tp);
	Timer::sleep(15000);
	DirectedChannel::shutdown();
}
