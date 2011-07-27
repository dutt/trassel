#ifndef _THREADPOOL_
#define _THREADPOOL_

#include <boost/thread.hpp>
#include <math.h>
#include "typedefs.h"
#include <queue>

namespace trassel {
	class Task {
	public:
		virtual void operator()() = 0;
	};

	template<class T>
	class ThreadPool {
		class TaskRunner {
		public:
			void operator()() {
				while(!ThreadPool::getQuit()) {
					T* task = ThreadPool::getTask();
					(*task)();
				}
			}
		};
	public:
		~ThreadPool() {
			setQuit(true);
			while(!mThreads.empty()) {
				boost::thread* t = mThreads.front();
				t->join();
				delete t;
				mThreads.pop_front();
			}
		}

		static void schedule(T* task) {
			boost::unique_lock<boost::mutex> lock(mTaskMutex);
			mTasks.push(task);
			lock.unlock();
			mTaskWaitCond.notify_one();
		}

		static void setup(uint16 numThreads) {
			mInstance = new ThreadPool(numThreads);
		}

		static void setup() {
			uint16 numThreads = std::min<uint16>(1, boost::thread::hardware_concurrency());
			setup(numThreads);
		}

		static void setQuit(bool quit) {
			getInstance()->setQuitInternal(quit);
		}

	private:
		ThreadPool<T>* getInstance() { return mInstance; }
		static ThreadPool<T>* mInstance;
		ThreadPool(uint16 numThreads) {
			mQuit = false;
			for(int a = 0; a < numThreads; ++a) {
				mThreads.push_back(new boost::thread(TaskRunner()));
			}
		}

		//quit
		static bool mQuit;
		static boost::mutex mQuitMutex;
		void setQuitInternal(bool quit) {
			boost::unique_lock<boost::mutex> lock(mQuitMutex);
			mQuit = quit;
			lock.unlock();
		}
		static bool getQuit() {
			bool quit = false;
			boost::unique_lock<boost::mutex> lock(mQuitMutex);
			//lock.lock();
			quit = mQuit;
			lock.unlock();
			return quit;
		}

		//task processing
		static boost::mutex mTaskMutex;
		static boost::condition_variable mTaskWaitCond;
		static uint32 getTaskCount() {
			uint32 count = 0;
			count = mTasks.size();
			return count;
		}

		static T* getTask() {
			boost::unique_lock<boost::mutex> lock(mTaskMutex);
			if(getTaskCount() == 0)
				mTaskWaitCond.wait(lock); //when wait returns mTaskLock is locked and we have a task
			T* task = mTasks.front();
			mTasks.pop();
			lock.unlock();
			return task;
		}
		static std::queue<T*> mTasks;
		static std::list<boost::thread*> mThreads;
	};

	//definition of static variables
	template<class T>
	bool ThreadPool<T>::mQuit;
	template<class T>
	boost::mutex ThreadPool<T>::mQuitMutex;
	template<class T>
	std::queue<T*> ThreadPool<T>::mTasks;
	template<class T>
	std::list<boost::thread*> ThreadPool<T>::mThreads;
	template<class T>
	boost::mutex ThreadPool<T>::mTaskMutex;
	template<class T>
	boost::condition_variable ThreadPool<T>::mTaskWaitCond;
	template<class T>
	ThreadPool<T>* ThreadPool<T>::mInstance = 0;

	typedef ThreadPool<Task> TaskPool;
}

#endif //_THREADPOOL_