/**
* @file Node.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a note event and a aftertouch event
*
*/
#ifndef TTMM_NODE_H
#define TTMM_NODE_H

#include <vector>

namespace ttmm
{
using uint8 = unsigned char;
using std::vector;
/**
	* Enum type for Note including the Quarter = 1, Haft = 2, Whole/Full = 4
	*/
enum NoteType
{
    Quarter = 1,
    Whole = 4 * Quarter,
    Haft = 2 * Quarter
};

/**
	* @class Node
	* @brief Encapsulates a note event 
	*
	* @see Song, Stroke, Channel
	*/
class Node
{
public:
    /**
		* Constructor: Create a Node
		*/
    Node();
    //Node(int nodeNumber,int strength,int length);
    /**
		* Destructor
		*/
    ~Node();
    /**
		* Change the node number
		* 
		* @param nodeNumber a integer argument (0-127)
		*/
    void setNodeNumber(int nodeNumber);
    /**
		* Get the node number
		* 
		* @return the integer value of node
		*/
    int getNodeNumber();
    /**
		* Get the Terz of the original node
		*
		* @return the integer value of node
		*/
    int getNodeNumberTerz();
    /**
		* Get the Quinte of the original node
		*
		* @return the integer value of node
		*/
    int getNodeNumberQuinte();
    /**
		* Get the NodeNumber with new octaveup
		*
		* @return the integer value of node
		*/
    int getNodeNumberOctaveUp();
    /**
		* Get the NodeNumber with new octavedown
		*
		* @return the integer value of node
		*/
    int getNodeNumberOctaveDown();
    /**
		* Change the velocity of node
		* 
		* @param length a uint8 argument (0-127)
		*/
    void setLength(uint8 length);
    /**
		* Get the velocity of node
		* 
		* @return the uint8 value of velocity
		*/
    uint8 getLength();
    /**
		* Change the timestamp (tick per frame) of each node event
		* 
		* @param timestamp a double argument
		*/
    void setTimestamp(double timestamp);
    /**
		* Get the timestamp when the node event begins
		* 
		* @return the timestamp of node event
		*/
    double getTimestamp();
    /**
		* Change the duration of a node event for determining how long the node event keeps up
		* 
		* @param delta a double argument (based on tick per frame, not the seconds)
		*/
    void setDelta(double delta);
    /**
		* Get the duration of a node event
		* 
		* @return the duration of a node event
		*/
    double getDelta();
    /**
		* Change the timestamp of each quarter after dividing
		*
		* @param startQuarter a vector integer argument
		*/
    void setStartQuarter(vector<double> startQuarter);
    /**
		* Get the timestamp when the quarter notes begin
		*
		* @return the timestamp of the quarter notes
		*/
    vector<double> getStartQuarter();
    /**
		* Change the duration of a quarter note after dividing
		*
		* @param delta a double argument 
		*/
    void setDurationQuarter(double durQuarter);
    /**
		* Get the duration of a quarter note
		*
		* @return the duration of a quarter note
		*/
    double getDurationQuarter();
    /**
		* set Note type
		*
		* @param noteType type of note
		*/
    void setNoteType(NoteType noteType);
    /**
		* get Note type
		*
		* @return a type of note in Enumtype
		*/
    NoteType getNoteType();

    /**
	*
	*/
private:
    //0-127: Nodes -1: break
    int nodeNumber; ///<a node number value (0-127)
    int nodeNumberTerz; ///<Calculated when nodeNumber is set. Terz of the original node
    int nodeNumberQuinte; ///<Calculated when nodeNumber is set. Quinte of the original node
    int nodeNumberOctaveUp; ///<Calculated the nodeNumber by the increasing of octave
    int nodeNumberOctaveDown; //<Calculated the nodeNumber by the decreasing of octave
    uint8 length; ///<a velocity of a note (0-127)
    double timestamp; ///<a time position (tick per frame/bpm) of a note
    double delta; ///<a duration od a note from on to off
    //=========================================================================================
    NoteType nodeType = ttmm::NoteType::Quarter; ///<by default, type od a note is quarter note
    vector<double> startQuarter; ///<separate starttime of haft/whole note to quarter notes
    double durQuarter; ///<duration of a quarter note after dividing
};
}
#endif