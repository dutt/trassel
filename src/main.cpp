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
			switch(msg->type) {
				case MsgType::BoolMsgType:
					cout <<"Bool: " <<(msg->boolMsg.value?"true":"false") <<endl;
					break;
				case MsgType::StringMsgType:
					cout <<"String: \"" <<msg->stringMsg.value <<"\"" <<endl;
					break;
			}
			lock.unlock();
		}
	}
};

class TestProducer : public MessageClient {
public:
	void operator()() {
		for(int a = 5; a < 6; ++a) {
			BoolMsg bmsg;
			bmsg.value = true;
			sendMessage(bmsg, 0, 0);
			bmsg.value = false;
			sendMessage(bmsg, 0, 0);
			StringMsg smsg;
			smsg.value = "muffins";
			sendMessage(smsg, 0, 0);
		}
	}
};

int main(int argc, char** argv) {
	Channel<Message*>::setup();
	int csize = 1;
	int psize = 1;
	TestConsumer* c = new TestConsumer[csize];
	TestProducer* p = new TestProducer[psize];
	boost::thread** cthreads = new boost::thread*[csize];
	boost::thread** pthreads = new boost::thread*[psize];
	for(int a = 0; a < csize; ++a)
		cthreads[a] = new boost::thread(TestConsumer());
	for(int a = 0; a < psize; ++a)
		pthreads[a] = new boost::thread(TestProducer());
	while(true) {
		Timer::sleep(1);
	}
}

