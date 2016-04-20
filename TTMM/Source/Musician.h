/***********************************************************************
* Module:  Timestamp.h
* Author:  Isabell, Tine
* Purpose: Declaration of the Musican (Child)
***********************************************************************/
#pragma once

#include "Buffer.h"
#include "ExceptionTypes.h"

namespace ttmm {

template<typename InputData, typename RingBuffer>
class Musician {
	
public:
  using Buffer = RingBuffer;
  Musician() = default;
  virtual ~Musician() = default;
  Buffer& getHistory() { return history; }
  bool playedCorrect = false;

  using BufferData = typename RingBuffer::data_type;
  
  bool isActive() const { return active; }
  void deactivate() { active = false; }
  void activate() { active = true; }

  void increaseAccuracy() { accuracy++; }
  void decreaseAccuracy() { accuracy--; }
  int getAccuracy() const { return accuracy; }
  float getVolumeFactor() { return volumefactor; };

  void pushEvent(InputData& event) {
	  std::vector<BufferData> events;
	  createNewEventsToAdd(event, events);
	  for (BufferData const &data : events)
	  {
		  (void)history.push(data);
	  }
  }

  Timestamp getLastMatchedNoteTimestamp() const {
      return lastMatchedNoteTimestamp;
  }

  void setLastMatchedNoteTimestamp(Timestamp ts) {
      lastMatchedNoteTimestamp = ts;
  }
  virtual BufferData* pushData(BufferData const& preparedData) {
      return &history.push(preparedData);
  }

protected:
	float volumefactor = 1.0;

private:
	//void createNewEventsToAdd(Vector4 const& ground, std::vector<ttmm::PoseEvent> &events);
    virtual void createNewEventsToAdd(InputData const& event, std::vector<BufferData> &events) {
        throw GeneralException("Whoops, Musician::createNewEventsToAdd was called.");
    }

  bool active = false;
  Buffer history;
  int accuracy = 0;
  Timestamp lastMatchedNoteTimestamp;
};
}
