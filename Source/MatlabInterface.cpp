#include "MatlabInterface.h"

MatlabInterface::MatlabInterface() 
	: GenericProcessor("Matlab Interface"), Thread("MatlabInterface"),
	dataQueue(std::make_unique<DataQueue>(WRITE_BLOCK_LENGTH, DATA_BUFFER_NBLOCKS)),
	socketThread(std::make_unique<SocketThread>()),
	connected(false),
	selectedStreamIdx(0),
	selectedChannels({})
{
	addStringParameter(Parameter::PROCESSOR_SCOPE, "host", "Host address", "Set host address", "127.0.0.1", true);
	addStringParameter(Parameter::PROCESSOR_SCOPE, "port", "Port number", "Set port number", "1234", true);
	addSelectedStreamParameter(Parameter::PROCESSOR_SCOPE, "stream", "Stream", "Set the stream to stream to Matlab", {}, 0, true);
	addSelectedChannelsParameter(Parameter::STREAM_SCOPE, "channels", "Channels", "Set the continuous channels to stream to Matlab", 16, true);

	startTimer(500);
}

MatlabInterface::~MatlabInterface()
{
	if (!connected && socketThread->socket->connection != nullptr)
		socketThread->socket->connection->close();

	socketThread->signalThreadShouldExit();
	socketThread->waitForThreadToExit(2000);
}

void MatlabInterface::timerCallback()
{
	startThread();
}

void MatlabInterface::run()
{
	if (connect()) {
		stopTimer();
		MessageManager::callAsync([this]() {
			CoreServices::updateSignalChain(this);
		});
	}
}

bool MatlabInterface::connect()
{
	String port = getParameter("port")->getValue();
	String host = getParameter("host")->getValue();

	if (socketThread->openSocket(port.getIntValue(), host) > 0)
		connected = true;
	else
		connected = false;

	return connected;
}

void MatlabInterface::parameterValueChanged(Parameter* param)
{
	if (param->getName() == "stream")
	{
		selectedStreamIdx = param->getValue();
		Array<int> channels;
		for (auto channel : getDataStreams()[selectedStreamIdx]->getContinuousChannels())
			channels.add(channel->getGlobalIndex());
		setSelectedChannels(channels);
	}
}

void MatlabInterface::setSelectedChannels(Array<int> channels)
{
	selectedChannels = channels;
	socketThread->setSelectedChannels(selectedChannels);
}

AudioProcessorEditor* MatlabInterface::createEditor()
{
    editor = std::make_unique<MatlabInterfaceEditor>(this);
    return editor.get();
}

void MatlabInterface::process(AudioSampleBuffer& buffer)
{
	if (!connected)
		return;

	const int nChannels = selectedChannels.size();

	if (!socketThread->isThreadRunning())
	{
		dataQueue->setChannels(selectedChannels);
		socketThread->setQueuePointers(dataQueue.get());
		socketThread->setFirstBlockFlag(true);
		socketThread->setSelectedChannels(selectedChannels);
		socketThread->startThread();
	}

	for (int i = 0; i < getDataStreams().size(); i++)
	{
		if (i != selectedStreamIdx)
			continue;

		const uint16 streamId = getDataStreams()[i]->getStreamId();
		uint32 numSamples = getNumSamplesInBlock(streamId);
		int64 sampleNumber = getFirstSampleNumberForBlock(streamId);

		const DataStream* stream = getDataStreams()[i];

		int local_index = 0;
		for (auto global_index : selectedChannels)
		{
			dataQueue->writeChannel(buffer, local_index, global_index, numSamples, sampleNumber);
			local_index++;
		}

	}

}

void MatlabInterface::updateSettings()
{
	isEnabled = connected;

	for (auto stream : getDataStreams())
        parameterValueChanged(stream->getParameter("channels"));
}
