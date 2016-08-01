#ifndef SINGLETON_T_H
#define SINGLETON_T_H


#include <iostream>

template <typename T>
class CSingletonT
{
public :
    static bool IsAlive();
    static void Destory();
    static T* Instance();
private :
    CSingletonT(){}
    static T *s_instance;
};

template <typename T>
T* CSingletonT<T>::s_instance = NULL;


template <typename T>
bool CSingletonT<T>::IsAlive()
{
    return s_instance != NULL;
}

template <typename T>
void CSingletonT<T>::Destory()
{
    if (s_instance != NULL) {
        delete s_instance;
        s_instance = NULL;
    }
}

template <typename T>
T* CSingletonT<T>::Instance()
{
    if (s_instance == NULL) {
        s_instance = new T();
    }
    return s_instance;
}

#endif

