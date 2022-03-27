#pragma once

#include "Component.h"
#include "NetworkAbstraction.h"
#include "NetworkPacket.h"
#include "Globals.h"
#include "EventManager.h"
#include "EventDie.h"
#include "EventPeerTransform.h"
#include "EventPeerLeft.h"
#include "EventZombieReset.h"
#include "PlayerComponent.h"
#include "EventGameOver.h"

class NetworkUpdateComponent : public Component
{
    NetworkAbstraction networkAbstraction;
    const NetworkAbstraction::NetworkBuffer& buffer;
    NetworkPacket& networkPacket;
    uint32_t lastPeerCount = 0;
    uint32_t sendCounter = 0;
    int32_t receiveCounter = -1;
    float sendingSpectator = 0.0f;
    Vector<uint8_t> zombieId;
public:
    NetworkUpdateComponent(const char* lobbyName, const String& username, NetworkPacket& networkPacketIn) : Component(utils::getGUID()),
        networkAbstraction(username, lobbyName, networkPacketIn), buffer(networkAbstraction.getNetworkBuffer()), networkPacket(networkPacketIn)
    {
    }

    void update(float dt, Actor* owner) override
    {
#if 1
        networkAbstraction.update();
        assert(networkAbstraction.isJoinedLobby());
        uint32_t currentPeerCount = networkAbstraction.getConnectedPeersCount();
        if (currentPeerCount > lastPeerCount)
        {
            while (currentPeerCount < zombieId.size())
            {
                zombieId.emplace_back((int8_t)-1);
            }
            utils::log("Reset due to joined player!");
            Globals::eventManager->TriggerEvent<EventZombieReset>(true);
        }
        else if (currentPeerCount < lastPeerCount)
        {
            while (currentPeerCount > zombieId.size())
            {
                zombieId.pop_back();
            }
            utils::log("Event Peer left fired!");
            Globals::eventManager->TriggerEvent<EventPeerLeft>();
        }
        lastPeerCount = currentPeerCount;

        networkAbstraction.receive();
        if (buffer.size != 0)
        {
            do
            {
                if (buffer.size != sizeof(NetworkPacket))
                {
                    continue;
                }
                assert(buffer.size == sizeof(NetworkPacket));
                auto networkPacket = (ReceivePacket*)buffer.bytes;

                if (zombieId[networkPacket->playerId] != networkPacket->networkPacket.zombie)
                {
                    zombieId[networkPacket->playerId] = networkPacket->networkPacket.zombie;
                    if (networkPacket->networkPacket.zombie != (uint8_t)-1)
                    {
                        utils::log("Zombie Die Peer");
                        EventDie::eventId = networkPacket->networkPacket.zombie;
                        Globals::eventManager->TriggerEvent<EventDie>();
                    }
                }
                Globals::eventManager->TriggerEvent<EventPeerTransfrom>(networkPacket->playerId, networkPacket->networkPacket.peerTransforms);

                networkAbstraction.receive();
                receiveCounter = 0;
            } while (buffer.size != 0);
        }
        else if(receiveCounter >= 0)
        {
            ++receiveCounter;
        }

        if (lastPeerCount > 0 && receiveCounter > (int32_t)(dt * 300.0f))
        {
            Globals::eventManager->TriggerEvent<EventGameOver>();
        }

        if (sendingSpectator <= 5.0f && sendCounter >= 10)
        {
            if (networkPacket.peerTransforms.posY == PlayerComponent::SPECTATOR_POSITION_Y)
            {
                sendingSpectator += dt;
            }
            
            networkAbstraction.send((uint8_t*)&networkPacket, sizeof(NetworkPacket), false);
            sendCounter = 0;
        }
        ++sendCounter;
#endif
    }
};