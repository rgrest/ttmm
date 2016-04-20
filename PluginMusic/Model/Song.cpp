#include "Song.h"

using namespace ttmm;

Song::Song(short bpm)
{
    this->bpm = bpm;
    this->tracks.clear();
}

Song::~Song()
{
    this->tracks.clear();
}

int Song::getNumberofTracks()
{
    return this->tracks.size();
}

void Song::readAllTracks()
{
    //remove the file if it exists
    std::ifstream in;
    in.open("song.txt");
    if (in.is_open())
    {
        std::remove("song.txt");
    }
    //init a output file
    std::ofstream ff;
    ff.open("song.txt", std::ios::out | std::ios_base::app);

    //return number of Tracks
    int nrTracks = this->getNumberofTracks();
    ff << "BPM: " << this->bpm << " --- Nr of Tracks: " << nrTracks << std::endl;

    for (int i = 0; i < nrTracks; i++)
    {
        ff << "Track: " << (i + 1) << "\t Duration: " << this->tracks[i].getDurationOfTrack() << "(s) "
           << "\t Key: " << this->tracks[i].getKeySignature() << " Minor(1)/Major(0): " << this->tracks[i].getMinorOrMajor()
           << "\t Type: " << this->tracks[i].getNumerator() << "/" << this->tracks[i].getDenominator()
           //<< "\t Tick length: " << this->tracks[i].getTicklength()
           << "\t Seconds per Quarter note: " << this->tracks[i].getSecondPerQuarter() << " (s)"
           << std::endl;
        //return number of channels
        int nrChannels = this->tracks[i].getNumberofChannels();
        ff << "Nr of Channels: " << nrChannels << std::endl;
        for (auto& mymap : this->tracks[i].getChannels())
        {
            Channel channel = mymap.second;
            ff << "Channel: " << (mymap.first + 1)
               << "\t has " << channel.getStrokes().size()
               << " strokes." << std::endl;
            for (auto k : channel.getStrokeNumbers())
            {
                Stroke stroke = channel.getStroke(k);
                ff << "Stroke " << (k + 1) << ": "
                   << stroke.getStart() << " " << stroke.getEnd()
                   << std::endl;

                for (size_t i = 0; i < stroke.getNodeOfTone().size(); i++)
                {
                    Node nodeTone = stroke.getNodeOfTone()[i];
                    ff << " timestamp: " << nodeTone.getTimestamp()
                       << " duration: " << nodeTone.getDelta()
                       << " node: " << nodeTone.getNodeNumber()
                       << " velocity: " << unsigned(nodeTone.getLength())
                       << std::endl;
                }

                for (int i = 0; i < stroke.getNodeCount(); i++)
                {
                    Node node = stroke.getNode(i);
                    ff << " timestamp: " << node.getTimestamp()
                       << " duration: " << node.getDelta()
                       << " node: " << node.getNodeNumber()
                       << " velocity: " << unsigned(node.getLength())
                       << std::endl;

                    if (node.getNoteType() == 1)
                    {
                        ff << "Quarter note to quarter notes: "
                           << node.getStartQuarter()[0]
                           << " " << node.getDurationQuarter() << std::endl;
                    }
                    if (node.getNoteType() == 2)
                    {
                        ff << "Divide the half note to quarter notes: "
                           << node.getStartQuarter()[0] << " " << node.getStartQuarter()[1]
                           << " " << node.getDurationQuarter() << std::endl;
                    }
                    else if (node.getNoteType() == 4)
                    {
                        ff << "Divide the whole note to quarter notes: "
                           << node.getStartQuarter()[0] << " " << node.getStartQuarter()[1]
                           << " " << node.getStartQuarter()[2] << " " << node.getStartQuarter()[3]
                           << " " << node.getDurationQuarter() << std::endl;
                    }
                }
                /*
				for (int i = 0; i < stroke.getAftertouchCount(); i++)
				{
					Aftertouch aftertouch = stroke.getAftertouch(i);
					ff << " timestamp: " << aftertouch.timestamp
						<< " node: " << aftertouch.nodeNumber
						<< " velocity: " << unsigned(aftertouch.strength)
						<< std::endl;
				}*/
            }
            ff << std::endl;
        }
        ff << std::endl
           << std::endl;
    }
    ff.close();
}

void Song::generateTrack(int index)
{
    if (this->tracks.size() <= size_t(index))
    {
        return;
    }
    //return endtime of track
    double endTimeofTrack = this->tracks[index].getEnd();
    double durationNote = this->tracks[index].getSecondPerQuarter();
    //return number of channels
    int nrChannels = this->tracks[index].getNumberofChannels();
    //return the list of channel number
    vector<int> channelnumbers = this->tracks[index].getChannelNumbers();
    for (int channelnumber : channelnumbers)
    {
        Channel channel = this->tracks[index].getChannel(channelnumber);
        size_t nrOfStroke = channel.getStrokes().size();
        for (size_t k = 0; k < nrOfStroke; k++)
        {
            Stroke stroke = channel.getStroke(k);
            Stroke tempStroke;
            //add new node of tone with the new timestamp
            size_t numberOfTone = stroke.getNodeOfTone().size();
            for (size_t i = 0; i < numberOfTone; i++)
            {
                Node nodeTone = stroke.getNodeOfTone()[i];
                nodeTone.setTimestamp(nodeTone.getTimestamp() + endTimeofTrack);
                tempStroke.setStart(nodeTone.getTimestamp());
                tempStroke.setEnd(nodeTone.getTimestamp() + nodeTone.getDelta());
                tempStroke.setNodeOfTone(nodeTone);
            }
            //add new node with the new timestamp
            auto numberOfNode = stroke.getNodeCount();
            for (int i = 0; i < numberOfNode; i++)
            {
                Node node = stroke.getNode(i);
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
            /*//add new aftertouch with the new timestamp
			auto numberOfAT = stroke.getAftertouchCount();
			for (int i = 0; i < numberOfAT; i++)
			{
				Aftertouch aftertouch = stroke.getAftertouch(i);
				aftertouch.timestamp = aftertouch.timestamp + endTimeofTrack;
				tempStroke.addAftertouch(aftertouch);
			}
			*/
            //add the updated stroke to song obj
            this->tracks[index].editChannel(channelnumber, (k + nrOfStroke), tempStroke);
        }
    }
    this->tracks[index].setEnd(2 * endTimeofTrack);
}

void Song::readTrack(int index)
{
    if (size_t(index) >= tracks.size())
    {
        return;
    }
    std::string filename = "track";
    filename += std::to_string(index);
    filename += ".txt";
    //remove the file if it exists
    std::ifstream in;
    in.open(filename);
    if (in.is_open())
    {
        std::remove(filename.c_str());
    }
    //init a output file
    std::ofstream ff;
    ff.open(filename, std::ios::out | std::ios_base::app);

    ff << "BPM: " << this->bpm << std::endl;
    ff << "Track: " << index
       << "\t Key: " << this->tracks[index].getKeySignature() << " " << this->tracks[index].getMinorOrMajor()
       << "\t Type: " << this->tracks[index].getNumerator() << " " << this->tracks[index].getDenominator()
       << std::endl;
    //return number of channels
    int nrChannels = this->tracks[index].getNumberofChannels();
    //return the list of channel number
    ff << "Nr of Channels: " << nrChannels << std::endl;
    vector<int> channelnumbers = this->tracks[index].getChannelNumbers();
    for (int channelnumber : channelnumbers)
    {
        Channel channel = this->tracks[index].getChannel(channelnumber);
        ff << "Channel: " << (channelnumber + 1)
           << "\t has " << channel.getStrokes().size()
           << " strokes. " << std::endl;
        for (size_t k = 0; k < channel.getStrokes().size(); k++)
        {
            Stroke stroke = channel.getStroke(k);
            ff << "Stroke " << k << ": " << std::endl;

            for (size_t i = 0; i < stroke.getNodeOfTone().size(); i++)
            {
                Node nodeTone = stroke.getNodeOfTone()[i];
                ff << " timestamp: " << nodeTone.getTimestamp()
                   << " duration: " << nodeTone.getDelta()
                   << " node: " << nodeTone.getNodeNumber()
                   << " velocity: " << unsigned(nodeTone.getLength())
                   << std::endl;
            }

            for (int i = 0; i < stroke.getNodeCount(); i++)
            {
                Node node = stroke.getNode(i);
                ff << "Type: " << node.getNoteType()
                   << " timestamp: " << node.getTimestamp()
                   << " duration: " << node.getDelta()
                   << " node: " << node.getNodeNumber()
                   << " velocity: " << unsigned(node.getLength())
                   << std::endl;
            }
            /*
			for (int i = 0; i < stroke.getAftertouchCount(); i++)
			{
				Aftertouch aftertouch = stroke.getAftertouch(i);
				ff << " timestamp: " << aftertouch.timestamp
					<< " node: " << aftertouch.nodeNumber
					<< " velocity: " << unsigned(aftertouch.strength)
					<< std::endl;
			}*/
        }
        ff << std::endl;
    }
    ff << std::endl
       << std::endl;
    ff.close();
}

void Song::addTrack(Track& track)
{
    this->tracks.push_back(track);
}

vector<Track> Song::getTracks()
{
    return this->tracks;
}

Track Song::getTrack(int index)
{
    return this->tracks[index];
}
Track* Song::getTrackp(int index)
{
    if (size_t(index) >= this->tracks.size())
    {
        return nullptr;
    }
    return &this->tracks[index];
}

void Song::addBPM(short const& bpm)
{
    this->bpm = bpm;
}

short Song::getBpm()
{
    return this->bpm;
}
