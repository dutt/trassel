#include "timer.h"
#include "messageclient.h"

struct container {
	container() : i(0), c(0) { }
	container(int _i, char _c) {
		i = _i;
		c = _c;
	}
	int i;
	char c;
};

class TestTaskConsumer : public trassel::Task {
public:
	void handleMessage(trassel::Message msg);
	void quit();
};

class TestConsumer : public trassel::MessageClient {
public:
	void operator()();
};

class TestProducer : public trassel::MessageClient {
	MessageClient* mReceiver;
public:
	TestProducer(trassel::MessageClient* receiver) : MessageClient(1), mReceiver(receiver) {}

	void operator()();
};

void directed_test();