/**
* @author FH-Minden Musikinformatik 2015, Musicgroup (Marcel, Patrick, Scott)
* @brief Encapsulates a note event and a aftertouch event
*
*/
#ifndef TTMM_AFTERTOUCH_H
#define TTMM_AFTERTOUCH_H

/**
* @class Aftertouch
* @brief Encapsulates a aftertouch event
*
* @see Song, Stroke, Channel
*/
class Aftertouch
{
public:
    int nodeNumber; ///<a node number value (0-127)
    int strength; ///<a pressure of a note (0=127) (use rarely)
    double timestamp; ///<a time position (tick per frame/bpm) of a note
};

#endif
