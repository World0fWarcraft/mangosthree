/*
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2025 MaNGOS <https://www.getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#include "Common.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Opcodes.h"
#include "ObjectGuid.h"
#include "Log.h"
#include "Player.h"
#include "Vehicle.h"
#include "ObjectMgr.h"

void WorldSession::HandleDismissControlledVehicle(WorldPacket& recvPacket)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_DISMISS_CONTROLLED_VEHICLE");
    recvPacket.hexlike();

    MovementInfo movementInfo;                              // Not used at the moment

    recvPacket >> movementInfo;

    TransportInfo* transportInfo = _player->GetTransportInfo();
    if (!transportInfo || !transportInfo->IsOnVehicle())
    {
        return;
    }

    Unit* vehicle = (Unit*)transportInfo->GetTransport();

    // Something went wrong
    if (movementInfo.GetGuid() != vehicle->GetObjectGuid())
    {
        return;
    }

    // Remove Vehicle Control Aura
    vehicle->RemoveSpellsCausingAura(SPELL_AURA_CONTROL_VEHICLE, _player->GetObjectGuid());
}

void WorldSession::HandleRequestVehicleExit(WorldPacket& recvPacket)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_REQUEST_VEHICLE_EXIT");
    recvPacket.hexlike();

    TransportInfo* transportInfo = _player->GetTransportInfo();
    if (!transportInfo || !transportInfo->IsOnVehicle())
    {
        return;
    }

    Unit* vehicle = (Unit*)transportInfo->GetTransport();

    // Check for exit flag
    if (VehicleSeatEntry const* seatEntry = vehicle->GetVehicleInfo()->GetSeatEntry(transportInfo->GetTransportSeat()))
        if (seatEntry->m_flags & SEAT_FLAG_CAN_EXIT)
        {
            vehicle->RemoveSpellsCausingAura(SPELL_AURA_CONTROL_VEHICLE, _player->GetObjectGuid());
        }
}

void WorldSession::HandleRequestVehicleSwitchSeat(WorldPacket& recvPacket)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_REQUEST_VEHICLE_SWITCH_SEAT");
    recvPacket.hexlike();

    ObjectGuid vehicleGuid;
    uint8 seat;

    recvPacket >> vehicleGuid.ReadAsPacked();
    recvPacket >> seat;

    TransportInfo* transportInfo = _player->GetTransportInfo();
    if (!transportInfo || !transportInfo->IsOnVehicle())
    {
        return;
    }

    Unit* vehicle = (Unit*)transportInfo->GetTransport();

    // Something went wrong
    if (vehicleGuid != vehicle->GetObjectGuid())
    {
        return;
    }

    vehicle->GetVehicleInfo()->SwitchSeat(_player, seat);
}

void WorldSession::HandleChangeSeatsOnControlledVehicle(WorldPacket& recvPacket)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE");
    recvPacket.hexlike();

    ObjectGuid srcVehicleGuid;
    MovementInfo movementInfo;
    ObjectGuid destVehicleGuid;
    uint8 seat;

    recvPacket >> movementInfo;

    srcVehicleGuid = movementInfo.GetGuid();
    destVehicleGuid = movementInfo.GetGuid2();
    seat = movementInfo.GetByteParam();

    TransportInfo* transportInfo = _player->GetTransportInfo();
    if (!transportInfo || !transportInfo->IsOnVehicle())
    {
        return;
    }

    Unit* srcVehicle = (Unit*)transportInfo->GetTransport();

    // Something went wrong
    if (srcVehicleGuid != srcVehicle->GetObjectGuid())
    {
        return;
    }

    if (srcVehicleGuid != destVehicleGuid)
    {
        Unit* destVehicle = _player->GetMap()->GetUnit(destVehicleGuid);

        if (!destVehicle || !destVehicle->IsVehicle())
        {
            return;
        }

        // Change vehicle is not possible
        if (destVehicle->GetVehicleInfo()->GetVehicleEntry()->m_flags & VEHICLE_FLAG_DISABLE_SWITCH)
        {
            return;
        }

        SpellClickInfoMapBounds clickPair = sObjectMgr.GetSpellClickInfoMapBounds(destVehicle->GetEntry());
        for (SpellClickInfoMap::const_iterator itr = clickPair.first; itr != clickPair.second; ++itr)
            if (itr->second.IsFitToRequirements(_player, destVehicle->GetTypeId() == TYPEID_UNIT ? (Creature*)destVehicle : NULL))
            {
                _player->CastSpell(destVehicle, itr->second.spellId, true);
            }
    }
    else
    {
        srcVehicle->GetVehicleInfo()->SwitchSeat(_player, seat);
    }
}

void WorldSession::HandleRideVehicleInteract(WorldPacket& recvPacket)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_RIDE_VEHICLE_INTERACT");
    recvPacket.hexlike();

    ObjectGuid playerGuid;
    recvPacket >> playerGuid;

    Player* vehicle = _player->GetMap()->GetPlayer(playerGuid);

    if (!vehicle || !vehicle->IsVehicle())
    {
        return;
    }

    // Only allowed if in same raid
    if (!vehicle->IsInSameRaidWith(_player))
    {
        return;
    }

    _player->CastSpell(vehicle, SPELL_RIDE_VEHICLE_HARDCODED, true);
}

void WorldSession::HandleEjectPassenger(WorldPacket& recvPacket)
{
    DEBUG_LOG("WORLD: Received opcode CMSG_CONTROLLER_EJECT_PASSENGER");
    recvPacket.hexlike();

    ObjectGuid passengerGuid;
    recvPacket >> passengerGuid;

    Unit* passenger = _player->GetMap()->GetUnit(passengerGuid);

    if (!passenger || !passenger->IsBoarded())
    {
        return;
    }

    // _player is not a vehicle
    if (!_player->IsVehicle())
    {
        return;
    }

    VehicleInfo* vehicleInfo = _player->GetVehicleInfo();

    // _player must be transporting passenger
    if (!vehicleInfo->HasOnBoard(passenger))
    {
        return;
    }

    // Check for eject flag
    if (VehicleSeatEntry const* seatEntry = vehicleInfo->GetSeatEntry(passenger->GetTransportInfo()->GetTransportSeat()))
    {
        if (seatEntry->m_flagsB & SEAT_FLAG_B_EJECTABLE)
        {
            _player->RemoveSpellsCausingAura(SPELL_AURA_CONTROL_VEHICLE, passengerGuid);
        }
    }
}

void WorldSession::HandleRequestVehiclePrevSeat(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received CMSG_REQUEST_VEHICLE_PREV_SEAT");

        // ToDo
}

void WorldSession::HandleRequestVehicleNextSeat(WorldPacket& recv_data)
{
    DEBUG_LOG("WORLD: Received CMSG_REQUEST_VEHICLE_NEXT_SEAT");

        // ToDo
}
