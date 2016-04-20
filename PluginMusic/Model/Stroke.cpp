#include "Stroke.h"

using namespace ttmm;

Stroke::~Stroke()
{
    this->aftertouches.clear();
    this->nodes.clear();
    this->tone.clear();
}

Stroke::Stroke()
{
    this->nodes.clear();
    this->aftertouches.clear();
}

vector<Node> Stroke::getNodeOfTone()
{
    return this->tone;
}

void Stroke::setNodeOfTone(Node nodeNrOfTone)
{
    this->tone.push_back(nodeNrOfTone);
}

Node Stroke::getNode(int index)
{
    return this->nodes[index];
}

std::vector<Node> Stroke::getNodes()
{
    return this->nodes;
}

int Stroke::getNodeCount()
{
    return this->nodes.size();
}

void Stroke::addNode(Node& node)
{
    this->nodes.push_back(node);
}

Aftertouch Stroke::getAftertouch(int index)
{
    return this->aftertouches[index];
}

std::vector<Aftertouch> Stroke::getAftertouchs()
{
    return this->aftertouches;
}

int Stroke::getAftertouchCount()
{
    return this->aftertouches.size();
}

void Stroke::addAftertouch(Aftertouch& aft)
{
    this->aftertouches.push_back(aft);
}

void Stroke::setStrokeType(StrokeType strokeType)
{
    //tonika =0      subdominante= +5(high) -7(low)   dominante = +7
    int changeNodeNrBy = 0;

    /*-----------------------------------  DUR  ------------------------------------*/
    if (this->strokeType == StrokeType::Tonika || this->strokeType == StrokeType::MollParallelTonika)
    {
        if (strokeType == StrokeType::Dominante || strokeType == StrokeType::MollParallelDominante)
        {
            changeNodeNrBy = 7;
        }
        else if (strokeType == StrokeType::Subdominante || strokeType == StrokeType::MollParallelSubdominante)
        {
            changeNodeNrBy = -7;
        }
    }
    else if (this->strokeType == StrokeType::Dominante || this->strokeType == StrokeType::MollParallelDominante)
    {
        if (strokeType == StrokeType::Subdominante || strokeType == StrokeType::MollParallelSubdominante)
        {
            changeNodeNrBy = -14;
        }
        else if (strokeType == StrokeType::Tonika || strokeType == StrokeType::MollParallelTonika)
        {
            changeNodeNrBy = -7;
        }
    }
    else if (this->strokeType == StrokeType::Subdominante || this->strokeType == StrokeType::MollParallelSubdominante)
    {
        if (strokeType == StrokeType::Dominante || strokeType == StrokeType::MollParallelDominante)
        {
            changeNodeNrBy = 14;
        }
        else if (strokeType == StrokeType::Tonika || strokeType == StrokeType::MollParallelTonika)
        {
            changeNodeNrBy = 7;
        }
    }
    if (changeNodeNrBy != 0)
    {

        for (size_t i = 0; i < this->nodes.size(); i++)
        {
            auto currentNote = &this->nodes[i];
            currentNote->setNodeNumber(currentNote->getNodeNumber() + changeNodeNrBy);
        }
        for (size_t i = 0; i < this->tone.size(); i++)
        {
            auto currentNote = &this->tone[i];
            currentNote->setNodeNumber(currentNote->getNodeNumber() + changeNodeNrBy);
        }
    }
    /*-----------------------------------  MOLL  ------------------------------------*/

    //check if current Stroke is dur or moll
    if (this->strokeType == StrokeType::Tonika || this->strokeType == StrokeType::Dominante || this->strokeType == StrokeType::Subdominante)
    {
        if (strokeType == StrokeType::MollParallelTonika || strokeType == StrokeType::MollParallelDominante || strokeType == StrokeType::MollParallelSubdominante)
        {
            //from dur to moll

            //node
            for (size_t i = 0; i < this->nodes.size(); i++)
            {
                auto currentNote = &this->nodes[i];
                //detection if terz is played -> play small terz instead (-4)
                if (this->tone[1].getNodeNumber() % 12 == currentNote->getNodeNumber() % 12)
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() - 4);
                }
                else
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() - 3);
                }
            }

            //node of tone
            for (size_t i = 0; i < this->tone.size(); i++)
            {
                auto currentNote = &this->tone[i];
                //the second node in tone is terz -> play small terz (-4)
                if (i != 1)
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() - 3);
                }
                else
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() - 4);
                }
            }
        }
    }
    else
    {
        if (strokeType == StrokeType::Tonika || strokeType == StrokeType::Dominante || strokeType == StrokeType::Subdominante)
        {
            //from moll to dur
            for (size_t i = 0; i < this->nodes.size(); i++)
            {
                auto currentNote = &this->nodes[i];
                //detection if terz is played -> play small terz instead (+4)
                if (this->tone[1].getNodeNumber() % 12 == currentNote->getNodeNumber() % 12)
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() + 4);
                }
                else
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() + 3);
                }
            }
            for (size_t i = 0; i < this->tone.size(); i++)
            {
                auto currentNote = &this->tone[i];
                //the second node in tone is terz -> play small terz (+4)
                if (i != 1)
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() + 3);
                }
                else
                {
                    currentNote->setNodeNumber(currentNote->getNodeNumber() + 4);
                }
            }
        }
    }
    this->strokeType = strokeType;
}

double Stroke::getStart()
{
    return this->startStroke;
}

double Stroke::getEnd()
{
    return this->endStroke;
}

void Stroke::setStart(double startStroke)
{
    this->startStroke = startStroke;
}

void Stroke::setEnd(double endStroke)
{
    this->endStroke = endStroke;
}

void Stroke::clear()
{
    this->aftertouches.clear();
    this->nodes.clear();
    this->tone.clear();
    this->startStroke = 0;
    this->endStroke = 0;
}