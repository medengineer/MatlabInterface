#ifndef MATLABINTERFACE_H_DEFINED
#define MATLABINTERFACE_H_DEFINED

#include <ProcessorHeaders.h>
#include "MatlabInterfaceEditor.h"

#include "DataQueue.h"
#include "SocketThread.h"

#include <iostream>
#include <chrono>
#include <functional>

#include <algorithm>
#include <random>
#include <vector>

#define MAX_MESSAGE_SIZE 		2000

#define WRITE_BLOCK_LENGTH		1024	
#define DATA_BUFFER_NBLOCKS		300
#define EVENT_BUFFER_NEVENTS	512
#define SPIKE_BUFFER_NSPIKES	512

#define NIDAQ_BIT_VOLTS			0.001221f
#define NPX_BIT_VOLTS			0.195f
#define MAX_BUFFER_SIZE			40960
#define CHANNELS_PER_THREAD		384


class MatlabInterface : public GenericProcessor, public Timer, public Thread
{
public:

	/** Constructor */
	MatlabInterface();

	/** Destructor */
	~MatlabInterface();

	/** Create custom editor */
	AudioProcessorEditor* createEditor() override;

	/** Override stopAcquisition */
	bool stopAcquisition() override;

	/** Set currently selected channel */
	void setSelectedChannels(Array<int> channels);

	/** Get currently selected channel */
	Array<int> getSelectedChannels() { return selectedChannels; };

	/** Check for Matlab instance at specified address/port */
	bool connect();

	/** Stream continuous channel to Matlab */
	void process(AudioBuffer<float>& buffer) override;

	/** Update selected channel for each stream */
	void updateSettings() override;
	
	/** Called whenever a parameter's value is changed (called by GenericProcessor::setParameter())*/
    void parameterValueChanged(Parameter* param) override;

	std::unique_ptr<SocketThread> socketThread;

	/** Timer callback */
	void timerCallback() override;

	/** Thread run method */
	void run() override;

private:

	bool startedAcquisition = false;

	bool connected;

	int selectedStreamIdx;

	Array<int> selectedChannels;

	std::unique_ptr<DataQueue> dataQueue;

	std::vector<float> matlab_data;

};

#endif