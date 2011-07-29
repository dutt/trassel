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

class TestDirectedTaskConsumer : public trassel::Task {
public:
	TestDirectedTaskConsumer(trassel::DirectedChannel<trassel::Message, trassel::uint8>* channel) : trassel::Task(channel) {}
	void handleMessage(trassel::Message msg);
	void quit();
};

class TestDirectedConsumer : public trassel::MessageClient {
public:
	TestDirectedConsumer(trassel::DirectedChannel<trassel::Message, trassel::uint8>* channel) : trassel::MessageClient(channel) {}
	void operator()();
};

class TestDirectedProducer : public trassel::MessageClient {
	MessageClient* mReceiver;
public:
	TestDirectedProducer(trassel::DirectedChannel<trassel::Message, trassel::uint8>* channel, trassel::MessageClient* receiver) : trassel::MessageClient(channel, 10), mReceiver(receiver) {}

	void operator()();
};

void directed_test();