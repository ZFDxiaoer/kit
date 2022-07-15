#ifndef UTIL_SINGLETON_H
#define UTIL_SINGLETON_H

#include "stddef.h"
#include <mutex>

using namespace std;

template <class TClass>
class CSingleton
{
protected:
	CSingleton();
    ~CSingleton();

private:
    CSingleton(const CSingleton&);
    const CSingleton &operator=(const CSingleton&);

public:

	static TClass * &GetInstance();

private:

    static TClass *m_pInstance;
    static std::mutex m_Mutex;
};

template <class TClass>
TClass *CSingleton<TClass>::m_pInstance = NULL;
template <class TClass>
std::mutex CSingleton<TClass>::m_Mutex;

template <class TClass>
TClass * &CSingleton<TClass>::GetInstance()
{
    if(m_pInstance == NULL)
    {
        std::unique_lock<std::mutex> lock(m_Mutex);
        if(m_pInstance == NULL)
        {
            m_pInstance = new TClass();
        }
    }
        
    return m_pInstance;
}

template <class TClass>
CSingleton<TClass>::CSingleton()
{}

template <class TClass>
CSingleton<TClass>::~CSingleton()
{}

#endif