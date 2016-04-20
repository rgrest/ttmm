#include "MidiReader.h"

using namespace ttmm;
using std::string;

string ExePath()
{
    char buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    string::size_type pos = string(buffer).find_last_of("\\/");
    return string(buffer).substr(0, pos);
}

#define songpath ExePath() + "\\..\\PluginMusic\\Soundfiles\\"
MidiReader::MidiReader(string songname)
{
    this->songname = songname;
}
MidiReader::MidiReader()
{
    ifstream inFile;
    inFile.open(ExePath() + "\\selectedsong.sng");
    std::stringstream strStream;
    strStream << inFile.rdbuf();
    this->songname = strStream.str();
}

MidiReader::~MidiReader()
{
    this->fileMidi.clear();
    if (file)
    {
        delete file;
    }
}

//read our mididata to Song structur
String MidiReader::readToSong(Song& tempSong)
{
    file = new juce::File(songpath + this->songname);
    juce::FileInputStream finput(*file);
    if (finput.failedToOpen())
    { 
        if (file)
        {
            delete file;
        }
    }
    else
    {
        fileMidi.readFrom(finput); // note the *
        //tempSong.addBPM(fileMidi.getTimeFormat());
        tempSong.addBPM(120);
        fileMidi.convertTimestampTicksToSeconds();
        int tracks = fileMidi.getNumTracks();
        //ttmm::logfileMidiReader->write("No of Tracks: ", tracks);
        int kS = 0, mm = 0, nm = 0, dn = 0;
        double ticklength, secondperqn; //second per quarternote based on musicfile
        //second per quarternote based on BPM and Takttyp
        double secondperqnFromBPM;
        juce::MidiMessageSequence const* seq;
        juce::MidiMessage tempMessage;
        if (tracks == 1) //type Midi = 0
        {
            seq = fileMidi.getTrack(0); //get all events of a Track
            readSignatureInfos(seq, kS, mm, nm, dn, ticklength, secondperqn, fileMidi.getTimeFormat());
            secondperqnFromBPM = (minuteToSeconds / double(120)) * (double(dn) / double(nm));
            Track track(kS, mm, nm, dn, ticklength, secondperqnFromBPM);
            //ttmm::logfileMidiReader->write("reading file ...");
            convertToTrack(*seq, track, secondperqn);
            //ttmm::logfileMidiReader->write("add track to song structure");
            tempSong.addTrack(track);
        }
        else //type Midi = 1
        {
            seq = fileMidi.getTrack(0); //get the Track[0]
            //read the KeySignature, TimeSignatur, TempoSignatur to kS, mm, nm, dn, ticklength, secondperqn
            readSignatureInfos(seq, kS, mm, nm, dn, ticklength, secondperqn, fileMidi.getTimeFormat()); //get configure informations of the Track[0]
            secondperqnFromBPM = (minuteToSeconds / fileMidi.getTimeFormat()) * (double(dn) / double(nm));
            for (int i = 1; i < tracks; i++)
            {
                //ttmm::logfileMidiReader->write("reading track ", i);
                seq = fileMidi.getTrack(i); //get all events of each Track
                Track track(kS, mm, nm, dn, ticklength, secondperqnFromBPM);
                convertToTrack(*seq, track, secondperqn);
                //ttmm::logfileMidiReader->write("add track to song structure");
                tempSong.addTrack(track);
            }
            //ttmm::logfileMidiReader->write("succeeded!!!");
            //ttmm::logfileMidiReader->write("");
            //ttmm::logfileMidiReader->write("");
        }
    }
	return songname;
}

//convert Message events in a Track to Channel object
void MidiReader::convertToTrack(juce::MidiMessageSequence const& seq, Track& track, double secondperqn)
{
    track.setStart(start - tolerantOfTone);
    std::cout << "seconds per quarter from Musicfile: " << secondperqn << std::endl;
    // for keeping state of the note On -> Off
    double** state = new double*[16];
    double** state2 = new double*[16];
    for (int i = 0; i < 16; ++i)
    {
        state[i] = new double[128];
        state2[i] = new double[128];
        for (int j = 0; j < 128; ++j)
        {
            state[i][j] = -1;
            state2[i][j] = -1;
        }
    }
    // for keeping state of the Tone (Tonart)
    double** stateTone = new double*[16];
    double** stateTone2 = new double*[16];
    for (int i = 0; i < 16; ++i)
    {
        stateTone[i] = new double[128];
        stateTone2[i] = new double[128];
        for (int j = 0; j < 128; ++j)
        {
            stateTone[i][j] = -1;
            stateTone2[i][j] = -1;
        }
    }
    Channel tempChannel;
    //node1ofTone, node2ofTone, node3ofTone keep the three Node objects for Tone
    //tempNode keeps the Node object for node on/off events
    Node tempNode, node1ofTone, node2ofTone, node3ofTone;
    Aftertouch tempAftertouch;
    //tempStroke keeps the node events
    Stroke tempStroke;
    int channelNr;
    //nodeNr1, nodeNr2, nodeNr3 keep the nodeNr for Nodes of Tone
    //nodeNr keeps the NodeNr for node on/off events
    int nodeNr, nodeNr1, nodeNr2, nodeNr3;
    int velocity;
    double secondperqnFromBPM = track.getSecondPerQuarter();
    std::cout << "seconds per quarter from BPM: " << secondperqnFromBPM << std::endl;
    //this keeps the index for stroke
    //update it after a Tone (Tonart) is added
    int index = 0;
    int i;
    for (i = 0; i < seq.getNumEvents(); i++)
    {
        juce::MidiMessage m = seq.getEventPointer(i)->message;
        //get 2 next messages for testing, whether or not they are Tone (tonart)
        juce::MidiMessage m2, m3;
        if ((i + 2) < seq.getNumEvents())
        {
            m2 = seq.getEventPointer(i + 1)->message;
            m3 = seq.getEventPointer(i + 2)->message;
        }
        /*
        if (m.isAftertouch())
        {
            channelNr = m.getChannel() - 1;
            tempAftertouch.nodeNumber = m.getNoteNumber();
            tempAftertouch.strength = m.getAfterTouchValue();
            tempAftertouch.timestamp = m.getTimeStamp();
            //add aftertouch to Stroke
            //Stroke tempStroke;
            tempStroke.addAftertouch(tempAftertouch);
            //ttmm::logfileMidiReader->write("Add Aftertouch to Channel");
        }*/
        if (m.isNoteOn())
        {
            channelNr = m.getChannel() - 1;
            nodeNr = m.getNoteNumber();
            velocity = m.getVelocity();
            if (velocity == 0)
            {
                //check whether or not m is NodeOn or m is a one of NodeNr of Tone
                if ((m.getTimeStamp() == m2.getTimeStamp()) && (m.getTimeStamp() == m3.getTimeStamp()))
                {
                    // notes off of Tone (Tonart)
                    channelNr = m.getChannel() - 1;
                    nodeNr1 = m.getNoteNumber();
                    nodeNr2 = m2.getNoteNumber();
                    nodeNr3 = m3.getNoteNumber();
                    //check whether or not a nodeOn of Tone with the same node number
                    //on the same channel is added
                    if (stateTone[channelNr][nodeNr1] == -1 || stateTone[channelNr][nodeNr2] == -1
                        || stateTone[channelNr][nodeNr3] == -1)
                    {
                        continue;
                    }
                    //add start/end time of stroke
                    tempStroke.setStart(stateTone2[channelNr][nodeNr1]);
                    tempStroke.setEnd(start - tolerantOfTone);
                    //add 3 nodeNr of Tone to Stroke for storing
                    node1ofTone.setTimestamp(stateTone2[channelNr][nodeNr1]);
                    node1ofTone.setDelta(start - tolerantOfTone - stateTone2[channelNr][nodeNr1]);
                    node1ofTone.setNodeNumber(nodeNr1);
                    //add node to Stroke
                    tempStroke.setNodeOfTone(node1ofTone);
                    //
                    node2ofTone.setTimestamp(stateTone2[channelNr][nodeNr2]);
                    node2ofTone.setDelta(start - tolerantOfTone - stateTone2[channelNr][nodeNr2]);
                    node2ofTone.setNodeNumber(nodeNr2);
                    //add node to Stroke
                    tempStroke.setNodeOfTone(node2ofTone);
                    //
                    node3ofTone.setTimestamp(stateTone2[channelNr][nodeNr3]);
                    node3ofTone.setDelta(start - tolerantOfTone - stateTone2[channelNr][nodeNr3]);
                    node3ofTone.setNodeNumber(nodeNr3);
                    //add node to Stroke
                    tempStroke.setNodeOfTone(node3ofTone);
                    if (!track.isNotInTrack(channelNr, tempChannel))
                    {
                        track.editChannel(channelNr, index, tempStroke);
                    }
                    else
                    {
                        tempChannel.addStroke(index, tempStroke);
                        track.addChannel(channelNr, tempChannel);
                    }
                    //reset the tempstroke
                    tempStroke.clear();
                    //set state of channelNr and nodeNr1/2/3 to -1 again
                    stateTone[channelNr][nodeNr1] = -1;
                    stateTone[channelNr][nodeNr2] = -1;
                    stateTone[channelNr][nodeNr3] = -1;
                    stateTone2[channelNr][nodeNr1] = -1;
                    stateTone2[channelNr][nodeNr2] = -1;
                    stateTone2[channelNr][nodeNr3] = -1;
                    //ttmm::logfileMidiReader->write("Added Nodes of Tone to Channel, Stroke");
                    //jump i and pass the three nodeNr of Tone
                    i = i + 2;
                    //update index of stroke
                    index++;
                }
                else
                {
                    // note off
                    if (state[channelNr][nodeNr] == -1)
                    {
                        continue;
                    }
                    //init a node
                    double duration = m.getTimeStamp() - state[channelNr][nodeNr];
                    tempNode.setTimestamp(state2[channelNr][nodeNr]);
                    tempNode.setNodeNumber(nodeNr);
                    //check type of note and divide to the quarter notes
                    if ((duration >= (secondperqn - 0.1))
                        && (duration <= (secondperqn + 0.1))) //is a quarter note
                    {
                        tempNode.setNoteType(ttmm::NoteType::Quarter);
                        //add timestamp and duration of a quarter note
                        vector<double> startQuarter;
                        for (int i = 0; i < 1; i++)
                        {
                            startQuarter.push_back(state2[channelNr][nodeNr] + double(i) * (secondperqnFromBPM));
                        }
                        tempNode.setStartQuarter(startQuarter);
                        tempNode.setDurationQuarter(secondperqnFromBPM - tolerantOfNote);
                        tempNode.setDelta((double(1) * secondperqnFromBPM) - tolerantOfNote);
                        start += secondperqnFromBPM;
                    }
                    else if ((duration >= ((2 * secondperqn) - 0.1))
                        && (duration <= ((2 * secondperqn) + 0.1))) //is a haft note?
                    {
                        tempNode.setNoteType(ttmm::NoteType::Haft);
                        //divide the haft note to 2 quarter notes with the same duration
                        vector<double> startQuarter;
                        for (int i = 0; i < 2; i++)
                        {
                            startQuarter.push_back(state2[channelNr][nodeNr] + double(i) * (secondperqnFromBPM));
                        }
                        tempNode.setStartQuarter(startQuarter);
                        tempNode.setDurationQuarter(secondperqnFromBPM - tolerantOfNote);
                        tempNode.setDelta((double(2) * secondperqnFromBPM) - tolerantOfNote);
                        start += double(2) * secondperqnFromBPM;
                    }
                    else if ((duration >= ((4 * secondperqn) - 0.1))
                        && (duration <= ((4 * secondperqn) + 0.1))) //is a whole note
                    {
                        tempNode.setNoteType(ttmm::NoteType::Whole);
                        //divide the whole note to 4 quarter notes with the same duration
                        vector<double> startQuarter;
                        for (int i = 0; i < 4; i++)
                        {
                            startQuarter.push_back(state2[channelNr][nodeNr] + double(i) * (secondperqnFromBPM));
                        }
                        tempNode.setStartQuarter(startQuarter);
                        tempNode.setDurationQuarter(secondperqnFromBPM - tolerantOfNote);
                        tempNode.setDelta((double(4) * secondperqnFromBPM) - tolerantOfNote);
                        start += double(4) * secondperqnFromBPM;
                    }
                    //add node to Stroke
                    tempStroke.addNode(tempNode);
                    //set state of channelNr and nodeNr to -1 again
                    state[channelNr][nodeNr] = -1;
                    state2[channelNr][nodeNr] = -1;
                    //ttmm::logfileMidiReader->write("Added Node to Channel, Stroke");
                }
            }
            else
            {
                //check whether or not m is NodeOn or m is a one of NodeNr of Tone
                if ((m.getTimeStamp() == m2.getTimeStamp()) && (m.getTimeStamp() == m3.getTimeStamp()))
                {
                    //add 3 nodeNr of Tone to stateTone for tracking to 3 NoteOff of Tone
                    node1ofTone.setLength(m.getVelocity());
                    stateTone[channelNr][m.getNoteNumber()] = m.getTimeStamp();
                    stateTone2[channelNr][m.getNoteNumber()] = start - tolerantOfTone;
                    node2ofTone.setLength(m2.getVelocity());
                    stateTone[channelNr][m2.getNoteNumber()] = m2.getTimeStamp();
                    stateTone2[channelNr][m2.getNoteNumber()] = start - tolerantOfTone;
                    node3ofTone.setLength(m3.getVelocity());
                    stateTone[channelNr][m3.getNoteNumber()] = m3.getTimeStamp();
                    stateTone2[channelNr][m3.getNoteNumber()] = start - tolerantOfTone;
                    //jump i and pass the three nodeNr of Tone"
                    i = i + 2;
                }
                else
                {
                    // note on
                    tempNode.setLength(velocity);
                    //add timestamp of Node On to state[channelNr][nodeNr] for checking node off
                    state[channelNr][nodeNr] = m.getTimeStamp();
                    state2[channelNr][nodeNr] = start;
                    //ttmm::logfileMidiReader->write("Node nr:" + nodeNr + " " + state[channelNr][nodeNr]);
                }
            }
        }
        else if (m.isNoteOff())
        {
            //check whether or not m is NodeOn or m is a one of NodeNr of Tone
            if ((m.getTimeStamp() == m2.getTimeStamp()) && (m.getTimeStamp() == m3.getTimeStamp()))
            {
                // notes off of Tone (Tonart)
                channelNr = m.getChannel() - 1;
                nodeNr1 = m.getNoteNumber();
                nodeNr2 = m2.getNoteNumber();
                nodeNr3 = m3.getNoteNumber();
                //check whether or not a nodeOn of Tone with the same node number
                //on the same channel is added
                if (stateTone[channelNr][nodeNr1] == -1 || stateTone[channelNr][nodeNr2] == -1
                    || stateTone[channelNr][nodeNr3] == -1)
                {
                    continue;
                }
                //add start/end time of stroke
                tempStroke.setStart(stateTone2[channelNr][nodeNr1]);
                tempStroke.setEnd(start - tolerantOfTone);
                //ttmm::logfileMidiReader->write(std::to_string(tempStroke.getStart()));
                //add 3 nodeNr of Tone to Stroke for storing
                node1ofTone.setTimestamp(stateTone2[channelNr][nodeNr1]);
                node1ofTone.setDelta(start - tolerantOfTone - stateTone2[channelNr][nodeNr1]);
                node1ofTone.setNodeNumber(nodeNr1);
                //add node to Stroke
                tempStroke.setNodeOfTone(node1ofTone);
                //
                node2ofTone.setTimestamp(stateTone2[channelNr][nodeNr2]);
                node2ofTone.setDelta(start - tolerantOfTone - stateTone2[channelNr][nodeNr2]);
                node2ofTone.setNodeNumber(nodeNr2);
                //add node to Stroke
                tempStroke.setNodeOfTone(node2ofTone);
                //
                node3ofTone.setTimestamp(stateTone2[channelNr][nodeNr3]);
                node3ofTone.setDelta(start - tolerantOfTone - stateTone2[channelNr][nodeNr3]);
                node3ofTone.setNodeNumber(nodeNr3);
                //add node to Stroke
                tempStroke.setNodeOfTone(node3ofTone);
                if (!track.isNotInTrack(channelNr, tempChannel))
                {
                    track.editChannel(channelNr, index, tempStroke);
                }
                else
                {
                    tempChannel.addStroke(index, tempStroke);
                    track.addChannel(channelNr, tempChannel);
                }
                //reset the tempstroke
                tempStroke.clear();
                //set stateTone of channelNr and nodeNr1/2/3 to -1 again
                stateTone[channelNr][nodeNr1] = -1;
                stateTone[channelNr][nodeNr2] = -1;
                stateTone[channelNr][nodeNr3] = -1;
                stateTone2[channelNr][nodeNr1] = -1;
                stateTone2[channelNr][nodeNr2] = -1;
                stateTone2[channelNr][nodeNr3] = -1;
                //ttmm::logfileMidiReader->write("Added Nodes of Tone to Channel, Stroke");
                //jump i and pass the three nodeNr of Tone
                i = i + 2;
                //update index of stroke
                index++;
            }
            else
            {
                // note off
                channelNr = m.getChannel() - 1;
                nodeNr = m.getNoteNumber();
                //check whether or not a nodeOn with the same node number on the same channel is added
                if (state[channelNr][nodeNr] == -1)
                {
                    continue;
                }
                //init a node
                double duration = m.getTimeStamp() - state[channelNr][nodeNr];
                tempNode.setTimestamp(state2[channelNr][nodeNr]);
                tempNode.setNodeNumber(nodeNr);

                //check type of note and divide to the quarter notes
                if ((duration >= (secondperqn - 0.1))
                    && (duration <= (secondperqn + 0.1))) //is a quarter note
                {
                    tempNode.setNoteType(ttmm::NoteType::Quarter);
                    //add timestamp and duration of a quarter note
                    vector<double> startQuarter;
                    for (int i = 0; i < 1; i++)
                    {
                        startQuarter.push_back(state2[channelNr][nodeNr] + double(i) * (secondperqnFromBPM));
                    }
                    tempNode.setStartQuarter(startQuarter);
                    tempNode.setDurationQuarter(secondperqnFromBPM - tolerantOfNote);
                    tempNode.setDelta(secondperqnFromBPM - tolerantOfNote);
                    start += secondperqnFromBPM;
                }
                else if ((duration >= ((2 * secondperqn) - 0.1))
                    && (duration <= ((2 * secondperqn) + 0.1))) //is a haft note?
                {
                    tempNode.setNoteType(ttmm::NoteType::Haft);
                    //divide the haft note to 2 quarter notes with the same duration
                    vector<double> startQuarter;
                    for (int i = 0; i < 2; i++)
                    {
                        startQuarter.push_back(state2[channelNr][nodeNr] + double(i) * (secondperqnFromBPM));
                    }
                    tempNode.setStartQuarter(startQuarter);
                    tempNode.setDurationQuarter(secondperqnFromBPM - tolerantOfNote);
                    tempNode.setDelta((double(2) * secondperqnFromBPM) - tolerantOfNote);
                    start += double(2) * secondperqnFromBPM;
                }
                else if ((duration >= ((4 * secondperqn) - 0.1))
                    && (duration <= ((4 * secondperqn) + 0.1))) //is a whole note
                {
                    tempNode.setNoteType(ttmm::NoteType::Whole);
                    //divide the whole note to 4 quarter notes with the same duration
                    vector<double> startQuarter;
                    for (int i = 0; i < 4; i++)
                    {
                        startQuarter.push_back(state2[channelNr][nodeNr] + double(i) * (secondperqnFromBPM));
                    }
                    tempNode.setStartQuarter(startQuarter);
                    tempNode.setDurationQuarter(secondperqnFromBPM - tolerantOfNote);
                    tempNode.setDelta((double(4) * secondperqnFromBPM) - tolerantOfNote);
                    start += double(4) * secondperqnFromBPM;
                }

                //add node to Stroke
                tempStroke.addNode(tempNode);
                //set state of channelNr and nodeNr to -1 again
                state[channelNr][nodeNr] = -1;
                state2[channelNr][nodeNr] = -1;
                //ttmm::logfileMidiReader->write("Added Node to Channel, Stroke");
            }
        }
    }
    //tempChannel = track.getChannel(channelNr);
    track.setEnd(start - tolerantOfTone);
    track.setOriginalNumbersOfStroke(index);
    //testing
    //ttmm::logfileMidiReader->write("Channel: ", (channelNr + 1), tempChannel.getStrokes().size());
    //delete poiter state, state2, stateTone, stateTone2 after process
    for (int i = 0; i < 16; ++i)
    {
        delete[] state[i];
        delete[] stateTone[i];
        delete[] state2[i];
        delete[] stateTone2[i];
    }
    delete[] state;
    delete[] stateTone;
    delete[] state2;
    delete[] stateTone2;
}

void MidiReader::readSignatureInfos(juce::MidiMessageSequence const* seq,
    int& kS, int& mm, int& nm, int& dn,
    double& ticklength, double& secondperqn, short const& timeformat)
{
    juce::MidiMessage tempMessage;
    for (int event = 0; event < seq->getNumEvents(); event++)
    {
        tempMessage = (seq->getEventPointer(event)->message);

        if (tempMessage.isKeySignatureMetaEvent())
        {
            kS = tempMessage.getKeySignatureNumberOfSharpsOrFlats();
            //0 is major, 1 is minor
            if (tempMessage.isKeySignatureMajorKey())
            {
                mm = 0;
            }
            else
            {
                mm = 1;
            }
            //ttmm::logfileMidiReader->write("Key Signature", kS, mm);
        }
        else if (tempMessage.isTimeSignatureMetaEvent())
        {
            tempMessage.getTimeSignatureInfo(nm, dn);
            //ttmm::logfileMidiReader->write("Time Signature", nm, dn);
        }
        else if (tempMessage.isTempoMetaEvent())
        {
            ticklength = tempMessage.getTempoMetaEventTickLength(timeformat);
            secondperqn = tempMessage.getTempoSecondsPerQuarterNote();
            //ttmm::logfileMidiReader->write("Tempo Signature", ticklength, secondperqn);
        }
    }
}