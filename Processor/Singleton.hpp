#pragma once

#include <memory>

namespace CPU
{
  template <typename T>
  class Singleton {
  protected:
    struct token {};
    Singleton() {}

  public:
    static T& getInstance();
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
  };

  template<typename T>
  T& Singleton<T>::getInstance()
  {
    static const std::unique_ptr<T> instance{ new T { token{} } };
    return *instance;
  }
}
