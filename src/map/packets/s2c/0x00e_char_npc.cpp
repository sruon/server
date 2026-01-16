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

#include "common/lua.h"
#include "common/utils.h"
#include "packets/basic.h"
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

// Calculate padded name size (4-byte aligned) per retail analysis
// With Name flag: minimum 16 bytes, then 20, 24, etc based on length
// Without Name flag: retail uses just 4 bytes (padding only, no name data)
size_t calcNameSize(const uint8_t* nameField, const size_t maxLen, bool hasNameFlag)
{
    if (!hasNameFlag)
    {
        // Retail NPC updates without Name flag use 56 bytes total
        // Header(4) + CommonData(46) + ModelId(2) + 4 = 56
        return 4;
    }

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
    packet.Flags1.GraphSize       = PNpc->modelSize;
    packet.Flags2.g               = static_cast<uint8_t>(PNpc->modelHitboxSize * 10);
    packet.Flags3.CliPriorityFlag = PNpc->priorityRender;
    packet.Flags3.BallistaTeam    = static_cast<uint8_t>(PNpc->allegiance);
    packet.Flags3.MonStat         = PNpc->animationsub;
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

    // Size: Header + CommonData + ModelId + Name (variable, or 4-byte padding without Name flag)
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    // Retail uses Name2 (0x40) for equipped NPCs, not Name (0x08)
    // Client uses Name2 to lookup name from DAT files by UniqueNo for NPCs (ActIndex 0x400-0x6FF)
    if (SendFlg.Name2)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    // Size: Header + CommonData + GrapIDTbl[9] + Name (variable, or 4-byte padding without Name2 flag)
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) * 9 + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name2));
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
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + sizeof(uint32_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name));
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

    // Retail uses Name2 (0x40) for GrapIdTbl types, not Name (0x08)
    if (SendFlg.Name2)
    {
        const auto& name = PNpc->getName();
        std::memcpy(packet.Name, name.c_str(), std::min<size_t>(name.size() + 1, sizeof(packet.Name)));
    }

    // Size: Header + CommonData + GrapIDTbl[9] + Name (variable, or 4-byte padding without Name2 flag)
    setSize(sizeof(GP_SERV_HEADER) + sizeof(CommonData) + sizeof(uint16_t) * 9 + calcNameSize(packet.Name, sizeof(packet.Name), SendFlg.Name2));
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

                // Ships and Elevators use KingFlag instead of facetarget
                if (PEntity->objtype == TYPE_SHIP ||
                    (PEntity->objtype == TYPE_NPC &&
                        (static_cast<ModelType>(PEntity->look.size) == ModelType::Ship ||
                         static_cast<ModelType>(PEntity->look.size) == ModelType::Elevator)))
                {
                    data.Flags0.KingFlag = true;
                }
                else
                {
                    data.Flags0.facetarget = PEntity->m_TargID;
                }
            }

            // Retail: CliPosInitFlag is ALWAYS set when General flag is set (100% correlation)
            // This tells the client to initialize/reset entity position state
            if (SendFlg.General)
            {
                data.Flags1.CliPosInitFlag = true;
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
                    data.Flags3.unknown_3_1   = PMob->render.hasAnimOverride();
                    data.Flags3.unknown_3_2   = PMob->render.hasSubAnim3Bit();
                    data.Flags3.unknown_2_3   = PMob->render.hasUnknownLocalization1();
                    data.Flags3.unknown_2_4   = PMob->render.hasUnknownLocalization2();
                    data.Flags3.unknown_3_4   = PMob->render.isCollisionDisabled();
                    data.Flags3.unknown_3_5   = PMob->render.isHPAndNameHidden();
                    data.Flags3.unknown_3_6   = PMob->render.isCompassHidden();
                    data.Flags3.unknown_3_7   = PMob->render.isHalfTransparent();
                    data.Flags2.NamedFlag     = PMob->render.hasNamedFlag();
                    data.Flags2.ShadowFlag    = PMob->render.isShadowHidden();
                    data.Flags2.SingleFlag    = PMob->render.hasSingleFlag();
                    break;
                }
                case TYPE_PET:
                {
                    auto* PPet = static_cast<CPetEntity*>(PEntity);
                    // Pets do NOT have MonsterFlag set (retail verified)
                    data.Flags1.TargetOffFlag = PPet->GetUntargetable();
                    data.Flags3.MonStat       = PPet->animationsub;
                    data.Flags3.PetNewFlag     = true;
                    data.Flags2.NamedFlag      = PPet->render.hasNamedFlag();
                    data.Flags2.ShadowFlag     = PPet->render.isShadowHidden();
                    data.Flags2.SingleFlag     = PPet->render.hasSingleFlag();

                    // CharmFlag is set when pet has a PC master (affects name color display)
                    if (PPet->PMaster && PPet->PMaster->objtype == TYPE_PC)
                    {
                        data.Flags2.CharmFlag = true;
                    }
                    break;
                }
                case TYPE_TRUST:
                {
                    // Retail trust flags (verified from 107K packets):
                    // Flags1: MonsterFlag=0, LfgFlag=1, AnonymousFlag=1
                    // Flags2: byte 0x27 = 0x28 (CharmFlag + NamedFlag)
                    // Flags3: byte 0x28 = 0x41, 0x29 = 0x01, 0x2B = 0x06
                    auto* PTrust              = static_cast<CTrustEntity*>(PEntity);
                    data.Flags1.LfgFlag       = true;
                    data.Flags1.AnonymousFlag  = true;
                    data.Flags1.TargetOffFlag  = PTrust->GetUntargetable();
                    data.Flags1.GraphSize      = PTrust->modelSize;
                    data.Flags3.MonStat        = PTrust->animationsub;
                    data.Flags2.NamedFlag      = true; // Always set for trusts
                    data.Flags2.CharmFlag      = true; // Always set for trusts (PC master)
                    break;
                }
                case TYPE_NPC:
                {
                    auto* PNpc                 = static_cast<CNpcEntity*>(PEntity);
                    data.Flags3.PetFlag        = PNpc->IsTriggerable();
                    data.Flags1.TargetOffFlag  = PNpc->GetUntargetable();
                    data.Flags3.MonStat        = PNpc->animationsub;
                    data.Flags3.unknown_3_1    = PNpc->render.hasAnimOverride();
                    data.Flags3.unknown_3_2    = PNpc->render.hasSubAnim3Bit();
                    data.Flags3.unknown_2_3    = PNpc->render.hasUnknownLocalization1();
                    data.Flags3.unknown_2_4    = PNpc->render.hasUnknownLocalization2();
                    data.Flags3.unknown_3_4    = PNpc->render.isCollisionDisabled();
                    data.Flags3.unknown_3_5    = PNpc->render.isHPAndNameHidden();
                    data.Flags3.unknown_3_6    = PNpc->render.isCompassHidden();
                    data.Flags3.unknown_3_7    = PNpc->render.isHalfTransparent();
                    data.Flags2.NamedFlag      = PNpc->render.hasNamedFlag();
                    data.Flags2.ShadowFlag     = PNpc->render.isShadowHidden();
                    data.Flags2.SingleFlag     = PNpc->render.hasSingleFlag();
                    // Doors always have TalkUcoffFlag set (retail verified)
                    if (static_cast<ModelType>(PNpc->look.size) == ModelType::Door)
                    {
                        data.Flags1.TalkUcoffFlag = true;
                    }
                    // GrapIdTbl types (Equipped, EquippedMisc): Retail uses Name2 (0x40) instead of Name (0x08)
                    // Name and Name2 are mutually exclusive - clear Name when setting Name2
                    if (static_cast<ModelType>(PNpc->look.size) == ModelType::Equipped ||
                        static_cast<ModelType>(PNpc->look.size) == ModelType::Chocobo)
                    {
                        if (data.SendFlg.Name)
                        {
                            data.SendFlg.Name  = false;
                            data.SendFlg.Name2 = true;
                        }
                    }
                    break;
                }
                default:
                    break;
            }

            // Trust-specific flags (retail: byte 0x28=0x41, 0x2B=0x06)
            if (PEntity->objtype == TYPE_TRUST)
            {
                data.Flags3.TrustFlag   = true;
                data.Flags3.PetFlag     = true;
                data.Flags3.unknown_3_1 = true;
                data.Flags3.unknown_3_2 = true;
            }

            // All namevis bits are now handled via EntityRenderContainer (loaded in applyNamevis):
            //   bit 0 (0x01) = VIS_ICON            → render.hasInfoIcon() → MentorFlag
            //   bit 1 (0x02) = VIS_ANIM_OVERRIDE   → render.hasAnimOverride() → unknown_3_1
            //   bit 2 (0x04) = VIS_SUBANIM_3BIT    → render.hasSubAnim3Bit() → unknown_3_2
            //   bit 3 (0x08) = VIS_HIDE_NAME       → render.isNameHidden() → unknown_3_3
            //   bit 4 (0x10) = VIS_NO_COLLISION    → render.isCollisionDisabled() → unknown_3_4
            //   bit 5 (0x20) = VIS_HIDE_HP_AND_NAME → render.isHPAndNameHidden() → unknown_3_5
            //   bit 6 (0x40) = VIS_HIDE_COMPASS    → render.isCompassHidden() → unknown_3_6
            //   bit 7 (0x80) = VIS_GHOST_PHASE     → render.isHalfTransparent() → unknown_3_7
            if (PEntity->objtype == TYPE_NPC || PEntity->objtype == TYPE_MOB)
            {
                data.Flags3.MentorFlag = PEntity->render.hasInfoIcon();
            }

            if (SendFlg.General)
            {
                // Gender is stored in entityFlags bit 7 (0x80): 1=male, 0=female.
                // Loaded via render.applyEntityFlags() and accessed via render.getGender().
                switch (PEntity->objtype)
                {
                    case TYPE_MOB:
                    case TYPE_PET:
                        data.Flags1.Gender = PEntity->render.getGender();
                        break;
                    case TYPE_TRUST:
                    {
                        // Trusts have humanoid races - use the race formula
                        // Race 1-2: Hume M/F, 3-4: Elvaan M/F, 5-6: Tarutaru M/F, 7-8: Mithra/Galka
                        // Formula: (race % 2) ^ (race > 6)
                        // Results: 1(M)=1, 2(F)=0, 3(M)=1, 4(F)=0, 5(M)=1, 6(F)=0, 7(F)=0, 8(M)=1
                        const uint8_t race = PEntity->look.race;
                        if (race >= 1 && race <= 8)
                        {
                            data.Flags1.Gender = (race % 2) ^ (race > 6);
                        }
                        else
                        {
                            data.Flags1.Gender = 1; // Default male for non-standard
                        }
                        break;
                    }
                    case TYPE_NPC:
                        data.Flags1.Gender = PEntity->render.getGender();
                        // TODO: Research needed - retail shows strong correlation between LfgFlag/AnonymousFlag
                        // and Gender for equipped NPCs (always equal, ~74% set). Hacky approximation for now.
                        // Male (Gender=0) → flags=1, Female (Gender=1) → flags=0
                        if (static_cast<ModelType>(PEntity->look.size) == ModelType::Equipped)
                        {
                            data.Flags1.LfgFlag       = !data.Flags1.Gender;
                            data.Flags1.AnonymousFlag = !data.Flags1.Gender;
                        }
                        break;
                    default:
                        data.Flags1.Gender = 1;
                        break;
                }

                // French article flags from entityFlags (loaded via render.applyEntityFlags)
                // LinkShellFlag = French Feminine (la vs le)
                // LinkDeadFlag = French Elision (l')
                data.Flags1.LinkShellFlag = PEntity->render.hasLinkShellFlag();
                data.Flags1.LinkDeadFlag  = PEntity->render.hasLinkDeadFlag();

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

// ============================================================================
// Unpack helpers - common fields shared by all SubKinds
// ============================================================================
namespace
{
auto unpackCommon(const CommonData& common) -> sol::table
{
    auto result = lua.create_table();

    result["UniqueNo"] = common.UniqueNo;
    result["ActIndex"] = common.ActIndex;

    auto sendFlg             = lua.create_table();
    sendFlg["Position"]      = common.SendFlg.Position;
    sendFlg["ClaimStatus"]   = common.SendFlg.ClaimStatus;
    sendFlg["General"]       = common.SendFlg.General;
    sendFlg["Name"]          = common.SendFlg.Name;
    sendFlg["Model"]         = common.SendFlg.Model;
    sendFlg["Despawn"]       = common.SendFlg.Despawn;
    sendFlg["Name2"]         = common.SendFlg.Name2;
    sendFlg["raw"]           = *reinterpret_cast<const uint8_t*>(&common.SendFlg);
    result["SendFlg"]        = sendFlg;

    result["dir"] = common.dir;
    result["x"]             = common.x;
    result["z"]             = common.z;
    result["y"]             = common.y;
    result["Speed"]         = common.Speed;
    result["SpeedBase"]     = common.SpeedBase;
    result["Hpp"]           = common.Hpp;
    result["server_status"] = common.server_status;
    result["BtTargetID"]    = common.BtTargetID;
    result["SubKind"]       = common.SubKind;
    result["Status"]        = common.Status;

    auto flags0            = lua.create_table();
    flags0["MovTime"]      = common.Flags0.MovTime;
    flags0["RunMode"]      = common.Flags0.RunMode;
    flags0["unknown_1_6"]  = common.Flags0.unknown_1_6;
    flags0["GroundFlag"]   = common.Flags0.GroundFlag;
    flags0["KingFlag"]     = common.Flags0.KingFlag;
    flags0["facetarget"]   = common.Flags0.facetarget;
    flags0["raw"]          = *reinterpret_cast<const uint32_t*>(&common.Flags0);
    result["Flags0"]       = flags0;

    auto flags1               = lua.create_table();
    flags1["MonsterFlag"]     = common.Flags1.MonsterFlag;
    flags1["HideFlag"]        = common.Flags1.HideFlag;
    flags1["SleepFlag"]       = common.Flags1.SleepFlag;
    flags1["ChocoboIndex"]    = common.Flags1.ChocoboIndex;
    flags1["CliPosInitFlag"]  = common.Flags1.CliPosInitFlag;
    flags1["GraphSize"]       = common.Flags1.GraphSize;
    flags1["LfgFlag"]         = common.Flags1.LfgFlag;
    flags1["AnonymousFlag"]   = common.Flags1.AnonymousFlag;
    flags1["YellFlag"]        = common.Flags1.YellFlag;
    flags1["AwayFlag"]        = common.Flags1.AwayFlag;
    flags1["Gender"]          = common.Flags1.Gender;
    flags1["PlayOnelineFlag"] = common.Flags1.PlayOnelineFlag;
    flags1["LinkShellFlag"]   = common.Flags1.LinkShellFlag;
    flags1["LinkDeadFlag"]    = common.Flags1.LinkDeadFlag;
    flags1["TargetOffFlag"]   = common.Flags1.TargetOffFlag;
    flags1["TalkUcoffFlag"]   = common.Flags1.TalkUcoffFlag;
    flags1["GmLevel"]         = common.Flags1.GmLevel;
    flags1["HackMove"]        = common.Flags1.HackMove;
    flags1["InvisFlag"]       = common.Flags1.InvisFlag;
    flags1["TurnFlag"]        = common.Flags1.TurnFlag;
    flags1["BazaarFlag"]      = common.Flags1.BazaarFlag;
    flags1["raw"]             = *reinterpret_cast<const uint32_t*>(&common.Flags1);
    result["Flags1"]          = flags1;

    auto flags2              = lua.create_table();
    flags2["r"]              = common.Flags2.r;
    flags2["g"]              = common.Flags2.g;
    flags2["b"]              = common.Flags2.b;
    flags2["PvPFlag"]        = common.Flags2.PvPFlag;
    flags2["ShadowFlag"]     = common.Flags2.ShadowFlag;
    flags2["ShipStartMode"]  = common.Flags2.ShipStartMode;
    flags2["CharmFlag"]      = common.Flags2.CharmFlag;
    flags2["GmIconFlag"]     = common.Flags2.GmIconFlag;
    flags2["NamedFlag"]      = common.Flags2.NamedFlag;
    flags2["SingleFlag"]     = common.Flags2.SingleFlag;
    flags2["AutoPartyFlag"]  = common.Flags2.AutoPartyFlag;
    flags2["raw"]            = *reinterpret_cast<const uint32_t*>(&common.Flags2);
    result["Flags2"]         = flags2;

    auto flags3                  = lua.create_table();
    flags3["TrustFlag"]          = common.Flags3.TrustFlag;
    flags3["LfgMasterFlag"]      = common.Flags3.LfgMasterFlag;
    flags3["PetNewFlag"]         = common.Flags3.PetNewFlag;
    flags3["unknown_0_3"]        = common.Flags3.unknown_0_3;
    flags3["MotStopFlag"]        = common.Flags3.MotStopFlag;
    flags3["CliPriorityFlag"]    = common.Flags3.CliPriorityFlag;
    flags3["PetFlag"]            = common.Flags3.PetFlag;
    flags3["OcclusionoffFlag"]   = common.Flags3.OcclusionoffFlag;
    flags3["BallistaTeam"]       = common.Flags3.BallistaTeam;
    flags3["MonStat"]            = common.Flags3.MonStat;
    flags3["unknown_2_3"]        = common.Flags3.unknown_2_3;
    flags3["unknown_2_4"]        = common.Flags3.unknown_2_4;
    flags3["SilenceFlag"]        = common.Flags3.SilenceFlag;
    flags3["unknown_2_6"]        = common.Flags3.unknown_2_6;
    flags3["NewCharacterFlag"]   = common.Flags3.NewCharacterFlag;
    flags3["MentorFlag"]         = common.Flags3.MentorFlag;
    flags3["unknown_3_1"]        = common.Flags3.unknown_3_1;
    flags3["unknown_3_2"]        = common.Flags3.unknown_3_2;
    flags3["unknown_3_3"]        = common.Flags3.unknown_3_3;
    flags3["unknown_3_4"]        = common.Flags3.unknown_3_4;
    flags3["unknown_3_5"]        = common.Flags3.unknown_3_5;
    flags3["unknown_3_6"]        = common.Flags3.unknown_3_6;
    flags3["unknown_3_7"]        = common.Flags3.unknown_3_7;
    flags3["raw"]                = *reinterpret_cast<const uint32_t*>(&common.Flags3);
    result["Flags3"]             = flags3;

    return result;
}

auto unpackName(const uint8_t* nameData, size_t maxLen) -> std::string
{
    std::string name(reinterpret_cast<const char*>(nameData), maxLen);
    return name.c_str(); // Trim at null
}
} // namespace

// ============================================================================
// SubKind unpack implementations
// ============================================================================

auto FixedModel::unpack() const -> sol::table
{
    auto result      = unpackCommon(data());
    auto dataTable   = lua.create_table();
    dataTable["model_id"] = data().ModelId;
    dataTable["Name"]     = unpackName(data().Name, sizeof(data().Name));
    result["Data"]        = dataTable;
    return result;
}

auto Equipped::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    auto grapTbl   = lua.create_table();
    for (int i = 0; i < 9; ++i)
    {
        grapTbl[i + 1] = data().GrapIDTbl[i];
    }
    dataTable["GrapIDTbl"] = grapTbl;
    dataTable["Name"]      = unpackName(data().Name, sizeof(data().Name));
    result["Data"]         = dataTable;
    return result;
}

auto Door::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    dataTable["DoorId"] = data().DoorId;
    // Note: Door struct has Name[12] but may not be used - verify against retail
    result["Data"] = dataTable;
    return result;
}

auto Elevator::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    dataTable["DoorId"]  = data().DoorId;
    dataTable["Time"]    = data().Time;
    dataTable["EndTime"] = data().EndTime;
    result["Data"]       = dataTable;
    return result;
}

auto Airship::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    dataTable["DoorId"]  = data().DoorId;
    dataTable["Time"]    = data().Time;
    dataTable["EndTime"] = data().EndTime;
    result["Data"]       = dataTable;
    return result;
}

auto MiscNpc::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    dataTable["model_id"] = data().ModelId;
    dataTable["Name"]     = unpackName(data().Name, sizeof(data().Name));
    result["Data"]        = dataTable;
    return result;
}

auto Automaton::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    dataTable["model_id"] = data().ModelId;
    dataTable["Name"]     = unpackName(data().Name, sizeof(data().Name));
    result["Data"]        = dataTable;
    return result;
}

auto EquippedMisc::unpack() const -> sol::table
{
    auto result    = unpackCommon(data());
    auto dataTable = lua.create_table();
    auto grapTbl   = lua.create_table();
    for (int i = 0; i < 9; ++i)
    {
        grapTbl[i + 1] = data().GrapIDTbl[i];
    }
    dataTable["GrapIDTbl"] = grapTbl;
    dataTable["Name"]      = unpackName(data().Name, sizeof(data().Name));
    result["Data"]         = dataTable;
    return result;
}

// ============================================================================
// Reconstruct variant from raw packet
// ============================================================================

auto fromPacket(CBasicPacket& packet) -> CharNpcPacket
{
    // Read SubKind from buffer to determine which type to return
    const auto* data   = static_cast<uint8_t*>(packet);
    const auto* common = reinterpret_cast<const CommonData*>(data + sizeof(GP_SERV_HEADER));

    switch (common->SubKind)
    {
        case 0:
            return *reinterpret_cast<FixedModel*>(&packet);
        case 1:
            return *reinterpret_cast<Equipped*>(&packet);
        case 2:
            return *reinterpret_cast<Door*>(&packet);
        case 3:
            return *reinterpret_cast<Elevator*>(&packet);
        case 4:
            return *reinterpret_cast<Airship*>(&packet);
        case 5:
            return *reinterpret_cast<MiscNpc*>(&packet);
        case 6:
            return *reinterpret_cast<Automaton*>(&packet);
        case 7:
            return *reinterpret_cast<EquippedMisc*>(&packet);
        default:
            return FixedModel{}; // Fallback
    }
}

} // namespace GP_SERV_COMMAND_CHAR_NPC
