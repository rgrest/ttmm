/**
* @file Channel.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a channel
*
*/
#ifndef TTMM_CHANNEL_H
#define TTMM_CHANNEL_H

#include <vector>
#include "Stroke.h"

namespace ttmm
{
using std::vector;
/**
	* @class Channel
	* @brief Encapsulates a channel
	*
	* @see Song, Stroke, Node 
	*/
class Channel
{
public:
    /**
		* Constructor: create a Channel
		*/
    Channel();
    /**
		* Destructor
		*/
    ~Channel();
    /**
		* Add a stroke event to the channnel
		* 
		* @param node a reference to a stroke object
		*/
    void addStroke(int index, Stroke& stroke);
    /**
		* Get the i-th stroke object
		* 
		* @param index a integer argument
		* @return the stroke object at the index-th position
		*/
    Stroke getStroke(int index);
    /**
		* Get a pointer of the i-th stroke object
		*
		* @param index a integer argument
		* @return the pointer of the stroke object at the index-th position
		*/
    Stroke* getStrokep(int index);
    /**
		* Get all of strokes in the channel
		* 
		* @return the list of stroke objects
		*/
    map<int, Stroke> getStrokes();
    /**
		* Get the size of a list of stroke objects
		* 
		* @return number of Strokes
		*/
    int getNumberofStrokes();
    /**
		* Check whether or not the pair (stroke index, stroke object) in Channel exists
		*
		* @param index a stroke index
		* @param stroke a reference to the stroke object
		* @return true if not in channel, otherwise false
		*/
    bool isNotInChannel(int index, Stroke& stroke);
    /**
		* Add the members of a stroke object to a Stroke which exists in Channel
		*
		* @param index a channel number
		* @param stroke a reference to the stroke object
		*/
    void editStroke(int index, Stroke& stroke);
    /**
		* remove a stroke object in Channel
		*
		* @param index a channel number
		*/
    void removeStroke(int index);
    /**
		* Get the list of stroke indexes in channel
		*
		* @return stroke indexes in a integer list
		*/
    vector<int> getStrokeNumbers();
    /**
		* Add a instrument to the channel
		* 
		* @param ins a reference to a integer argument
		*/
    void addInstrument(int& ins);
    /**
		* Get the instrument of a channel
		* 
		* @return the number value (0-127) responding to the list of instrument in the midi format
		*/
    int getInstrument();

    /**
	*
	*/
private:
    map<int, Stroke> strokes; ///<a list of stroke events
    int instrument = 0; ///<a instrument of a channel (0-127)
};
}
#endif