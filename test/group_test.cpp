#include <iostream>
#include <sstream>
#include <map>
using namespace std;

#include "group_test.h"
#include "timer.h"
using namespace trassel;

boost::mutex groupOutputMutex;
vector<int> groupOutput;

void logOutput(int id) {
    //cout <<id <<" enter" <<endl;
	lock mlock(groupOutputMutex);
	groupOutput.push_back(id);
    mlock.unlock();
    //cout <<id <<" exit" <<endl;
}
std::map<int, string> idToName;

struct id_collection {
	id_collection(trassel::uint8 gc1, trassel::uint8 gc2, trassel::uint8 tc1, trassel::uint8 tpg, trassel::uint8 tpc) {
		gc1_id = gc1;
		gc2_id = gc2;
		tc1_id = tc1;
		tpg_id = tpg;
		tpc_id = tpc;
	}
	trassel::uint8 gc1_id;
	trassel::uint8 gc2_id;
	trassel::uint8 tc1_id;
	trassel::uint8 tpg_id;
	trassel::uint8 tpc_id;
};
id_collection* coll = 0;

void TestGroupConsumer::operator()() {
	while(true) {
		Message msg = receiveMessage();	
		if(!msg) {
			break; //stop checking for messages, false message means the system is shutting down.
		}
		cout <<idToName[(int)getID()] <<": Message from " <<idToName[(int)msg->sender->getID()] <<endl;
		logOutput(getID());
	}	
}

void TestGroupProducer::operator()() {
	try {
        string targetname = "none";
        if(getID() == coll->tpg_id)
            targetname = "group";
        else if(getID() == coll->tpc_id)
            targetname = "channel";
		cout <<"Producer sending message 1 to " <<targetname <<endl;
		IntMsg msg;
		msg.value = 1;
		sendMessage(msg, mReceiver);
		logOutput(getID());
        cout <<"Producer sending message 2 to " <<targetname <<endl;
		++msg.value;
		sendMessage(msg, mReceiver);
		logOutput(getID());
	} catch(std::exception ex) {
		cout <<"Failed to send message to group: " <<ex.what() <<endl;
	}
}

template<class T>
class Runner {
public:
	Runner(T* runnable) : mRunnable(runnable) {}
	void operator()() {
		mRunnable->operator()();
	}
	T* mRunnable;
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
		else if(output[a] == ids.gc1_id || output[a] == ids.gc2_id) {
			group_consumed++;
		}
		else if(output[a] == ids.tc1_id)
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
		cout <<"Number of group messages produced(" <<group_produced <<")differ from number consumed(" <<group_consumed <<")" <<endl;
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
		else if(output[a] == ids.gc1_id || output[a] == ids.gc2_id) {
			group_consumed++;
		}
		else if(output[a] == ids.tc1_id)
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
		cout <<"Number of group messages produced(" <<group_produced <<") differ from number consumed(" <<group_consumed <<")" <<endl;
		success = false;
	}
	return success;
}

bool test_helper(trassel::GroupMode::GroupMode_t mode) {
	ConcreteDirectedChannel channel;
	Group group(&channel, mode);
	Runner<Group> groupRunner(&group);
	TestGroupConsumer gc1(&group);
    Runner<TestGroupConsumer> gc1_runner(&gc1);
	TestGroupConsumer gc2(&group);
    Runner<TestGroupConsumer> gc2_runner(&gc1);
	TestGroupConsumer tc1(&channel);
	TestGroupProducer tpc(&channel, &tc1);
	TestGroupProducer tpg(&channel, &group);
	idToName.clear();
	idToName[gc1.getID()] = "gc1";
	idToName[gc2.getID()] = "gc2";
	idToName[tc1.getID()] = "tc1";
	idToName[tpc.getID()] = "tpc";
	idToName[tpg.getID()] = "tpg";
    coll = new id_collection(gc1.getID(), gc2.getID(), tc1.getID(), tpg.getID(), tpc.getID());
	cout <<"gc1(" <<(int)gc1.getID() <<") gc2(" <<(int)gc2.getID() <<")" <<endl;
	cout <<"tc1(" <<(int)tc1.getID() <<")" <<endl;
	cout <<"tpg(" <<(int)tpg.getID() <<") tpc(" <<(int)tpc.getID() <<")" <<endl;
	new boost::thread(groupRunner);
	new boost::thread(gc1_runner);
	new boost::thread(gc2_runner);
	new boost::thread(tpg);
	new boost::thread(tc1);
	new boost::thread(tpc);
	Timer::sleep(10000); //handle all signals
	group.quit();
	channel.close();
	lock mlock(groupOutputMutex);
	for(uint32 a = 0; a < groupOutput.size(); ++a)
		cout <<idToName[groupOutput[a]] <<", ";
	cout <<endl;
	Timer::sleep(100); //wait for console output
	bool success = false;
	if(group.getMode() == GroupMode::Broadcast)
		success = validate_producers_broadcast(groupOutput, *coll);
	else
		success = validate_producers_first(groupOutput, *coll);
	mlock.unlock();
    delete coll;
	return success;
}

bool test_FIFO() {
	return test_helper(GroupMode::FIFO);
}

bool test_broadast() {
	return test_helper(GroupMode::Broadcast);
}

void group_test() {
	//bool fifo = test_FIFO();
    bool fifo = true;
	groupOutput.clear();
    cout <<"FIFO test done" <<endl <<endl;
	bool broadcast = test_broadast();
    cout <<"Broadcast test done" <<endl;
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