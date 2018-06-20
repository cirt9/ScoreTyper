#include "packetprocessorwrapper.h"

namespace Client
{
    PacketProcessorWrapper::PacketProcessorWrapper(QObject * parent) : QObject(parent)
    {
        packetProcessor = new Client::PacketProcessor();

        connect(packetProcessor, &Client::PacketProcessor::registrationReply,
                this, &PacketProcessorWrapper::registrationReply);
        connect(packetProcessor, &Client::PacketProcessor::loggingReply,
                this, &PacketProcessorWrapper::loggingReply);
        connect(packetProcessor, &Client::PacketProcessor::profileDownloadRedply,
                this, &PacketProcessorWrapper::profileDownloadReply);
        connect(packetProcessor, &Client::PacketProcessor::requestError,
                this, &PacketProcessorWrapper::requestError);
    }

    PacketProcessorWrapper::~PacketProcessorWrapper()
    {
        packetProcessor->deleteLater();
    }

    Client::PacketProcessor * PacketProcessorWrapper::getPacketProcessor() const
    {
        return packetProcessor;
    }
}
