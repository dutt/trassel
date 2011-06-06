#include "timer.h"
#include "messageclient.h"
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

class TestTaskConsumer : public Task {
public:
	void handleMessage(Message msg) {
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
				Timer::sleep(3000);
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
		msg->done();
	}
	void quit() {
		cout <<(int)getID() <<": Shutting down" <<endl;
		return;
	}
};

class TestConsumer : public MessageClient {
public:
	void operator()() {
		bool keep_running = true;
		while(keep_running) {
			Message msg = receiveMessage();
			if(!msg) {
				break; //stop checking for messages, false message means the system is shutting down.
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
					Timer::sleep(3000);
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
			msg->done();
		}
		cout <<(int)getID() <<": Shutting down" <<endl;
	}
};

class TestProducer : public MessageClient {
	MessageClient* mReceiver;
public:
	TestProducer(MessageClient* receiver) : MessageClient(1), mReceiver(receiver) {}

	void operator()() {
		try {
			cout <<(int)getID() <<": Sending messages to " <<(int)mReceiver->getID() <<endl;
			BoolMsg bmsg;
			bmsg.value = true;
			cout <<(int)getID() <<": Sending true bool" <<endl;
			Message reply = sendMessage(bmsg, mReceiver, false, true);
			if(reply) {
				cout  <<(int)getID() <<": Got reply to first msg: " <<reply->boolMsg.value <<endl;
				reply->done();
			}
			else {
				cout <<(int)getID() <<": Reception of reply timed out" <<endl;
			}
		} catch(std::exception ex) {
			cout <<"Failed to send true bool" <<endl;
		}
		Timer::sleep(2000);
		try {
			BoolMsg bmsg;
			bmsg.value = false;
			cout <<(int)getID() <<": Sending false bool" <<endl;
			sendMessage(bmsg, mReceiver);
		} catch(std::exception ex) {
			cout <<"Failed to send false bool" <<endl;
		}
		Timer::sleep(2000);
		try {
			StringMsg smsg;
			smsg.value = "muffins";
			cout <<(int)getID() <<": Sending string" <<endl;
			sendMessage(smsg, mReceiver, false);
		} catch(std::exception ex) {
			cout <<"Failed to send string msg" <<endl;
		}
		Timer::sleep(2000);
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
	DirectedChannel<Message, uint8>::setup();
	TestConsumer tc;
	TestTaskConsumer ttc;
	TestProducer tp(&ttc);
	//new boost::thread(ttc);
	//new boost::thread(tp);
	START_TASK(ttc);
	START_TASK(tp);
	Timer::sleep(18000);
	DirectedChannel<Message, uint8>::shutdown();
	Timer::sleep(500); //wait for final console output
}
