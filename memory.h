#ifndef MEMORY_H
#define MEMORY_H

template <typename T>
class SmartPointer
{
  public:
    SmartPointer(T* pointer);
    ~SmartPointer();
};

#endif
