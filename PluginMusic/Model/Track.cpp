#include "Track.h"

using namespace ttmm;

Track::Track(int ks, int mm, int numerator, int denominator, double tl, double spqn)
{
    this->KeySignature = ks;
    this->minorOrmajor = mm;
    this->numerator = numerator;
    this->denominator = denominator;
    this->ticklength = tl;
    this->secondperquarter = spqn;
    this->channels.clear();
}

Track::~Track()
{
    this->channels.clear();
}

void Track::addChannel(int key, Channel& c)
{
    this->channels.insert(std::pair<int, Channel>(key, c));
}

bool Track::isNotInTrack(int key, Channel& c)
{
    for (auto mymap : this->channels)
    {
        if (mymap.first == key)
        {
            return false;
        }
    }
    return true;
}

void Track::editChannel(int key, int indexofStroke, Stroke& stroke)
{
    if (!this->channels[key].isNotInChannel(indexofStroke, stroke))
    {
        this->channels[key].editStroke(indexofStroke, stroke);
    }
    else
    {
        this->channels[key].addStroke(indexofStroke, stroke);
    }
}

Channel Track::getChannel(int key)
{
    //return this->channels.at(index);
    return this->channels[key];
}
Channel* Track::getChannelp(int key)
{
    //return this->channels.at(index);
    return &this->channels[key];
}

std::map<int, Channel> Track::getChannels()
{
    //return this->channels.at(index);
    return this->channels;
}

int Track::getChannelCount()
{
    return this->channels.size();
}

double Track::getTicklength()
{
    return this->ticklength;
}

void Track::setTicklength(double tl)
{
    this->ticklength = tl;
}

double Track::getSecondPerQuarter()
{
    return this->secondperquarter;
}

void Track::setSecondPerQuarter(double spqn)
{
    this->secondperquarter = spqn;
}

int Track::getNumerator()
{
    return this->numerator;
}

void Track::setNumerator(int nm)
{
    this->numerator = nm;
}

int Track::getDenominator()
{
    return this->denominator;
}

void Track::setDenominator(int dn)
{
    this->denominator = dn;
}

int Track::getKeySignature()
{
    return this->KeySignature;
}

void Track::setKeySignature(int kS)
{
    this->KeySignature = kS;
}

int Track::getMinorOrMajor()
{
    return this->minorOrmajor;
}

void Track::setMinorOrMajor(int mm)
{
    this->minorOrmajor = mm;
}

vector<int> Track::getChannelNumbers()
{
    vector<int> listOfChannelNr;
    for (auto& mymap : this->channels)
    {
        listOfChannelNr.push_back(mymap.first);
    }
    return listOfChannelNr;
}

int Track::getNumberofChannels()
{
    return this->channels.size();
}

double Track::getStart()
{
    return this->startTrack;
}

double Track::getEnd()
{
    return this->endTrack;
}

void Track::setStart(double startTrack)
{
    this->startTrack = startTrack;
}

void Track::setEnd(double endTrack)
{
    this->endTrack = endTrack;
}

double Track::getDurationOfTrack()
{
    return this->endTrack - this->startTrack;
}

int Track::getOriginalNumbersOfStroke()
{
    return this->originalNumbersOfStroke;
}

void Track::setOriginalNumbersOfStroke(int numbers)
{
    this->originalNumbersOfStroke = numbers;
}