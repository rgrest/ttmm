//*******************************
// Buffer.cpp Version 1.0					*
// Filename: Buffer.cpp	    	*
// Class-Definitions    		*
// Holger Heeren  11.05.2015	*
//*******************************

#ifndef BUFFER
#define BUFFER

#include <array>
#include <vector>
#include <chrono>

#include "Types.h"
#include "TimeTools.h"

namespace ttmm
{

/**
 @class BufferData
 @brief Base class of data for the @code RingBuffer.

 Deriving the data type for the buffer from this class
 guarantees that each data entry has a correct timestamp
 in the unique format of TTMM.
 */
struct BufferData
{
    Timestamp timestamp; ///< Time of construction of this data unit
    
    /**
     Default c'tor.
    */
    BufferData()
        : timestamp(TimeInfo::timeInfo().now())
    {
    }
};

/**
 @class Ringbuffer
 @brief Template parameterized on the type of data stored and its size.
*/
template <typename BufferDataType, size_t BufferSize>
class RingBuffer
{
private:
    using Buffer = std::array<BufferDataType, BufferSize>;

public:
    using size_type = typename Buffer::size_type;
    using data_type = typename BufferDataType;

    RingBuffer() {}
    ~RingBuffer() = default;
    RingBuffer(const RingBuffer& rhs) = default;

    /**
     @return true if the buffer is empty.
     */
    bool isEmpty() const { return currentBufferSize == 0; }

    /**
     Return the content of the ringbuffer.
   
     As a sideeffect, the ringbuffer will be cleared.

     @param[out] out The vector is guaranteed to contain every
     element currently in
     the ringbuffer or be cleared if no element is available.
    */
    void popAll(std::vector<BufferDataType>& out);

    /**
     Return a copy of the content of the ringbuffer.

     @param[out] out The vector is guaranteed to contain every
     element currently in
     the ringbuffer or be cleared if no element is available.
    */
    void getCopyOfAll(std::vector<BufferDataType>& out) const;


    /**
    Return a copy of the content of the ringbuffer reversed.

    @param[out] out The vector is guaranteed to contain every
    element currently in
    the ringbuffer or be cleared if no element is available.
    */
    void getReverseCopyOfAll(std::vector<BufferDataType>& out) const;

    /**
     Adds a single element to the buffer.
     Since this is a ringbuffer, this method might overwrite elements.
     @param[in] data The data to store in the buffer.
     @return A ref to the element inserted.
    */
    BufferDataType& push(BufferDataType const& data);

    /**
      Return a pointer the last event added to the buffer.
     */
    virtual BufferDataType const* getLatestEvent() const;
    // BufferDataType const* getLatestEventBefore(Timestamp timestamp) const;

    /**
     Returns the current size of the buffer which is <= BufferSize.
     @return currentSize
    */
    size_type currentSize() const { return currentBufferSize; }
    
    /**
     Returns true once if a new event has been added.
     @return changed.
    */
    bool hasChanged() { 
        return changed;
    }

    void setHasChanged(bool state) {
        changed = state;
    }
protected:
    Buffer buffer; ///< Ringbuffer
    size_t insertPosition = 0; ///< Insert position in the ring buffer
    size_t currentBufferSize = 0; ///< Current number of elements stored

    Timestamp currentTimestamp{};
private:
    bool changed = false;
};
}

template <typename BufferDataType, size_t BufferSize>
BufferDataType& ttmm::RingBuffer<BufferDataType, BufferSize>::push(
    BufferDataType const& data)
{
    changed = true;
    buffer[insertPosition] = data;

    currentTimestamp = data.timestamp;

    insertPosition = (insertPosition + 1) % BufferSize; // ringbuffer
    currentBufferSize = std::min(BufferSize, currentBufferSize + 1); // can't get larger

    return buffer[insertPosition];
}

template <typename BufferDataType, size_t BufferSize>
BufferDataType const*
ttmm::RingBuffer<BufferDataType, BufferSize>::getLatestEvent() const
{
    if (!isEmpty())
    {
        auto lastElement = insertPosition - 1;
        if (insertPosition == 0)
        {
            lastElement = BufferSize - 1;
        }
        return &buffer[lastElement];
    }
    return nullptr;
}


// \todo: Do some testing to verify that the following behaves correctly in all
// cases
template <typename BufferDataType, size_t BufferSize>
void ttmm::RingBuffer<BufferDataType, BufferSize>::popAll(
    std::vector<BufferDataType>& output)
{

    // return empty buffer if no data available
    output.clear();

    // Get the current oldest element
    auto oldest = (currentBufferSize == BufferSize ? (insertPosition + 1) % BufferSize : 0);

    for (Buffer::size_type i = 0; i < currentBufferSize; ++i)
    {
        output.push_back(buffer[(oldest + i) % BufferSize]);
    }

    insertPosition = 0;
    currentBufferSize = 0;
}

template <typename BufferDataType, size_t BufferSize>
void ttmm::RingBuffer<BufferDataType, BufferSize>::getCopyOfAll(
    std::vector<BufferDataType>& output) const
{

    // return empty buffer if no data available
    output.clear();

    // Get the current oldest element
    auto oldest = (currentBufferSize == BufferSize ? (insertPosition + 1) % BufferSize : 0);

    for (Buffer::size_type i = 0; i < currentBufferSize; ++i)
    {
        output.push_back(buffer[(oldest + i) % BufferSize]);
    }
}

template <typename BufferDataType, size_t BufferSize>
void ttmm::RingBuffer<BufferDataType, BufferSize>::getReverseCopyOfAll(
    std::vector<BufferDataType>& output) const
{

    getCopyOfAll(output);
    std::reverse(std::begin(output), std::end(output));
}

#endif
