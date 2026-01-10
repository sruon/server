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

// Calculate padded name size (min 16 bytes, 4-byte aligned) per retail analysis
// Retail uses 68-byte packets (16-byte name) for names <= 15 chars,
// and 72-byte packets (20-byte name) for names 16+ chars.
size_t calcNameSize(const uint8_t* nameField, const size_t maxLen)
{
    size_t len = 0;
    while (len < maxLen && nameField[len] != 0)
    {
        ++len;
    }
    return std::max<size_t>(16, (len + 4) & ~3);
}

void writeGeneralFieldsMob(CommonData& packet, CMobEntity* PMob)
{
    packet.Hpp           = PMob->GetHPP();
    packet.server_status = PMob->animation;

    // MonsterFlag is set unconditionally in factory (per retail)
    packet.Flags1.GraphSize = PMob->modelSize;
    packet.Flags2.g         = static_cast<uint8_t>(PMob->modelHitboxSize * 10);

    // Flags2.b: Retail shows 0x50 (bits 4+6) for alive, 0x00 for dead.
    // Low nibble = GEO Indi effect, high nibble -> Render.Flags7 bits 24-27 (Model Visibility)
    // packet.Flags2.b = PMob->GetHPP() > 0 ? 0x50 : 0x00;

    if (PMob->StatusEffectContainer)
    {
        packet.Flags3.MotStopFlag = PMob->StatusEffectContainer->HasStatusEffect(EFFECT_TERROR);
    }
    packet.Flags3.CliPriorityFlag = PMob->priorityRender;
    packet.Flags3.BallistaTeam    = static_cast<uint8_t>(PMob->allegiance);
    packet.Flags3.MonStat         = PMob->animationsub;
}

void writeGeneralFieldsNpc(CommonData& packet, CNpcEntity* PNpc)
{
    packet.Hpp           = 100;
    packet.server_status = PNpc->animation;

    // PetFlag (triggerable) is set unconditionally in factory (per retail)
    packet.Flags1.GraphSize    = PNpc->modelSize;
    packet.Flags2.g            = static_cast<uint8_t>(PNpc->modelHitboxSize * 10);
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

    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PMob->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PMob->packetName.empty() ? PMob->getName() : PMob->packetName;
        // Copy name + null terminator (c_str() includes null)
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    // Size: Header + CommonData + ModelId + Name (variable)
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

FixedModel::FixedModel(const sendflags_t SendFlg, const CPetEntity* PPet)
{
    auto& packet = this->data();

    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PPet->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PPet->packetName.empty() ? PPet->getName() : PPet->packetName;
        // Copy name + null terminator (c_str() includes null)
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

FixedModel::FixedModel(const sendflags_t SendFlg, const CTrustEntity* PTrust)
{
    auto& packet = this->data();

    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PTrust->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PTrust->packetName.empty() ? PTrust->getName() : PTrust->packetName;
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

FixedModel::FixedModel(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    packet.SubKind = static_cast<uint16_t>(SubKind::FixedModel);
    packet.ModelId = PNpc->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

// ============================================================================
// SubKind 1: Equipped - Only handles Model/Name data
// ============================================================================

Equipped::Equipped(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    packet.SubKind = static_cast<uint16_t>(SubKind::Equipped);
    if (SendFlg.Model)
    {
        // Skip look.size - GrapIDTbl starts at modelid (face/race)
        std::memcpy(packet.GrapIDTbl, &PNpc->look.modelid, sizeof(packet.GrapIDTbl));
    }

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    // Size: Header + CommonData + GrapIDTbl[9] + Name (variable)
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) * 9 + calcNameSize(packet.Name, sizeof(packet.Name)));
}

// ============================================================================
// SubKind 2: Door
//
// Retail SendFlags analysis (from captures):
//   - Doors ONLY use: Position|ClaimStatus|General (spawn + updates) or Despawn
//   - Name and Model flags are NEVER used for doors
//   - Client gets door names from DAT files, not packets
//
// Fields set by each flag:
//   Position: dir, x, z, y, Flags0, Speed (50), SpeedBase (50)
//   ClaimStatus: BtTargetID (always 0 for doors)
//   General: Hpp (100), server_status (8=closed, 9=open), Flags1, Flags3
//
// Key flags1 bits for doors: CliPosInitFlag=1, GraphSize=1
// ============================================================================

Door::Door([[maybe_unused]] const sendflags_t SendFlg, CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Retail SendFlags (from captures):
    //   Spawn/Update: Position|ClaimStatus|General (0x07)
    //   Despawn:      ClaimStatus|Despawn (0x22)
    // Name and Model flags are NEVER used for doors.

    packet.SubKind = static_cast<uint16_t>(SubKind::Door);

    // Retail: DoorId contains the first 4 bytes of the door's internal name (e.g. "_3h1", "_1e4")
    const auto& name = PNpc->getName();
    if (name.size() >= 4)
    {
        std::memcpy(&packet.DoorId, name.c_str(), sizeof(packet.DoorId));
    }

    // Name field stays empty - retail never uses Name flag for doors
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + sizeof(uint32_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

// ============================================================================
// SubKind 3: Elevator
//
// Retail SendFlags (from captures):
//   Spawn/Update: None (0x00), Name (0x08), Position|Name (0x09),
//                 or ClaimStatus|General|Name (0x0E)
// Name and Model flags are NEVER used for elevators.
//
// Fields:
//   DoorId:  First 4 bytes of entity name (ASCII, like doors)
//   Time:    Vana'diel timestamp (transport start/arrival time)
//   EndTime: Duration in seconds (4, 7, 8, 12 observed)
// ============================================================================

Elevator::Elevator([[maybe_unused]] const sendflags_t SendFlg, CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Retail SendFlags (from captures):
    //   Spawn/Update: None (0x00), Name (0x08), Position|Name (0x09),
    //                 or ClaimStatus|General|Name (0x0E)

    packet.SubKind = static_cast<uint16_t>(SubKind::Elevator);

    // Retail: DoorId contains the first 4 bytes of the elevator's internal name (like doors)
    const auto& name = PNpc->getName();
    if (name.size() >= 4)
    {
        std::memcpy(&packet.DoorId, name.c_str(), sizeof(packet.DoorId));
    }

    packet.Time = PNpc->GetLocalVar("TransportTimestamp");

    // Retail EndTime values observed: 4, 7, 8, 12 (varies by elevator)
    // TODO: Recapture elevators to determine correct per-elevator duration values
    // Default to 8 for now (matches old LSB behavior)
    auto duration  = PNpc->GetLocalVar("TransportDuration");
    packet.EndTime = duration > 0 ? duration : 8;

    // Fixed size: Header + CommonData + Unused + DoorId + Time + EndTime = 64 bytes
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + sizeof(uint32_t) * 3);
}

// ============================================================================
// SubKind 4: Airship
//
// Retail SendFlags (from captures):
//   Most common: Position|ClaimStatus|General|Name (2291), Position|ClaimStatus (1537)
//   Also seen:   None, Position|Model, ClaimStatus|General, Model, etc.
//
// Fields:
//   DoorId:  Numeric transport ID from `transport` table (NOT ASCII like doors/elevators)
//   Time:    Vana'diel timestamp (transport start/arrival time)
//   EndTime: Always 0 in retail
// ============================================================================

Airship::Airship([[maybe_unused]] const sendflags_t SendFlg, CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Retail SendFlags (from captures):
    //   Spawn/Update: None (0x00), Position (0x01), or Position|ClaimStatus|General (0x07)
    //   Despawn:      Despawn (0x20)
    // Name and Model flags are NEVER used for airships.

    packet.SubKind = static_cast<uint16_t>(SubKind::Airship);

    // DoorId comes from the npc_list.name field (varbinary).
    // For airships, this is stored as a single byte like 0x06.
    // Retail shows entity 17784936 always has door_id=6, matching npc_list.name=0x06.
    const auto& name = PNpc->getName();
    if (!name.empty())
    {
        packet.DoorId = static_cast<uint32_t>(static_cast<uint8_t>(name[0]));
    }
    else
    {
        packet.DoorId = 0;
    }
    packet.Time    = PNpc->GetLocalVar("TransportTimestamp");
    packet.EndTime = 0; // Always 0 in retail

    // Fixed size: 64 bytes (retail verified)
    setSize(0x40);
}

// ============================================================================
// SubKind 5: MiscNpc - Only handles Model/Name data
// ============================================================================

MiscNpc::MiscNpc(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    packet.SubKind = static_cast<uint16_t>(SubKind::MiscNpc);
    packet.ModelId = PNpc->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

// ============================================================================
// SubKind 6: Automaton - Only handles Model/Name data
// ============================================================================

Automaton::Automaton(const sendflags_t SendFlg, const CPetEntity* PPet)
{
    auto& packet = this->data();

    // Always set SubKind and ModelId - field is always present in packet
    packet.SubKind = static_cast<uint16_t>(SubKind::Automaton);
    packet.ModelId = PPet->GetModelId();

    if (SendFlg.Name)
    {
        const auto& name = PPet->packetName.empty() ? PPet->getName() : PPet->packetName;
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name)));
}

// ============================================================================
// SubKind 7: EquippedMisc - Only handles Model/Name data
// ============================================================================

EquippedMisc::EquippedMisc(const sendflags_t SendFlg, const CNpcEntity* PNpc)
{
    auto& packet = this->data();

    // Retail: GrapIDTbl is only populated when Model flag is set
    packet.SubKind = static_cast<uint16_t>(SubKind::EquippedMisc);
    if (SendFlg.Model)
    {
        // Skip look.size - GrapIDTbl starts at modelid (face/race)
        std::memcpy(packet.GrapIDTbl, &PNpc->look.modelid, sizeof(packet.GrapIDTbl));
    }

    if (SendFlg.Name)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    // Size: Header + CommonData + GrapIDTbl[9] + Name (variable)
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) * 9 + calcNameSize(packet.Name, sizeof(packet.Name)));
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

            case TYPE_SHIP:
                return Airship(SendFlg, static_cast<CNpcEntity*>(PEntity));

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
    std::visit(
        [&](auto& pkt)
        {
            using T    = std::decay_t<decltype(pkt)>;
            auto& data = *reinterpret_cast<typename T::PacketData*>(static_cast<uint8*>(pkt) + sizeof(GP_SERV_HEADER));

            // Always set identity
            data.UniqueNo = PEntity->id;
            data.ActIndex = PEntity->targid;
            data.SendFlg  = SendFlg;

            if (SendFlg.Despawn)
            {
                // PetKillFlag is set when a trust is being despawned/killed
                if (PEntity->objtype == TYPE_TRUST)
                {
                    data.Flags3.unknown_0_3 = true;
                }
                pkt.setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData));
                return;
            }

            if (SendFlg.Position)
            {
                data.dir                = PEntity->loc.p.rotation;
                data.x                  = PEntity->loc.p.x;
                data.z                  = PEntity->loc.p.y; // FFXI swaps y/z
                data.y                  = PEntity->loc.p.z;
                data.Flags0.MovTime     = PEntity->loc.p.moving;
                data.Flags0.RunMode     = PEntity->loc.p.flags.RunMode;
                data.Flags0.unknown_1_6 = PEntity->loc.p.flags.unknown;
                data.Flags0.GroundFlag  = PEntity->loc.p.flags.GroundFlag;
                data.Speed              = PEntity->GetSpeed();
                data.SpeedBase          = PEntity->animationSpeed;

                // Ships use KingFlag instead of facetarget
                if (PEntity->objtype == TYPE_SHIP ||
                    (PEntity->objtype == TYPE_NPC && static_cast<ModelType>(PEntity->look.size) == ModelType::Ship))
                {
                    data.Flags0.KingFlag = true;
                }
                else
                {
                    data.Flags0.facetarget = PEntity->m_TargID;
                }
            }

            // Retail: MonsterFlag is ALWAYS set for mobs, even in position-only updates
            // PetFlag (triggerable) and TargetOffFlag are also always set
            // HideFlag (Flags1 bit 1) controls ??? name display for burrowed worms/antlions
            data.Flags1.HideFlag = PEntity->GetHideFlag();
            // Flags3 bit 27 (unknown_3_3) - name visibility via hideName()
            data.Flags3.unknown_3_3 = PEntity->IsNameHidden();

            switch (PEntity->objtype)
            {
                case TYPE_MOB:
                {
                    auto* PMob                = static_cast<CMobEntity*>(PEntity);
                    data.Flags1.MonsterFlag   = true;
                    data.Flags1.TargetOffFlag = PMob->GetUntargetable();
                    data.Flags3.MonStat       = PMob->animationsub;
                    data.Flags2.NamedFlag     = PMob->render.hasNamedFlag();
                    break;
                }
                case TYPE_PET:
                {
                    auto* PPet = static_cast<CPetEntity*>(PEntity);
                    // Pets do NOT have MonsterFlag set (retail verified)
                    data.Flags1.TargetOffFlag  = PPet->GetUntargetable();
                    data.Flags1.CliPosInitFlag = true;
                    data.Flags3.MonStat        = PPet->animationsub;
                    data.Flags3.PetNewFlag     = true;
                    data.Flags2.NamedFlag      = PPet->render.hasNamedFlag();

                    // CharmFlag is set when pet has a PC master (affects name color display)
                    if (PPet->PMaster && PPet->PMaster->objtype == TYPE_PC)
                    {
                        data.Flags2.CharmFlag = true;
                    }
                    break;
                }
                case TYPE_TRUST:
                {
                    auto* PTrust              = static_cast<CTrustEntity*>(PEntity);
                    data.Flags1.MonsterFlag   = true;
                    data.Flags1.TargetOffFlag = PTrust->GetUntargetable();
                    data.Flags3.MonStat       = PTrust->animationsub;
                    data.Flags2.NamedFlag     = PTrust->render.hasNamedFlag();
                    break;
                }
                case TYPE_NPC:
                {
                    auto* PNpc                 = static_cast<CNpcEntity*>(PEntity);
                    data.Flags3.PetFlag        = PNpc->IsTriggerable();
                    data.Flags1.TargetOffFlag  = PNpc->GetUntargetable();
                    data.Flags1.CliPosInitFlag = true; // Required for doors/NPCs per retail
                    data.Flags3.MonStat        = PNpc->animationsub;
                    break;
                }
                default:
                    break;
            }

            // Trust-specific flags (always set)
            if (PEntity->objtype == TYPE_TRUST)
            {
                data.Flags3.TrustFlag  = true;
                data.Flags3.PetNewFlag = true;
                data.Flags3.PetFlag    = true;
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
            }

            if (SendFlg.ClaimStatus)
            {
                if (PEntity->objtype == TYPE_MOB || PEntity->objtype == TYPE_PET || PEntity->objtype == TYPE_TRUST)
                {
                    data.BtTargetID = static_cast<CMobEntity*>(PEntity)->m_OwnerID.id;
                }
            }
        },
        packet);

    return packet;
}

} // namespace GP_SERV_COMMAND_CHAR_NPC
