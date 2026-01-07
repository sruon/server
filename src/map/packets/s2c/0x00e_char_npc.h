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

#pragma once

#include "0x00d_char_pc.h"
#include "enums/model_type.h"

#include <variant>

class CBaseEntity;
class CMobEntity;
class CNpcEntity;
class CPetEntity;
class CTrustEntity;

// https://github.com/atom0s/XiPackets/tree/main/world/server/0x000E
namespace GP_SERV_COMMAND_CHAR_NPC
{
// SubKind values for NPC entities
enum class SubKind : uint8_t
{
    FixedModel   = 0, // Fixed-model NPC (uint16_t model_id)
    Equipped     = 1, // Equipped NPC (uint16_t GrapIDTbl[9] if Model flag)
    Door         = 2, // Door (uint16_t unused; uint32_t DoorId)
    Elevator     = 3, // Elevator/Movable (uint16_t unused; uint32_t DoorId; uint32_t Time; uint32_t EndTime)
    Airship      = 4, // Airship/Boat (uint16_t unused; uint32_t DoorId; uint32_t Time; uint32_t EndTime)
    MiscNpc      = 5, // Misc NPC (uint16_t model_id)
    Automaton    = 6, // Automaton/Misc (uint16_t model_id)
    EquippedMisc = 7, // Equipped Misc (uint16_t GrapIDTbl[9] if Model flag)
};

// Common fixed portion of packet (offset 0x04 to 0x31)
#pragma pack(push, 1)
struct CommonData
{
    uint32_t    UniqueNo;      // PS2: UniqueNo (offset 0x04)
    uint16_t    ActIndex;      // PS2: ActIndex (offset 0x08)
    sendflags_t SendFlg;       // PS2: SendFlg (offset 0x0A)
    uint8_t     dir;           // PS2: dir (offset 0x0B)
    float       x;             // PS2: x (offset 0x0C)
    float       z;             // PS2: z (offset 0x10)
    float       y;             // PS2: y (offset 0x14)
    flags0_t    Flags0;        // PS2: <bits> (offset 0x18)
    uint8_t     Speed;         // PS2: Speed (offset 0x1C)
    uint8_t     SpeedBase;     // PS2: SpeedBase (offset 0x1D)
    uint8_t     Hpp;           // PS2: HpMax (offset 0x1E)
    uint8_t     server_status; // PS2: server_status (offset 0x1F)
    flags1_t    Flags1;        // PS2: <bits> (offset 0x20)
    flags2_t    Flags2;        // PS2: <bits> (offset 0x24)
    flags3_t    Flags3;        // PS2: <bits> (offset 0x28)
    uint32_t    BtTargetID;    // PS2: BtTargetID (offset 0x2C)
    uint16_t    SubKind : 3;   // PS2: SubKind (offset 0x30)
    uint16_t    Status : 13;   // PS2: Status (offset 0x30)
};

// ============================================================================
// SubKind 0: FixedModel - Used by mobs, pets, trusts, simple NPCs
// ============================================================================
class FixedModel final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, FixedModel>
{
public:
    struct PacketData : CommonData
    {
        uint16_t ModelId;      // Model ID (offset 0x32)
        uint8_t  Name[16];     // Entity name (offset 0x34)
    };

    FixedModel() = default;
    FixedModel(const sendflags_t SendFlg, const CMobEntity* PMob);
    FixedModel(const sendflags_t SendFlg, const CPetEntity* PPet);
    FixedModel(const sendflags_t SendFlg, const CTrustEntity* PTrust);
    FixedModel(const sendflags_t SendFlg, const CNpcEntity* PNpc);
};

// ============================================================================
// SubKind 1: Equipped - NPCs that look like players with equipment
// ============================================================================
class Equipped final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, Equipped>
{
public:
    struct PacketData : CommonData
    {
        uint16_t GrapIDTbl[9]; // Equipment slots (offset 0x32)
        uint8_t  Name[16];     // Entity name (offset 0x44)
    };

    Equipped(const sendflags_t SendFlg, const CNpcEntity* PNpc);
};

// ============================================================================
// SubKind 2: Door
// ============================================================================
class Door final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, Door>
{
public:
    struct PacketData : CommonData
    {
        uint16_t Unused;       // Unused (offset 0x32)
        uint32_t DoorId;       // Door ID (offset 0x34)
        uint8_t  Name[12];     // Entity name (offset 0x38)
    };

    Door(const sendflags_t SendFlg, CNpcEntity* PNpc);
};

// ============================================================================
// SubKind 3: Elevator - Elevators and movable platforms
// ============================================================================
class Elevator final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, Elevator>
{
public:
    struct PacketData : CommonData
    {
        uint16_t Unused;       // Unused (offset 0x32)
        uint32_t DoorId;       // Door/entity ID (offset 0x34)
        uint32_t Time;         // Animation start time (offset 0x38)
        uint32_t EndTime;      // Animation end time (offset 0x3C)
    };

    Elevator(const sendflags_t SendFlg, CNpcEntity* PNpc);
};

// ============================================================================
// SubKind 4: Airship - Airships and boats
// ============================================================================
class Airship final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, Airship>
{
public:
    struct PacketData : CommonData
    {
        uint16_t Unused;       // Unused (offset 0x32)
        uint32_t DoorId;       // Door/entity ID (offset 0x34)
        uint32_t Time;         // Animation start time (offset 0x38)
        uint32_t EndTime;      // Animation end time (offset 0x3C)
    };

    Airship(const sendflags_t SendFlg, CNpcEntity* PNpc);
};

// ============================================================================
// SubKind 5: MiscNpc - Miscellaneous NPCs
// ============================================================================
class MiscNpc final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, MiscNpc>
{
public:
    struct PacketData : CommonData
    {
        uint16_t ModelId;      // Model ID with 0x9000 flag (offset 0x32)
        uint8_t  Name[16];     // Entity name (offset 0x34)
    };

    MiscNpc(const sendflags_t SendFlg, const CNpcEntity* PNpc);
};

// ============================================================================
// SubKind 6: Automaton - Puppet master automatons
// ============================================================================
class Automaton final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, Automaton>
{
public:
    struct PacketData : CommonData
    {
        uint16_t ModelId;      // Model ID with 0x9000 flag (offset 0x32)
        uint8_t  Name[16];     // Entity name (offset 0x34)
    };

    Automaton(const sendflags_t SendFlg, const CPetEntity* PPet);
};

// ============================================================================
// SubKind 7: EquippedMisc - Equipped miscellaneous NPCs
// ============================================================================
class EquippedMisc final : public GP_SERV_PACKET<PacketS2C::GP_SERV_COMMAND_CHAR_NPC, EquippedMisc>
{
public:
    struct PacketData : CommonData
    {
        uint16_t GrapIDTbl[9]; // Equipment slots (offset 0x32)
        uint8_t  Name[16];     // Entity name (offset 0x44)
    };

    EquippedMisc(const sendflags_t SendFlg, const CNpcEntity* PNpc);
};
#pragma pack(pop)

// ============================================================================
// Factory function: Creates the appropriate packet based on entity type
// ============================================================================
using CharNpcPacket = std::variant<
    FixedModel,
    Equipped,
    Door,
    Elevator,
    Airship,
    MiscNpc,
    Automaton,
    EquippedMisc>;

auto create(const sendflags_t SendFlg, CBaseEntity* PEntity) -> CharNpcPacket;

} // namespace GP_SERV_COMMAND_CHAR_NPC
