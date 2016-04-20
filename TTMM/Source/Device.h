#ifndef DEVICE_H
#define DEVICE_H

namespace ttmm
{
template <typename Data>
class Device
{
public:
    using DataType = Data;

    virtual bool isOpen() const = 0;
};
}

#endif