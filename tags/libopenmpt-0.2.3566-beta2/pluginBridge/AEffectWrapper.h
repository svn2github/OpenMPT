#pragma once

#include <vector>
#define VST_FORCE_DEPRECATED 0
#include "../include/vstsdk2.4/pluginterfaces/vst2.x/aeffectx.h"

#pragma pack(push, 8)

template<typename ptr_t>
struct AEffectProto
{
	int32_t magic;
	ptr_t dispatcher;
	ptr_t process;
	ptr_t setParameter;
	ptr_t getParameter;

	int32_t numPrograms;
	int32_t numParams;
	int32_t numInputs;
	int32_t numOutputs;

	int32_t flags;
	
	ptr_t resvd1;
	ptr_t resvd2;
	
	int32_t initialDelay;
	
	int32_t realQualities;
	int32_t offQualities;
	float ioRatio;

	ptr_t object;
	ptr_t user;

	int32_t uniqueID;
	int32_t version;

	ptr_t processReplacing;
	ptr_t processDoubleReplacing;
	char future[56];

	// Convert native representation to bridge representation.
	// Don't overwrite any values managed by the bridge wrapper or host in general.
	void FromNative(const AEffect &in)
	{
		magic = in.magic;
		//dispatcher = 0;
		//process = in.process ? 1 : 0;
		//setParameter = 0;
		//getParameter = 0;

		numPrograms = in.numPrograms;
		numParams = in.numParams;
		numInputs = in.numInputs;
		numOutputs = in.numOutputs;

		flags = in.flags;

		//resvd1 = 0;
		//resvd2 = 0;

		initialDelay = in.initialDelay;

		realQualities = in.realQualities;
		offQualities = in.offQualities;
		ioRatio = in.ioRatio;

		//object = 0;
		//user = 0;

		uniqueID = in.uniqueID;
		version = in.version;

		//processReplacing = in.processReplacing ? 2 : 0;
		//processDoubleReplacing = in.processDoubleReplacing ? 3 : 0;

		//memset(future, 0, sizeof(future));

		if(in.processReplacing == nullptr)
		{
			flags &= ~effFlagsCanReplacing;
		}
		if(in.processDoubleReplacing == nullptr)
		{
			flags &= ~effFlagsCanDoubleReplacing;
		}
	}
};

typedef AEffectProto<int32_t> AEffect32;
typedef AEffectProto<int64_t> AEffect64;


#pragma pack(pop)


// Translate a VSTEvents struct to bridge format (placed in data vector)
static void TranslateVSTEventsToBridge(std::vector<char> &data, const VstEvents *events, int32_t targetPtrSize)
{
	data.reserve(data.size() + sizeof(int32_t) + sizeof(VstEvent) * events->numEvents);
	// Write number of events
	PushToVector(data, events->numEvents);

	// Write events
	for(VstInt32 i = 0; i < events->numEvents; i++)
	{
		if(events->events[i]->type == kVstSysExType)
		{
			// This is going to be messy since the VstMidiSysexEvent event has a different size than other events on 64-bit platforms.
			// We are going to write the event using the target process pointer size.
			const VstMidiSysexEvent *event = reinterpret_cast<const VstMidiSysexEvent *>(events->events[i]);
			PushToVector(data, *events->events[i], sizeof(VstEvent) + sizeof(VstInt32));	// Regular VstEvent struct + dump size
			data.resize(data.size() + 3 * targetPtrSize);									// Dump pointer + two reserved VstIntPtrs
			// Embed SysEx dump as well...
			data.insert(data.end(), event->sysexDump, event->sysexDump + event->dumpBytes);
		} else
		{
			PushToVector(data, *events->events[i], events->events[i]->byteSize);
		}
	}
}


// Translate bridge format (void *ptr) back to VSTEvents struct (placed in data vector)
static void TranslateBridgeToVSTEvents(std::vector<char> &data, void *ptr)
{
	int32_t numEvents = *static_cast<const int32_t *>(ptr);

	data.resize(sizeof(VstInt32) + sizeof(VstIntPtr) + sizeof(VstEvent *) * numEvents, 0);
	VstEvents *events = reinterpret_cast<VstEvents *>(&data[0]);
	events->numEvents = numEvents;

	// Write pointers
	char *offset = static_cast<char *>(ptr) + sizeof(int32_t);
	for(int32_t i = 0; i < numEvents; i++)
	{
		events->events[i] = reinterpret_cast<VstEvent *>(offset);
		offset += events->events[i]->byteSize;
		if(events->events[i]->type == kVstSysExType)
		{
			VstMidiSysexEvent *event = reinterpret_cast<VstMidiSysexEvent *>(events->events[i]);
			event->byteSize = sizeof(VstMidiSysexEvent);	// Adjust to target platform
			event->sysexDump = offset;
			offset += event->dumpBytes;
		}
	}
}