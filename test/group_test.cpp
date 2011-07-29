#include <iostream>
#include <sstream>
using namespace std;

#include "group_test.h"
#include "timer.h"
using namespace trassel;

boost::mutex groupOutputMutex;
vector<int> groupOutput;

void TestGroupConsumer::operator()() {
	while(true) {
		Message msg = receiveMessage();	
		cout <<"Consumer got message" <<endl;
		if(!msg) {
			break; //stop checking for messages, false message means the system is shutting down.
		}
		cout <<(int)getID() <<": Message from " <<(int)msg->sender->getID() <<endl;
		lock mlock(groupOutputMutex);
		groupOutput.push_back(getID());
		mlock.unlock();
	}	
}

void TestGroupProducer::operator()() {
	try {
		cout <<"Producer sending message group" <<endl;
		BoolMsg bmsg;
		bmsg.value = true;
		sendMessage(bmsg, mReceiver);
		lock mlock(groupOutputMutex);
		groupOutput.push_back(getID());
		mlock.unlock();
		sendMessage(bmsg, mReceiver);
		mlock.lock();
		groupOutput.push_back(getID());
		mlock.unlock();
	} catch(std::exception ex) {
		cout <<"Failed to send true bool" <<ex.what() <<endl;
	}
}

class GroupRunner {
public:
	GroupRunner(Group* group) : mGroup(group) {}
	void operator()() {
		mGroup->operator()();
	}
	trassel::Group* mGroup;
};

struct id_collection {
	id_collection(trassel::uint8 gc1, trassel::uint8 gc2, trassel::uint8 tc1, trassel::uint8 tpg, trassel::uint8 tpc) {
		gc1_id = gc1;
		producer_id = gc2;
		dp_id = tc1;
		tpg_id = tpg;
		tpc_id = tpc;
	}
	trassel::uint8 gc1_id;
	trassel::uint8 producer_id;
	trassel::uint8 dp_id;
	trassel::uint8 tpg_id;
	trassel::uint8 tpc_id;
};

bool validate_producers_broadcast(vector<int>& output, id_collection& ids) {
	int channel_produced = 0; //channel producers
	int group_produced = 0; //group producers
	int group_consumed = 0; //group consumers
	int channel_consumed = 0; //channel consumers
	bool success = true;
	if(output.empty()) {
		cout <<"Nothing has happened" <<endl;
		return false;
	}
	for(trassel::uint32 a = 0; a < output.size(); ++a) {
		if(output[a] == ids.tpc_id)
			channel_produced++;
		else if(output[a] == ids.tpg_id) {
			group_produced += 2;
		}
		else if(output[a] == ids.gc1_id || output[a] == ids.producer_id) {
			group_consumed++;
		}
		else if(output[a] == ids.dp_id)
			channel_consumed++;
		if(group_consumed > group_produced) {
			cout <<"Position " <<(int)a <<" (" <<output[a] <<") is wrong. Consumed more messages from group than was produced" <<endl;
			success = false;
		}
		if(channel_consumed > channel_produced) {
			cout <<"Position " <<(int)a <<" (" <<output[a] <<") is wrong. Consumed more messages from channel than was produced" <<endl;
			success = false;
		}
	}
	if(channel_produced != channel_consumed) {
		cout <<"Number of channel messages produced(" <<channel_produced <<")differ from number consumed(" <<channel_consumed <<")" <<endl;
		success = false;
	}
	if(group_produced != group_consumed) {
		cout <<"Number of group messages produced(" <<channel_produced <<")differ from number consumed(" <<channel_consumed <<")" <<endl;
		success = false;
	}
	return success;
}

bool validate_producers_first(vector<int>& output, id_collection& ids) {
	int channel_produced = 0; //channel producers
	int group_produced = 0; //group producers
	int group_consumed = 0; //group consumers
	int channel_consumed = 0; //channel consumers
	bool success = true;
	if(output.empty()) {
		cout <<"Nothing has happened" <<endl;
		return false;
	}
	for(trassel::uint32 a = 0; a < output.size(); ++a) {
		if(output[a] == ids.tpc_id)
			channel_produced++;
		else if(output[a] == ids.tpg_id) {
			group_produced++;
		}
		else if(output[a] == ids.gc1_id || output[a] == ids.producer_id) {
			group_consumed++;
		}
		else if(output[a] == ids.dp_id)
			channel_consumed++;
		if(group_consumed > group_produced) {
			cout <<"Position " <<(int)a <<" (" <<output[a] <<") is wrong. Consumed more messages from group than was produced" <<endl;
			success = false;
		}
		if(channel_consumed > channel_produced) {
			cout <<"Position " <<(int)a <<" (" <<output[a] <<") is wrong. Consumed more messages from channel than was produced" <<endl;
			success = false;
		}
	}
	if(channel_produced != channel_consumed) {
		cout <<"Number of channel messages produced(" <<channel_produced <<")differ from number consumed(" <<channel_consumed <<")" <<endl;
		success = false;
	}
	if(group_produced != group_consumed) {
		cout <<"Number of group messages produced(" <<channel_produced <<")differ from number consumed(" <<channel_consumed <<")" <<endl;
		success = false;
	}
	return success;
}

bool test_helper(trassel::GroupMode::GroupMode_t mode) {
	ConcreteDirectedChannel channel;
	Group group(&channel, mode);
	GroupRunner groupRunner(&group);
	TestGroupConsumer gc1(&group);
	TestGroupConsumer gc2(&group);
	TestGroupConsumer tc1(&channel);
	TestGroupProducer tpc(&channel, &tc1);
	TestGroupProducer tpg(&channel, &group);
	id_collection coll(gc1.getID(), gc2.getID(), tc1.getID(), tpg.getID(), tpc.getID());
	cout <<"gc1(" <<(int)gc1.getID() <<") gc2(" <<(int)gc2.getID() <<")" <<endl;
	cout <<"tc1(" <<(int)tc1.getID() <<")" <<endl;
	cout <<"tpg(" <<(int)tpg.getID() <<") tpc(" <<(int)tpc.getID() <<")" <<endl;
	new boost::thread(groupRunner);
	new boost::thread(gc1);
	new boost::thread(gc2);
	new boost::thread(tpg);
	new boost::thread(tc1);
	new boost::thread(tpc);
	Timer::sleep(100);
	channel.close();
	lock mlock(groupOutputMutex);
	for(uint32 a = 0; a < groupOutput.size(); ++a)
		cout <<groupOutput[a] <<", ";
	cout <<endl;
	Timer::sleep(100); //wait for console output
	bool success = false;
	if(group.getMode() == GroupMode::Broadcast)
		return validate_producers_broadcast(groupOutput, coll);
	else
		return validate_producers_first(groupOutput, coll);

}
bool test_FIFO() {
	return test_helper(GroupMode::FIFO);
}

bool test_broadast() {
	return test_helper(GroupMode::Broadcast);
}

void group_test() {
	bool fifo = test_FIFO();
	groupOutput.clear();
	bool broadcast = test_broadast();
	if(fifo && broadcast) {
		cout <<"Test succeeded" <<endl;
	}
	else {
		if(!fifo)
			cout <<"FIFO Group messaging failed" <<endl;
		if(!broadcast)
			cout <<"Broadcast Group messaging failed" <<endl;
	}
}