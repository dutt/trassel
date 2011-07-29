#include "messageclient.h"

class TestGroupConsumer : public trassel::MessageClient {
public:
	TestGroupConsumer(trassel::DirectedChannel<trassel::Message, trassel::uint8>* channel) : trassel::MessageClient(channel) {
		mGroup = (trassel::Group*)channel;
		mGroup->attach(this);
	}
	~TestGroupConsumer() {
		mGroup->detach(this);
	}
	void operator()();
private:
	trassel::Group* mGroup;
};

class TestGroupProducer : public trassel::MessageClient {
	MessageClient* mReceiver;
public:
	TestGroupProducer(trassel::DirectedChannel<trassel::Message, trassel::uint8>* channel, trassel::MessageClient* receiver) : trassel::MessageClient(channel), mReceiver(receiver) {}

	void operator()();
};

void group_test();