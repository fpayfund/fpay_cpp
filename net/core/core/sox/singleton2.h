#ifndef _SINGLETON_H
#define _SINGLETON_H

template<class T>
class Singleton
{
protected:
  Singleton(){}
private:
  Singleton(const Singleton& s);
  Singleton& operator = (Singleton& s);
public:
  virtual ~Singleton(){}
  static T* Instance()
  {
    static T _inst;
    return &_inst;
  }
};

#endif
