#ifndef IPC_Client_H
#define IPC_Client_H

#include "../JuceLibraryCode/JuceHeader.h"
#include "DataExchange.pb.h"
#include "GeneralPluginProcessor.h"

namespace ttmm{

	class IPCConnection : public juce::InterprocessConnection
	{
	public:

		IPCConnection(GeneralPluginProcessor* owner, bool isServer = false) : 
            InterprocessConnection(true), owner(owner), ownerIsServer(isServer) {}
		~IPCConnection(){};

		void connectionMade() override final;
		void connectionLost() override final;
		void messageReceived(const juce::MemoryBlock& message) override final;

		void send(IPCSongInfo si);

	private:
		GeneralPluginProcessor* owner;
        bool ownerIsServer;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IPCConnection)
	};
}

#endif
