/*
===========================================================================

  Copyright (c) 2025 LandSandBoat Dev Teams

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/

===========================================================================
*/

#include "0x00e_char_npc.h"

#include "common/utils.h"
#include "mob_modifier.h"

#include <cstring>

#include "entities/baseentity.h"
#include "entities/mobentity.h"
#include "entities/npcentity.h"
#include "entities/petentity.h"
#include "entities/trustentity.h"
#include "status_effect_container.h"

namespace GP_SERV_COMMAND_CHAR_NPC
{

// ============================================================================
// Helper: Write common fields shared by all SubKinds
// ============================================================================
namespace
{

void writeGeneralFieldsMob(CommonData& packet, CMobEntity* PMob)
{
    packet.Hpp           = PMob->GetHPP();
    packet.server_status = PMob->animation;

    packet.Flags1.MonsterFlag = true;
    packet.Flags1.GraphSize   = PMob->modelSize;
    packet.Flags2.g           = static_cast<uint8_t>(PMob->modelHitboxSize * 10);

    if (PMob->StatusEffectContainer)
    {
        packet.Flags3.MotStopFlag = PMob->StatusEffectContainer->HasStatusEffect(EFFECT_TERROR);
    }
    packet.Flags3.CliPriorityFlag = PMob->priorityRender;
    packet.Flags3.BallistaTeam    = static_cast<uint8_t>(PMob->allegiance);
    packet.Flags3.MonStat         = PMob->animationsub;
    packet.Flags3.unknown_0_3     = PMob->getMobMod(MOBMOD_SPAWN_ANIMATIONSUB) != 0;
}

void writeGeneralFieldsNpc(CommonData& packet, CNpcEntity* PNpc)
{
    packet.Hpp                 = 100;
    packet.server_status       = PNpc->animation;
    packet.Flags1.GraphSize    = PNpc->modelSize;
    packet.Flags2.g            = static_cast<uint8_t>(PNpc->modelHitboxSize * 10);
    packet.Flags3.PetFlag      = PNpc->IsTriggerable();
    packet.Flags3.BallistaTeam = static_cast<uint8_t>(PNpc->allegiance);
    packet.Flags3.MonStat      = PNpc->animationsub;
}

} // anonymous namespace

// ============================================================================
// SubKind 0: FixedModel - Only handles Model/Name data
// ============================================================================

FixedModel::FixedModel(const sendflags_t SendFlg, const CMobEntity* PMob)
{
    auto& packet = this->data();

    // Always set SubKind and ModelId - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PMob->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PMob->packetName.empty() ? PMob->getName() : PMob->packetName;
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

FixedModel::FixedModel(const sendflags_t SendFlg, const CPetEntity* PPet)
{
    auto& packet = this->data();

    // Always set SubKind and ModelId - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PPet->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PPet->packetName.empty() ? PPet->getName() : PPet->packetName;
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

FixedModel::FixedModel(const sendflags_t SendFlg, const CTrustEntity* PTrust)
{
    auto& packet = this->data();

    // Always set SubKind and ModelId - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PTrust->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PTrust->packetName.empty() ? PTrust->getName() : PTrust->packetName;
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

FixedModel::FixedModel(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Always set SubKind and ModelId - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PNpc->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

// ============================================================================
// SubKind 1: Equipped - Only handles Model/Name data
// ============================================================================

Equipped::Equipped(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Always set SubKind and equipment data - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::Equipped);
    std::memcpy(packet.GrapIDTbl, &PNpc->look, sizeof(packet.GrapIDTbl));

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

// ============================================================================
// SubKind 2: Door - Only handles Model/Name data
// ============================================================================

Door::Door(const sendflags_t SendFlg, CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Always set SubKind and DoorId - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::Door);
    packet.DoorId  = PNpc->GetLocalVar("DoorId");

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

// ============================================================================
// SubKind 3: Elevator - Only handles Model data (no name field)
// ============================================================================

Elevator::Elevator(const sendflags_t SendFlg, CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Always set SubKind and elevator data - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::Elevator);
    packet.DoorId  = PNpc->GetLocalVar("DoorId");
    packet.Time    = PNpc->GetLocalVar("TransportTimestamp");
    packet.EndTime = PNpc->GetLocalVar("TransportDuration");
}

// ============================================================================
// SubKind 4: Airship - Only handles Model data (no name field)
// ============================================================================

Airship::Airship(const sendflags_t SendFlg, CNpcEntity* PNpc)
{
    auto& packet = this->data();

    if (SendFlg.Model)
    {
        packet.SubKind = static_cast<uint16_t>(SubKind::Airship);
        packet.DoorId  = PNpc->GetLocalVar("DoorId");
        packet.Time    = PNpc->GetLocalVar("TransportTimestamp");
        packet.EndTime = PNpc->GetLocalVar("TransportDuration");
    }
}

// ============================================================================
// SubKind 5: MiscNpc - Only handles Model/Name data
// ============================================================================

MiscNpc::MiscNpc(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    if (SendFlg.Model)
    {
        packet.SubKind = static_cast<uint16_t>(SubKind::MiscNpc);
        packet.ModelId = PNpc->GetModelId();
    }

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

// ============================================================================
// SubKind 6: Automaton - Only handles Model/Name data
// ============================================================================

Automaton::Automaton(const sendflags_t SendFlg, const CPetEntity* PPet)
{
    auto& packet = this->data();

    if (SendFlg.Model)
    {
        packet.SubKind = static_cast<uint16_t>(SubKind::Automaton);
        packet.ModelId = PPet->GetModelId();
    }

    if (SendFlg.Name)
    {
        const auto& name = PPet->packetName.empty() ? PPet->getName() : PPet->packetName;
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

// ============================================================================
// SubKind 7: EquippedMisc - Only handles Model/Name data
// ============================================================================

EquippedMisc::EquippedMisc(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    if (SendFlg.Model)
    {
        packet.SubKind = static_cast<uint16_t>(SubKind::EquippedMisc);
        std::memcpy(packet.GrapIDTbl, &PNpc->look, sizeof(packet.GrapIDTbl));
    }

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size(), sizeof(packet.Name)));
    }
}

// ============================================================================
// Factory function - creates packet and fills common fields
// ============================================================================

auto create(const sendflags_t SendFlg, CBaseEntity* PEntity) -> CharNpcPacket
{
    if (!PEntity)
    {
        ShowError("GP_SERV_COMMAND_CHAR_NPC::create: PEntity was null");
        return FixedModel{};
    }

    // Create the appropriate packet type
    CharNpcPacket packet = [&]() -> CharNpcPacket
    {
        switch (PEntity->objtype)
        {
            case TYPE_MOB:
                return FixedModel(SendFlg, static_cast<CMobEntity*>(PEntity));

            case TYPE_TRUST:
                return FixedModel(SendFlg, static_cast<CTrustEntity*>(PEntity));

            case TYPE_PET:
            {
                auto* PPet = static_cast<CPetEntity*>(PEntity);
                if (static_cast<ModelType>(PPet->look.size) == ModelType::Automaton)
                {
                    return Automaton(SendFlg, PPet);
                }
                return FixedModel(SendFlg, PPet);
            }

            case TYPE_NPC:
            {
                auto* PNpc = static_cast<CNpcEntity*>(PEntity);
                switch (static_cast<ModelType>(PNpc->look.size))
                {
                    case ModelType::Equipped:
                        return Equipped(SendFlg, PNpc);
                    case ModelType::Door:
                        return Door(SendFlg, PNpc);
                    case ModelType::Elevator:
                        return Elevator(SendFlg, PNpc);
                    case ModelType::Ship:
                        return Airship(SendFlg, PNpc);
                    case ModelType::Unk5:
                        return MiscNpc(SendFlg, PNpc);
                    case ModelType::Chocobo:
                        return EquippedMisc(SendFlg, PNpc);
                    case ModelType::Standard:
                    default:
                        return FixedModel(SendFlg, PNpc);
                }
            }

            default:
                ShowError("GP_SERV_COMMAND_CHAR_NPC::create: Unknown objtype %d", PEntity->objtype);
                return FixedModel{};
        }
    }();

    // Fill common fields via std::visit
    // Access CommonData at offset 4 (after GP_SERV_HEADER)
    std::visit(
        [&](auto& pkt)
        {
            auto& data = *reinterpret_cast<CommonData*>(static_cast<uint8*>(pkt) + sizeof(GP_SERV_HEADER));

            // Always set identity
            data.UniqueNo = PEntity->id;
            data.ActIndex = PEntity->targid;
            data.SendFlg  = SendFlg;

            if (SendFlg.Despawn)
            {
                return;
            }

            if (SendFlg.Position)
            {
                data.dir               = PEntity->loc.p.rotation;
                data.x                 = PEntity->loc.p.x;
                data.z                 = PEntity->loc.p.y; // FFXI swaps y/z
                data.y                 = PEntity->loc.p.z;
                data.Flags0.MovTime    = PEntity->loc.p.moving;
                data.Flags0.facetarget = PEntity->m_TargID;
                data.Speed             = PEntity->GetSpeed();
                data.SpeedBase         = PEntity->animationSpeed;
            }

            if (SendFlg.General)
            {
                switch (PEntity->objtype)
                {
                    case TYPE_MOB:
                    case TYPE_PET:
                    case TYPE_TRUST:
                        writeGeneralFieldsMob(data, static_cast<CMobEntity*>(PEntity));
                        break;
                    case TYPE_NPC:
                        writeGeneralFieldsNpc(data, static_cast<CNpcEntity*>(PEntity));
                        break;
                    default:
                        break;
                }

                // Trust-specific flags
                if (PEntity->objtype == TYPE_TRUST)
                {
                    data.Flags3.TrustFlag  = true;
                    data.Flags3.PetNewFlag = true;
                    data.Flags3.PetFlag    = true;
                }
            }

            if (SendFlg.ClaimStatus)
            {
                if (PEntity->objtype == TYPE_MOB || PEntity->objtype == TYPE_PET || PEntity->objtype == TYPE_TRUST)
                {
                    data.BtTargetID = static_cast<CMobEntity*>(PEntity)->m_OwnerID.id;
                }
            }

            // Calculate and set correct packet size based on SendFlg
            // Base size is GP_SERV_HEADER + CommonData (offset 0x32 = 50 bytes)
            constexpr size_t baseSize = sizeof(GP_SERV_HEADER) + sizeof(CommonData);
            size_t           size     = baseSize;

            if (SendFlg.Model || SendFlg.Name)
            {
                // Add SubKind-specific model/name data sizes
                using T = std::decay_t<decltype(pkt)>;

                // Calculate model data size (everything between CommonData and Name)
                // and name size (if present)
                if constexpr (std::is_same_v<T, FixedModel> || std::is_same_v<T, MiscNpc> || std::is_same_v<T, Automaton>)
                {
                    // ModelId (2) + Name[16]
                    if (SendFlg.Model)
                    {
                        size += sizeof(uint16_t); // ModelId
                    }
                    if (SendFlg.Name)
                    {
                        size += 16; // Name[16]
                    }
                }
                else if constexpr (std::is_same_v<T, Equipped> || std::is_same_v<T, EquippedMisc>)
                {
                    // GrapIDTbl[9] (18) + Name[16]
                    if (SendFlg.Model)
                    {
                        size += sizeof(uint16_t) * 9; // GrapIDTbl[9]
                    }
                    if (SendFlg.Name)
                    {
                        size += 16; // Name[16]
                    }
                }
                else if constexpr (std::is_same_v<T, Door>)
                {
                    // Unused (2) + DoorId (4) + Name[12]
                    if (SendFlg.Model)
                    {
                        size += sizeof(uint16_t) + sizeof(uint32_t); // Unused + DoorId
                    }
                    if (SendFlg.Name)
                    {
                        size += 12; // Name[12]
                    }
                }
                else if constexpr (std::is_same_v<T, Elevator> || std::is_same_v<T, Airship>)
                {
                    // Unused (2) + DoorId (4) + Time (4) + EndTime (4) - no name field
                    if (SendFlg.Model)
                    {
                        size += sizeof(uint16_t) + sizeof(uint32_t) * 3; // Unused + DoorId + Time + EndTime
                    }
                    // No name field for Elevator/Airship
                }
            }

            pkt.setSize(size);
        },
        packet);

    return packet;
}

} // namespace GP_SERV_COMMAND_CHAR_NPC
