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
		~Group();
		
		void attach(MessageClient* client);
		void detach(MessageClient* client);
		bool isAttached(MessageClient* client);

		void handleMessage(Message msg);
		void quit();

		Message pop(uint8 id);

		inline GroupMode::GroupMode_t getMode() { return mMode; }
		inline void setMode(GroupMode::GroupMode_t mode) { mMode = mode; }
	private:
		Message popInternal(uint8 id, lock& waitLock);
		std::map<uint8, std::queue<Message> > mClients;
		typedef std::map<uint8, std::queue<Message> >::iterator ClientIt;
		GroupMode::GroupMode_t mMode;
		bool mQuit;
		boost::condition_variable mEmptyCondition;
		boost::mutex mMutex;
	};
}

#endif //_TRASSEL_GROUP_