/**
* @file Track.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a track/ stroke
*
*/
#ifndef TTMM_TRACK_H
#define TTMM_TRACK_H

#include <vector>
#include <map>
#include "Channel.h"

namespace ttmm
{
using std::vector;
using std::map;
/**
	* @class Track
	* @brief Encapsulates a track
	*
	* @see Song, Channel, Node
	*/
class Track
{
public:
    /**
		* Constructor: create a Stroke/ Track with KeySignature (key, major or minor) and Type (numerator/denominator, e.g: 3/4 or 4/4)
		* @param ks a key argument (-7 to +7, 0 is C)
		* @param mm a major/minor argument (0 is major, 1 is minor
		* @param numerator a numerator argument
		* @param denominator a denominator argument
		* @param tl tick length of note
		* @param spqn second per quarter note
		*/
    Track(int ks, int mm, int numerator, int denominator, double tl, double spqn);
    /**
		* Destructor
		*/
    ~Track();
    /**
		* Get the Ticklength of the track
		*
		* @return the double value of Ticklength
		*/
    double getTicklength();
    /**
		* Change the Ticklength of a track
		*
		* @param tl a double argument
		*/
    void setTicklength(double tl);
    /**
		* Get the SecondPerQuarter of the track
		*					
		* @return the double value of SecondPerQuarter
		*/
    double getSecondPerQuarter();
    /**
		* Change the SecondPerQuarter of a track
		*
		* @param spqn a double argument					
		*/
    void setSecondPerQuarter(double spqn);
    /**
		* Get the numerator of the track
		* 
		* @return the integer value of numerator
		*/
    int getNumerator();
    /**
		* Change the numerator of a track
		* 
		* @param nm a integer argument
		*/
    void setNumerator(int nm);
    /**
		* Get the denominator of the track
		*
		* @return the integer value of denominator
		*/
    int getDenominator();
    /**
		* Change the denominator of a track
		*
		* @param dn a integer argument
		*/
    void setDenominator(int dn);
    /**
		* Get the key of the track
		*
		* @return the integer value of key (-7 to +7, 0 is C)
		*/
    int getKeySignature();
    /**
		* Change the key of a track
		*
		* @param kS a integer argument (-7 to +7, 0 is C)
		*/
    void setKeySignature(int kS);
    /**
		* Get the major/minor of the track
		*
		* @return the integer value of major/minor (0 is major, 1 is minor)
		*/
    int getMinorOrMajor();
    /**
		* Change the major/minor of a track
		*
		* @param mm a integer argument (0 or 1)
		*/
    void setMinorOrMajor(int mm);
    /**
		* Get the i-th channel in Track
		* 
		* @param index a integer argument
		* @return the channel object at the index-th position
		*/
    Channel getChannel(int index);
    /**
		* Get the pointer of the i-th channel in Track
		*
		* @param index a integer argument
		* @return the pointer of the channel object at the index-th position
		*/
    Channel* getChannelp(int index);
    /**
		* Get all of channels in Track
		* 
		* @return a map including (Channel number, Channel object respectively)
		*/
    std::map<int, Channel> getChannels();
    /**
		* Get the size of a map of channel objects
		* 
		* @return a integer value
		*/
    int getChannelCount();
    /**
		* Add a channel to a track
		* 
		* @param key a channel number
		* @param c a reference to the channel object
		*/
    void addChannel(int key, Channel& c);
    /**
		* Check whether or not the pair (channel number, channel object) in Track exists
		* 
		* @param key a channel number
		* @param c a reference to the channel object
		* @return true if not in track, otherwise false
		*/
    bool isNotInTrack(int key, Channel& c);
    /**
		* Add a stroke object to a channel which exists in Track
		* 
		* @param key a channel number
		* @param indexofStroke a Stroke index
		* @param stroke a reference to the stroke object
		*/
    void editChannel(int key, int indexofStroke, Stroke& stroke);
    /**
		* Get the list of channels in Track 
		*
		* @return channel numbers in a integer list
		*/
    vector<int> getChannelNumbers();
    /**
		* Get the size of a list of channel objects
		*
		* @return a integer value of num channels
		*/
    int getNumberofChannels();
    //setter for Node == Private!!!!111elf
    /**
		* Get the timestamp of the first event in Track
		* 
		* @return a timestamp value (tick per frame)
		*/
    double getStart();
    /**
		* Change the timestamp of the first event in Track
		*
		* @param startTrack a timestamp value (tick per frame)
		*/
    void setStart(double startTrack);
    /**
		* Get the timestamp of the last event in Track
		*
		* @return a timestamp value (tick per frame)
		*/
    double getEnd();
    /**
		* Change the timestamp of the last event in Track
		*
		* @param endTrack a timestamp value (tick per frame)
		*/
    void setEnd(double endTrack);
    /**
		* Get the duration of a track
		*
		* @return duration of a track with a integer value
		*/
    double getDurationOfTrack();

    int getOriginalNumbersOfStroke();

    void setOriginalNumbersOfStroke(int numbers);
    /**
	*
	*/
private:
    int KeySignature; ///<a key value
    int minorOrmajor; ///<a major or minor value
    int numerator; ///<a numerator value
    int denominator; ///<a denominator value
    double ticklength;
    double secondperquarter; ///<number of seconds for each quarter node

    std::map<int, Channel> channels; ///<a map object of (channel number, node/aftertouch events)
    double startTrack = 0; ///<time position of the first event
    double endTrack = 0; ///<time postion od the end event
    int originalNumbersOfStroke = 0;
};
}
#endif