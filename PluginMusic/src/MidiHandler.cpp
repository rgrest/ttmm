#include "MidiHandler.h"

using namespace ttmm;

MidiHandler::MidiHandler() {}

MidiHandler::~MidiHandler() {}

void MidiHandler::playNote(Tune tune, bool isOn, int key, Node note,
    juce::MidiBuffer& midiBuffer, int channelNr,
    double& playtime, int numSamples, int newVelocity,
    double secondOfBlock)
{
    juce::MidiMessage m;
    int noteNr = note.getNodeNumber();
    switch (tune)
    {
    case Main:
        break;
    case FirstAccompany:
        noteNr = key - 12;
        if (noteNr < 0)
        {
            noteNr = 127 + noteNr;
        }
        break;
    case SecondAccompany:
        noteNr = key - 5;
        if (noteNr < 0)
        {
            noteNr = 127 + noteNr;
        }
        break;
    default:
        break;
    }
    if (isOn)
    {
        m = juce::MidiMessage::noteOn(channelNr + 1, noteNr, (uint8)newVelocity);
        m.setTimeStamp(note.getTimestamp());
        midiBuffer.addEvent(m, static_cast<int>(((note.getTimestamp() - playtime) / secondOfBlock) * numSamples));
    }
    else
    {
        m = juce::MidiMessage::noteOff(channelNr + 1, noteNr, (uint8)0);
        m.setTimeStamp(note.getTimestamp() + note.getDelta());
        midiBuffer.addEvent(m, static_cast<int>((((note.getTimestamp() + note.getDelta()) - playtime) / secondOfBlock) * numSamples));
    }
}

// Wird von zyklisch aufgerufener Methode processAudioAndMidiSignals verwendet 
// und fügt alle Bestandteile eines TTMM-Tracks, die zeittechnisch im aktuellen 
// Block liegen, in einen juce-Midibuffer ein, der dann von allen angeschlossenen 
// Plugins blockweise ausgelesen und verarbeitet werden kann. 
// Liest außerdem Musikanten Tonart aus und passt Musikstück an
void MidiHandler::processTrackToNewMidiBuffer(
    Track* track, juce::MidiBuffer& newMidiBuffer, double& playtime,
    double secondEachBlock, int numSamples, IPCSongInfo_IPCMusician musician)
{
    if (track == nullptr)
    {
        return;
    }
    juce::MidiMessage m;
    vector<int> nrChannels = track->getChannelNumbers();
    for (int channelNr : nrChannels)
    {
        Channel* channel = track->getChannelp(channelNr);
        // get stroke indexes
        auto strokeIndexes = channel->getStrokeNumbers();
        for (auto index : strokeIndexes)
        {
            Stroke* stroke = channel->getStrokep(index);
            // edit stroke type based on the input values
            if (stroke->getStart() >= (playtime + secondEachBlock))
            {
                break;
            }
            Node nodeOfToneCheck = stroke->getNodeOfTone()[0];
            if ((nodeOfToneCheck.getTimestamp() >= playtime) && (nodeOfToneCheck.getTimestamp() < (playtime + secondEachBlock)))
            {
                // at the start of a new stroke, transpose if necessary
                if (musician.tune() == IPCSongInfo_IPCMusician_Tune::
                                           IPCSongInfo_IPCMusician_Tune_LEFT_UP)
                {
                    stroke->setStrokeType(StrokeType::Subdominante);
                }
                else if (musician.tune() == IPCSongInfo_IPCMusician_Tune::
                                                IPCSongInfo_IPCMusician_Tune_LEFT_DOWN)
                {
                    stroke->setStrokeType(StrokeType::MollParallelSubdominante);
                }
                else if (musician.tune() == IPCSongInfo_IPCMusician_Tune::
                                                IPCSongInfo_IPCMusician_Tune_MIDDLE_UP)
                {
                    stroke->setStrokeType(StrokeType::Tonika);
                }
                else if (musician.tune() == IPCSongInfo_IPCMusician_Tune::
                                                IPCSongInfo_IPCMusician_Tune_MIDDLE_DOWN)
                {
                    stroke->setStrokeType(StrokeType::MollParallelTonika);
                }
                else if (musician.tune() == IPCSongInfo_IPCMusician_Tune::
                                                IPCSongInfo_IPCMusician_Tune_RIGHT_UP)
                {
                    stroke->setStrokeType(StrokeType::Dominante);
                }
                else if (musician.tune() == IPCSongInfo_IPCMusician_Tune::
                                                IPCSongInfo_IPCMusician_Tune_RIGHT_DOWN)
                {
                    stroke->setStrokeType(StrokeType::MollParallelDominante);
                }
            }
            // add Tone to MidiBuffer
            for (auto& nodeOfTone : stroke->getNodeOfTone())
            {
                if (nodeOfTone.getTimestamp() >= (playtime + secondEachBlock))
                {
                    break;
                }
                if ((nodeOfTone.getTimestamp() >= playtime) && (nodeOfTone.getTimestamp() < (playtime + secondEachBlock)))
                {
                    // add nodeOn in channel 2
                    m = juce::MidiMessage::noteOn(5, nodeOfTone.getNodeNumber(), (uint8)nodeOfTone.getLength());
                    m.setTimeStamp(nodeOfTone.getTimestamp());
                    newMidiBuffer.addEvent(m, static_cast<int>(((nodeOfTone.getTimestamp() - playtime) / secondEachBlock) * numSamples));
                }
                if (((nodeOfTone.getTimestamp() + nodeOfTone.getDelta()) >= playtime) && ((nodeOfTone.getTimestamp() + nodeOfTone.getDelta()) < (playtime + secondEachBlock)))
                {
                    // add nodeOff in channel 2
                    m = juce::MidiMessage::noteOff(5, nodeOfTone.getNodeNumber(), (uint8)0);
                    m.setTimeStamp(nodeOfTone.getTimestamp() + nodeOfTone.getDelta());
                    newMidiBuffer.addEvent(juce::MidiMessage::noteOff(
                                               5, nodeOfTone.getNodeNumber(), (uint8)0),
                        static_cast<int>((((nodeOfTone.getTimestamp() + nodeOfTone.getDelta()) - playtime) / secondEachBlock) * numSamples));
                    count++;
                }
            }
            // add node On/Off, Metronom to MidiBuffer
            for (auto& node : stroke->getNodes())
            {
                if (node.getTimestamp() >= (playtime + secondEachBlock))
                {
                    break;
                }
                // add main sound in channel 2, 3, 4
                if ((node.getTimestamp() >= playtime) && (node.getTimestamp() < (playtime + secondEachBlock)))
                {
                    // add nodeOn in channel 2, 3, 4
                    this->playNote(ttmm::Tune::Main, true,
                        stroke->getNodeOfTone()[0].getNodeNumber(), node,
                        newMidiBuffer, 1, playtime, numSamples,
                        musician.volumech1(), secondEachBlock);
                    this->playNote(ttmm::Tune::FirstAccompany, true,
                        stroke->getNodeOfTone()[0].getNodeNumber(), node,
                        newMidiBuffer, 2, playtime, numSamples,
                        musician.volumech2(), secondEachBlock);
                    this->playNote(ttmm::Tune::SecondAccompany, true,
                        stroke->getNodeOfTone()[0].getNodeNumber(), node,
                        newMidiBuffer, 3, playtime, numSamples,
                        musician.volumech3(), secondEachBlock);
                }
                if (((node.getTimestamp() + node.getDelta()) >= playtime) && ((node.getTimestamp() + node.getDelta()) < (playtime + secondEachBlock)))
                {
                    // add nodeOff in channel 2, 3, 4
                    this->playNote(ttmm::Tune::Main, false,
                        stroke->getNodeOfTone()[0].getNodeNumber(), node,
                        newMidiBuffer, 1, playtime, numSamples,
                        musician.volumech1(), secondEachBlock);
                    this->playNote(ttmm::Tune::FirstAccompany, false,
                        stroke->getNodeOfTone()[0].getNodeNumber(), node,
                        newMidiBuffer, 2, playtime, numSamples,
                        musician.volumech2(), secondEachBlock);
                    this->playNote(ttmm::Tune::SecondAccompany, false,
                        stroke->getNodeOfTone()[0].getNodeNumber(), node,
                        newMidiBuffer, 3, playtime, numSamples,
                        musician.volumech3(), secondEachBlock);
                }
                // add metronom, tick for each quarter note
                for (auto start : node.getStartQuarter())
                {
                    if ((start >= playtime) && (start < (playtime + secondEachBlock)))
                    {
                        // add note on metronom with nodenr = 61 in channel 1
                        m = juce::MidiMessage::noteOn(1, 61, uint8(100));
                        m.setTimeStamp(start);
                        newMidiBuffer.addEvent(m, static_cast<int>(((start - playtime) / secondEachBlock) * numSamples));
                    }
                    if (((start + node.getDurationQuarter()) >= playtime) && ((start + node.getDurationQuarter()) < (playtime + secondEachBlock)))
                    {
                        // add note off metronom with nodenr = 61 in channel 1
                        m = juce::MidiMessage::noteOn(1, 61, uint8(0));
                        m.setTimeStamp(start + node.getDurationQuarter());
                        newMidiBuffer.addEvent(m, static_cast<int>((((start + node.getDurationQuarter()) - playtime) / secondEachBlock) * numSamples));
                    }
                }
            }
            // remove this stroke in channel when all noteOff of Tone is added to
            // buffer save the time for the next time
            if (count == 3)
            {
                if (channel->getNumberofStrokes() <= track->getOriginalNumbersOfStroke())
                {
                    //add the new stroke to end of Track
                    //return endtime of track
                    double endTimeofTrack = track->getEnd();
                    double durationNote = track->getSecondPerQuarter();
                    //add new node of tone with the new timestamp
                    Stroke tempStroke;
                    size_t numberOfTone = stroke->getNodeOfTone().size();
                    for (size_t i = 0; i < numberOfTone; i++)
                    {
                        Node nodeTone = stroke->getNodeOfTone()[i];
                        nodeTone.setTimestamp(nodeTone.getTimestamp() + endTimeofTrack);
                        tempStroke.setStart(nodeTone.getTimestamp());
                        tempStroke.setEnd(nodeTone.getTimestamp() + nodeTone.getDelta());
                        tempStroke.setNodeOfTone(nodeTone);
                    }
                    //add new node with the new timestamp
                    auto numberOfNode = stroke->getNodeCount();
                    for (int i = 0; i < numberOfNode; i++)
                    {
                        Node node = stroke->getNode(i);
                        node.setTimestamp(node.getTimestamp() + endTimeofTrack);
                        //reset start time of the quarter notes
                        vector<double> starts;
                        for (int i = 0; i < node.getNoteType(); i++)
                        {
                            starts.push_back(node.getTimestamp() + double(i) * (durationNote));
                        }
                        node.setStartQuarter(starts);
                        //add note
                        tempStroke.addNode(node);
                    }
                    channel->addStroke(channel->getNumberofStrokes() + index, tempStroke);
                }
                //remove the processed stroke
                channel->removeStroke(index);
                count = 0;
            }
        }
    }
    playtime += secondEachBlock;
}
