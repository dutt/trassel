#include "directed_test.h"
#include "timer.h"
using namespace trassel;

#include <iostream>
using namespace std;

boost::mutex directedOutputMutex;
vector<int> directedOutput;

void log(int i) {
	lock mlock(directedOutputMutex);
	directedOutput.push_back(i);
	mlock.unlock();
}
void TestDirectedTaskConsumer::handleMessage(Message msg) {
	cout <<(int)getID() <<": Message from " <<(int)msg->sender->getID() <<endl;
	switch(msg->type) {
	case MsgType::BoolMsgType:
		cout <<(int)getID() <<": Bool: " <<(msg->boolMsg.value?"true":"false") <<endl;
		if(!msg->boolMsg.value) {
			log(41);
			cout <<(int)getID() <<": Processing bool";
			for(int i = 0; i < 2; ++i) {
				cout <<".";
				Timer::sleep(500);
			}
			cout <<"done" <<endl;
		}
		else {
			BoolMsg data;
			data.value = false;
			cout  <<(int)getID() <<": Sending reply" <<endl;
			log(2);
			sendReply(msg, data);
		}
		break;
	case MsgType::StringMsgType:
		cout <<(int)getID() <<": Processing string";
		for(int i = 0; i < 2; ++i) {
			cout <<".";
			Timer::sleep(500);
		}
		cout <<"done" <<endl;
		cout <<(int)getID() <<": String: \"" <<msg->stringMsg.value <<"\"" <<endl;
		log(5);
		break;
	case MsgType::DataMsgType:
		cout <<(int)getID() <<": Data: ";
		container c;
		memcpy(&c, msg->dataMsg.value, msg->dataMsg.len);
		cout <<"i = " <<c.i <<", c = " <<c.c <<endl;
		log(71);
		break;
	}
	msg->done();
}

void TestDirectedTaskConsumer::quit() {
	cout <<(int)getID() <<": Shutting down" <<endl;
	return;
}

void TestDirectedConsumer::operator()() {
	while(true) {
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
					Timer::sleep(500);
				}
				cout <<"done" <<endl;
			}
			else {
				BoolMsg data;
				data.value = false;
				Timer::sleep(500);
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

void TestDirectedProducer::operator()() {
	try {
		cout <<(int)getID() <<": Sending messages to " <<(int)mReceiver->getID() <<endl;
		BoolMsg bmsg;
		bmsg.value = true;
		cout <<(int)getID() <<": Sending true bool" <<endl;
		Message reply = sendMessage(bmsg, mReceiver, false, true);
		log(1);
		if(reply) {
			cout  <<(int)getID() <<": Got reply to first msg: " <<reply->boolMsg.value <<endl;
			log(3);
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
		cout <<(int)getID() <<": Sending false bool async" <<endl;
		sendMessage(bmsg, mReceiver);
		log(42);
	} catch(std::exception ex) {
		cout <<"Failed to send false bool" <<endl;
	}
	Timer::sleep(2000);
	try {
		StringMsg smsg;
		smsg.value = "muffins";
		cout <<(int)getID() <<": Sending string sync" <<endl;
		sendMessage(smsg, mReceiver, false);
		log(6);
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
		log(72);
	} catch(std::exception ex) {
		cout <<"Failed to send data msg" <<endl;
	}
}

struct id_collection {
	id_collection(trassel::uint8 dc, trassel::uint8 dtc, trassel::uint8 dp) {
		dc_id = dc;
		dtc_id = dtc;
		dp_id = dp;
	}
	trassel::uint8 dc_id;
	trassel::uint8 dtc_id;
	trassel::uint8 dp_id;
};

void directed_test() {
	//DirectedChannel<Message, uint8> channel;
	ConcreteDirectedChannel channel;
	TestDirectedConsumer dc(&channel);
	TestDirectedTaskConsumer dtc(&channel);
	TestDirectedProducer dp(&channel, &dc);
	new boost::thread(dc);
	new boost::thread(dp);
	id_collection ids(dc.getID(), dtc.getID(), dp.getID());
	//START_TASK(ttc);
	//START_TASK(tp);
	//int debug_stop = 1;
	//while(debug_stop) {}
	Timer::sleep(18000);
	channel.close();
	Timer::sleep(500); //wait for final console output
	lock mlock(directedOutputMutex);
	for(uint32 a = 0; a < directedOutput.size(); ++a)
		cout <<directedOutput[a] <<", ";
	cout <<endl;
	Timer::sleep(18000);
}
