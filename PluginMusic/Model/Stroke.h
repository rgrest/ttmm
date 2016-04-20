/**
* @file Stroke.cpp
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a track/ stroke
*
*/
#ifndef TTMM_STROKE_H
#define TTMM_STROKE_H

#include <vector>
#include <map>
#include "Node.h"
#include "Aftertouch.h"

namespace ttmm
{
using std::vector;
using std::map;

/**
	* Enum type for Stroke including the Tonika = 0, Subdominante = -1, Dominante = 1, and the moll parallels
	*/
enum StrokeType
{
    Tonika = 0,
    Subdominante = -1,
    Dominante = 1,
    MollParallelTonika = -3,
    MollParallelSubdominante = -4,
    MollParallelDominante = -2
};
/**
	* @class Stroke
	* @brief Encapsulates a stroke
	*
	* @see Song, Channel, Node
	*/
class Stroke
{
public:
    /**
		* Constructor: create a Stroke
		*/
    Stroke();
    /**
		* Destructor
		*/
    ~Stroke();
    /**
		* Get the List of NodeNr of a Tone-art
		*
		* @return the list of List of NodeNr of a Tone-art
		*/
    vector<Node> getNodeOfTone();
    /**
		* Change the NodeNr of a Tone-art
		*
		* @param nodeNrOfTone NodeNr of a Tone-art
		*/
    void setNodeOfTone(Node nodeNrOfTone);
    /**
		* Get the i-th node in Stroke
		*
		* @param index a integer argument
		* @return the Node object at the index-th position
		*/
    Node getNode(int index);
    /**
		* Get all of Nodes in Stroke
		*
		* @return a list of Node object
		*/
    std::vector<Node> getNodes();
    /**
		* Get the size of Nodes
		*
		* @return a integer value
		*/
    int getNodeCount();
    /**
		* Add a Node to a Stroke
		*
		* @param c a reference to the Node object
		*/
    void addNode(Node& node);
    /**
		* Get the i-th Aftertouch in Stroke
		*
		* @param index a integer argument
		* @return the Aftertouch object at the index-th position
		*/
    Aftertouch getAftertouch(int index);
    /**
		* Get all of Aftertouches in Stroke
		*
		* @return a list of Aftertouches
		*/
    std::vector<Aftertouch> getAftertouchs();
    /**
		* Get the size of Aftertouches
		*
		* @return a integer value
		*/
    int getAftertouchCount();
    /**
		* Add a Aftertouch to a Stroke
		*
		* @param c a reference to the Aftertouch object
		*/
    void addAftertouch(Aftertouch& aft);
    /**
		* set Stroke Type
		*
		* @param strokeType StrokeType the Stroke is transposed to
		*/
    void setStrokeType(StrokeType strokeType);
    /**
		* Get the timestamp of the first event in Stroke
		*
		* @return a timestamp value (tick per frame)
		*/
    double getStart();
    /**
		* Change the timestamp of the first event in Stroke
		*
		* @param startStroke a timestamp value (tick per frame)
		*/
    void setStart(double startStroke);
    /**
		* Get the timestamp of the last event in Stroke
		*
		* @return a timestamp value (tick per frame)
		*/
    double getEnd();
    /**
		* Change the timestamp of the last event in Stroke
		*
		* @param endStroke a timestamp value (tick per frame)
		*/
    void setEnd(double endStroke);
    /**
		* clear the Stroke
		*
		*/
    void clear();
    /**
	*
	*/
private:
    // ich lasse erstmal so denn ich weisst nicht wie man 3 noteNr von TonArt zum ein MusikKey abbildet
    std::vector<Node> tone; ///<a list of tonart
    std::vector<Node> nodes; ///<a list of node events
    std::vector<Aftertouch> aftertouches; ///<a list of aftertouch events
    StrokeType strokeType = ttmm::StrokeType::Tonika; //the !current! StrokeType
    double startStroke = 0; ///<start time of stroke
    double endStroke = 0; ///<end time of stroke
};
}
#endif