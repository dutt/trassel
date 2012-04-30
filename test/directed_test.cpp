#include "directed_test.h"
#include "timer.h"
using namespace trassel;

#include <iostream>
using namespace std;

boost::mutex directedOutputMutex;
vector<int> directedOutput;

static void log(int i) {
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
			log(5);
			cout <<(int)getID() <<": Processing bool";
			for(int i = 0; i < 2; ++i) {
				cout <<".";
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
		log(6);
		for(int i = 0; i < 2; ++i) {
			cout <<".";
		}
		cout <<"done" <<endl;
		cout <<(int)getID() <<": String: \"" <<msg->stringMsg.value <<"\"" <<endl;
		break;
	case MsgType::DataMsgType:
		cout <<(int)getID() <<": Data: ";
		container c;
		memcpy(&c, msg->dataMsg.value, msg->dataMsg.len);
		cout <<"i = " <<c.i <<", c = " <<c.c <<endl;
		log(9);
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
			int a = 2+3;
			cout <<"quit" <<endl;
			break; //stop checking for messages, false message means the system is shutting down.
		}
		cout <<(int)getID() <<": Message from " <<(int)msg->sender->getID() <<endl;
		switch(msg->type) {
		case MsgType::BoolMsgType:
			cout <<(int)getID() <<": Bool: " <<(msg->boolMsg.value?"true":"false") <<endl;
			if(!msg->boolMsg.value) {
				log(5);
				cout <<(int)getID() <<": Processing bool";
				for(int i = 0; i < 2; ++i) {
					cout <<".";
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
			}
			cout <<"done" <<endl;
			cout <<(int)getID() <<": String: \"" <<msg->stringMsg.value <<"\"" <<endl;
			log(6);
			break;
		case MsgType::DataMsgType:
			cout <<(int)getID() <<": Data: ";
			container c;
			memcpy(&c, msg->dataMsg.value, msg->dataMsg.len);
			cout <<"i = " <<c.i <<", c = " <<c.c <<endl;
			log(9);
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
		log(1);
		Message reply = sendMessage(bmsg, mReceiver, false, true);
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
	try {
		BoolMsg bmsg;
		bmsg.value = false;
		cout <<(int)getID() <<": Sending false bool async" <<endl;
		log(4);
		sendMessage(bmsg, mReceiver);
	} catch(std::exception ex) {
		cout <<"Failed to send false bool" <<endl;
	}
	try {
		StringMsg smsg;
		smsg.value = "muffins";
		cout <<(int)getID() <<": Sending string sync" <<endl;
		log(5);
		sendMessage(smsg, mReceiver, false);
		log(7);
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
		log(8);
		sendMessage(dmsg, mReceiver);
		log(9);
	} catch(std::exception ex) {
		cout <<"Failed to send data msg" <<endl;
	}
}

struct id_collection {
	id_collection(trassel::uint8 consumer, trassel::uint8 producer) {
		consumer_id = consumer;
		producer_id = producer;
	}
	trassel::uint8 consumer_id;
	trassel::uint8 producer_id;
};

bool verify() {
	int max_found = 0;
	for(uint32 a = 0; a < directedOutput.size(); ++a) {
		if(directedOutput[a] < max_found) { //the numbers should not go down, 
											//then something is out of order
			cout <<"Item " <<a <<" was too late." <<endl;
			return false;
		}
		else if(directedOutput[a] > max_found)
			max_found = directedOutput[a];
	}
	return true;
}

bool test_directed_task() {
	cout <<"directed task" <<endl;
	ConcreteDirectedChannel channel;
	TestDirectedTaskConsumer consumer(&channel);
	TestDirectedProducer producer(&channel, &consumer);
	new boost::thread(consumer);
	new boost::thread(producer);
	id_collection ids(consumer.getID(), producer.getID());
	Timer::sleep(500);
	channel.close();
	Timer::sleep(100); //wait for final console output
	lock mlock(directedOutputMutex);
	for(uint32 a = 0; a < directedOutput.size(); ++a)
		cout <<directedOutput[a] <<", ";
	cout <<endl;
	channel.close();
	return verify();
}

bool test_directed_client() {
	cout <<"directed client" <<endl;
	ConcreteDirectedChannel channel;
	TestDirectedConsumer consumer(&channel);
	TestDirectedProducer producer(&channel, &consumer);
	new boost::thread(consumer);
	new boost::thread(producer);
	id_collection ids(consumer.getID(), producer.getID());
	Timer::sleep(100); //wait for messages to be processed
	channel.close();
	Timer::sleep(100); //wait for final console output
	lock mlock(directedOutputMutex);
	for(uint32 a = 0; a < directedOutput.size(); ++a)
		cout <<directedOutput[a] <<", ";
	cout <<endl;
	channel.close();
	return verify();
}

void directed_test() {
	bool client = test_directed_client();
	directedOutput.clear();
	cout <<"Test of directed client done" <<endl;
	bool task = test_directed_task();
	cout <<"Test of directed task done" <<endl;
	if(client && task) {
		cout <<"Test successful" <<endl;
	}
	else {
		if(!client)
			cout <<"Test of MessageClient failed" <<endl;
		if(!task)
			cout <<"Test of Task failed" <<endl;
	}
	directedOutput.clear();
}
