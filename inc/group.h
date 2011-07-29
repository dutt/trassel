#ifndef _TRASSEL_GROUP_

#include "messageclient.h"

namespace trassel {
	namespace GroupMode {
		enum GroupMode_t {
			FIFO,
			Broadcast 
		};
	}

	class Group : public Task, public ConcreteDirectedChannel {
	public:
		Group(const Group& other);
		Group(Channel<Message, uint8>* channel, GroupMode::GroupMode_t mode);

		void attach(MessageClient* client);
		void detach(MessageClient* client);
		bool isAttached(MessageClient* client);

		void handleMessage(Message msg);
		void quit();

		void push(Message msg);
		Message pop(uint8 id);

		GroupMode::GroupMode_t getMode() { return mMode; }
	private:
		Message popInternal(uint8 id, lock& waitLock);
		std::map<uint8, std::queue<Message> > mClients;
		typedef std::map<uint8, std::queue<Message> >::iterator ClientIt;
		GroupMode::GroupMode_t mMode;
		bool mQuit;
		boost::condition_variable mEmptyCondition;
		typedef boost::unique_lock<boost::mutex> lock;
		boost::mutex mMutex;
	};
}

#endif //_TRASSEL_GROUP_