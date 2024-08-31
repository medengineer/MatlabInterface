/*
------------------------------------------------------------------

This file is part of the Open Ephys GUI
Copyright (C) 2019 Open Ephys

------------------------------------------------------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "MatlabInterface.h"
#include "SocketThread.h"

#include <sstream>
#include <vector>
#include <algorithm>

SocketThread::SocketThread() : Thread("Socket Thread"),
								m_receivedFirstBlock(false),
								m_cleanExit(true)
{
}

SocketThread::~SocketThread()
{
}

void SocketThread::setQueuePointers(DataQueue* data)
{
	m_dataQueue = data;
}

void SocketThread::setSelectedChannels(Array<int> channels)
{
	m_channelArray = channels;
}

int SocketThread::openSocket(int port, String host)
{
	if (listen(port, host) == 1)
	{
		writeHeader();
		return 1;
	}
	else 
	{
		return 0;
	}
}

int SocketThread::listen(int port, String host)
{
    LOGC("Creating listener...");
    auto listenerSocket = std::make_unique<StreamingSocket>();
    if (!listenerSocket->createListener(port, host.isEmpty() ? "127.0.0.1" : host))
    {
        LOGC("Failed to create listener!");
        return -2;
    }

    while (true)
    {
        LOGD("Waiting for next connection...");
        socket.reset(std::move(listenerSocket->waitForNextConnection()));

        if (socket != nullptr && socket->isConnected())
        {
            LOGD("Connection established!");
			return 1;
        }
        else
        {
            LOGD("Failed to establish connection!");
            break;
        }
    }
    return -1;
}

int SocketThread::writeHeader()
{
	//TODO: Write header to Matlab to specify the number of channels and the sampling rate
	return 1;
}

void SocketThread::setFirstBlockFlag(bool didReceiveFirstBlock)
{
	m_receivedFirstBlock = didReceiveFirstBlock;
	this->notify(); //breaks the thread out of the wait method 
}

void SocketThread::run()
{

	const AudioSampleBuffer& dataBuffer = m_dataQueue->getAudioBufferReference();
	bool closeEarly = false;
	//1-Wait until the first block has arrived, so we can align the timestamps
	while (!m_receivedFirstBlock && !threadShouldExit())
	{
		printf("\rWaiting for first block..."); fflush(stdout);
		wait(1);
	}

	//3-Normal loop
	while (!threadShouldExit())
	{
		writeData(dataBuffer, BLOCK_MAX_WRITE_SAMPLES, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES, false);
	}

	//4-Before closing the thread, try to write the remaining samples
	if (!closeEarly)
	{
		writeData(dataBuffer, BLOCK_MAX_WRITE_SAMPLES, BLOCK_MAX_WRITE_EVENTS, BLOCK_MAX_WRITE_SPIKES, true);
	}
	m_cleanExit = true;
	m_receivedFirstBlock = false;

	LOGC("***** Socket thread finished");

}

void SocketThread::writeData(const AudioSampleBuffer& dataBuffer, int maxSamples, int maxEvents, int maxSpikes, bool lastBlock)
{

	Array<int64> timestamps;
	Array<CircularBufferIndexes> idx;
	m_dataQueue->startRead(idx, timestamps, maxSamples);
	m_numChannels = dataBuffer.getNumChannels();

	for (int chan = 0; chan < m_numChannels; ++chan)
	{
		if (idx[chan].size1 > 0)
		{
			writeSocket(chan, dataBuffer.getReadPointer(chan, idx[chan].index1), idx[chan].size1, 1);
			if (idx[chan].size2 > 0)
			{
				timestamps.set(chan, timestamps[chan] + idx[chan].size1);
				writeSocket(chan, dataBuffer.getReadPointer(chan, idx[chan].index2), idx[chan].size2, 2);
			}
		}
	}
	m_dataQueue->stopRead();

}

int SocketThread::writeSocket(int channel, const float* buffer, int size, int idx)
{
    // Resize buffer if necessary
    if (size > bufferSize)
    {
        LOGD("Write buffer overrun, resizing from: ", bufferSize, " to: ", size);
        bufferSize = size;
    }

    // Prepare the data in a single string using a string stream for efficiency
    std::ostringstream oss;
    std::copy(buffer, buffer + size, std::ostream_iterator<float>(oss, " "));

    // Write the whole line at once, reducing the number of socket calls
    std::string dataLine = oss.str() + "\n";
    int bytesWritten = socket->write(dataLine.c_str(), dataLine.size());
    if (bytesWritten < 0)
    {
        // Handle error
        std::cerr << "Failed to write data to socket" << std::endl;
        return -1; // or throw an exception
    }

    // Optionally, read response from server if necessary
    bool blockUntilSpecifiedAmountHasArrived = false;
    int bytesRead = socket->read(readBuffer, READ_BUFFER_SIZE, blockUntilSpecifiedAmountHasArrived);
    if (bytesRead < 0)
    {
        // Handle error
        std::cerr << "Failed to read data from socket" << std::endl;
        return -1; // or throw an exception
    }

    return 1; // Success
}