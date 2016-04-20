#include "IPCConnection.h"

void ttmm::IPCConnection::connectionMade()
{
    //ttmm::logfileGeneral->write("(IPC) a connection was made! yay!");
}

void ttmm::IPCConnection::connectionLost()
{
    //ttmm::logfileGeneral->write("(IPC) connection was lost :(");
}

void ttmm::IPCConnection::send(IPCSongInfo objectToSend)
{
    // create string from SongInfo
    std::string serialString;
    objectToSend.SerializeToString(&serialString);

    // create MemoryBlock for string
    juce::MemoryBlock messageData((const void *)serialString.data(), serialString.size());

    // send serialized SongInfo
    if (!sendMessage(messageData))
    {
        //ttmm::logfileGeneral->write("(IPC) Error: Sending Failed. Connection for Interprocess Communication is broken or something.");
    }
    else
    {
        //ttmm::logfileGeneral->write("(IPC) sent message.");
    }
}

void ttmm::IPCConnection::messageReceived(const juce::MemoryBlock &message)
{
    if (!ownerIsServer) return;
    std::string msgString = message.toString().toStdString();
    IPCSongInfo si;
    si.ParseFromString(msgString);
    owner->receivedIPC(si);
}