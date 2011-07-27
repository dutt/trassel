#ifndef _SINGLETON_
#define _SINGLETON_

namespace trassel {
	template<class T>
	class Singleton {
	public:
		inline static T* getInstance() { return mInstance; }
	protected:
		Singleton() {}
		Singleton(const Singleton& rhs);
		static T* mInstance;
	};

	template<class T>
	T* Singleton<T>::mInstance = 0;
}

#endif //_SINGLETON_