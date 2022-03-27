#pragma once

#include "EventData.h"
#include "NetworkPacket.h"

struct EventPeerTransfrom : public EventData
{
	inline static int32_t eventId = -1;
	uint8_t playerId = 0;
	PeerTransform peerTransform;

	EventPeerTransfrom(uint8_t playerId, const PeerTransform& peerTransform) : EventData(eventId), playerId(playerId), peerTransform(peerTransform) {}
};