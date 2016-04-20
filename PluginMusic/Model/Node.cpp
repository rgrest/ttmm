#include "Node.h"

using namespace ttmm;

#define TERZ 4
#define QUINTE 7 //Midicalculations (actually it's 3 and 5, but not here)
#define OCTAVE 12 // 12 == Octave in this case

Node::Node()
{
}

Node::~Node()
{
}

void Node::setNodeNumber(int nodeNumber)
{
    this->nodeNumber = nodeNumber;

    this->nodeNumberTerz = nodeNumber + TERZ;
    if (this->nodeNumberTerz > 127)
    {
        this->nodeNumberTerz = 0 + (this->nodeNumberTerz - 127);
    }

    this->nodeNumberQuinte = nodeNumber + QUINTE;
    if (this->nodeNumberQuinte > 127)
    {
        this->nodeNumberQuinte = 0 + (this->nodeNumberQuinte - 127);
    }

    this->nodeNumberOctaveUp = nodeNumber + OCTAVE;
    if (this->nodeNumberOctaveUp > 127)
    {
        this->nodeNumberOctaveUp = 0 + (this->nodeNumberOctaveUp - 127);
    }

    this->nodeNumberOctaveDown = nodeNumber - OCTAVE;
    if (this->nodeNumberOctaveDown < 0)
    {
        this->nodeNumberOctaveDown = 127 + this->nodeNumberOctaveDown;
    }
}
void Node::setLength(uint8 length)
{
    this->length = length;
}
void Node::setTimestamp(double ts)
{
    this->timestamp = ts;
}
void Node::setDelta(double delta)
{
    this->delta = delta;
}

void Node::setStartQuarter(vector<double> startQuarter)
{
    this->startQuarter = startQuarter;
}
void Node::setDurationQuarter(double durQuarter)
{
    this->durQuarter = durQuarter;
}

int Node::getNodeNumber()
{
    return this->nodeNumber;
}

int Node::getNodeNumberTerz()
{
    return this->nodeNumberTerz;
}

int Node::getNodeNumberQuinte()
{
    return this->nodeNumberQuinte;
}

int Node::getNodeNumberOctaveUp()
{
    return this->nodeNumberOctaveUp;
}

int Node::getNodeNumberOctaveDown()
{
    return this->nodeNumberOctaveDown;
}

uint8 Node::getLength()
{
    return this->length;
}
double Node::getTimestamp()
{
    return this->timestamp;
}
double Node::getDelta()
{
    return this->delta;
}

vector<double> Node::getStartQuarter()
{
    return this->startQuarter;
}
double Node::getDurationQuarter()
{
    return this->durQuarter;
}

void Node::setNoteType(NoteType nt)
{
    this->nodeType = nt;
}

NoteType Node::getNoteType()
{
    return this->nodeType;
}