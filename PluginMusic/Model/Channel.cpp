#include "Channel.h"
using namespace ttmm;

/*Channel::Channel(int ins)
{
	this->instrument = ins;
	this->nodes.clear();
}*/

Channel::Channel()
{
}

Channel::~Channel()
{
    this->strokes.clear();
}

void Channel::addStroke(int index, Stroke& stroke)
{

    this->strokes.insert(std::pair<int, Stroke>(index, stroke));
}

Stroke* Channel::getStrokep(int index)
{
    return &this->strokes[index];
}
Stroke Channel::getStroke(int index)
{
    return this->strokes[index];
}

map<int, Stroke> Channel::getStrokes()
{
    return this->strokes;
}

int Channel::getNumberofStrokes()
{
    return this->strokes.size();
}

void Channel::addInstrument(int& ins)
{
    this->instrument = ins;
}

int Channel::getInstrument()
{
    return this->instrument;
}

bool Channel::isNotInChannel(int index, Stroke& stroke)
{
    for (auto mymap : this->strokes)
    {
        if (mymap.first == index)
        {
            return false;
        }
    }
    return true;
}

void Channel::editStroke(int index, Stroke& stroke)
{
    for (int i = 0; i < stroke.getAftertouchCount(); i++)
    {
        this->strokes[index].addAftertouch(stroke.getAftertouch(i));
    }
    for (int i = 0; i < stroke.getNodeCount(); i++)
    {
        this->strokes[index].addNode(stroke.getNode(i));
    }
    for (size_t i = 0; i < stroke.getNodeOfTone().size(); i++)
    {
        this->strokes[index].setNodeOfTone(stroke.getNodeOfTone()[i]);
    }
}

void Channel::removeStroke(int index)
{
    this->strokes.erase(index);
}

vector<int> Channel::getStrokeNumbers()
{
    vector<int> listOfStrokeNr;
    for (auto& mymap : this->strokes)
    {
        listOfStrokeNr.push_back(mymap.first);
    }
    return listOfStrokeNr;
}