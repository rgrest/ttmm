/**
* @file Song.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a song
*
*/
#ifndef TTMM_SONG_H
#define TTMM_SONG_H

#include <vector>
#include "Track.h"
#include "../../TTMM/JuceLibraryCode/JuceHeader.h"
#include <fstream>
#include <string>

namespace ttmm
{
using std::vector;
/**
	* @class Song
	* @brief Encapsulates a song
	*
	* @see Stroke, Channel, Node
	*/
class Song
{
public:
    /**
		* Constructor: create a Song
		* 
		* @param bpm a integer argument (bit per minute)
		*/
    Song(short bpm);
    /**
		* Destructor
		*/
    ~Song();
    /**
		* Add a track to the song
		* 
		* @param stroke a reference to stroke object
		*/
    void addTrack(Track& track);
    /**
		* Get the i-th track object
		*
		* @param index a integer argument
		* @return the track object at the index-th position
		*/
    Track getTrack(int index);
    /**
		* Get the pointer of the i-th track object
		*
		* @param index a integer argument
		* @return the pointer of the track object at the index-th position
		*/
    Track* getTrackp(int index);
    /**
		* Get all of tracks in the song
		*
		* @return the list of track objects
		*/
    vector<Track> getTracks();
    /**
		* Get the size of a list of track objects and put into the Num-reference
		*
		* @return a integer value
		*/
    int getNumberofTracks();
    /**
		* Add the bit per minute to song
		* 
		* @param bpm a const reference to a short value
		*/
    void addBPM(short const& bpm);
    /**
		* Get the tempo of song
		* 
		* @return a short value (e.g 120 beats per minute)
		*/
    short getBpm();
    /**
		* Read all of tracks in song to a data
		* 
		*/
    void readAllTracks();
    /**
		* Read the i-th track to a data
		* 
		* @param index a integer argument
		*/
    void readTrack(int index);
    /**
		* generate the i-th track
		*
		* @param index a integer argument
		*/
    void generateTrack(int index);

    /**
	*
	*/
private:
    vector<Track> tracks; ///<a list of stroke
    short bpm; ///<a beats per minute value
};
}

#endif