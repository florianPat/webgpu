#pragma once

#include "Types.h"

struct PeerTransform
{
	int16_t posX = 0, posZ = 0;
	int8_t posY = 0, rotY = 0;
};

struct NetworkPacket
{
	uint8_t zombie = (uint8_t)-1;
	PeerTransform peerTransforms;
};

struct ReceivePacket
{
	NetworkPacket networkPacket;
	uint8_t playerId = 0;
};