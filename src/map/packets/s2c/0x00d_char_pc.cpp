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

#include "0x00d_char_pc.h"

#include "entities/charentity.h"
#include "items/item_linkshell.h"
#include "status_effect.h"
#include "status_effect_container.h"
#include "utils/mountutils.h"

GP_SERV_COMMAND_CHAR_PC::GP_SERV_COMMAND_CHAR_PC(sendflags_t SendFlg, CCharEntity* PChar)
{
    if (!PChar)
    {
        ShowError("GP_SERV_COMMAND_CHAR_PC::GP_SERV_COMMAND_CHAR_PC() - PChar was null.");
        return;
    }

    auto& packet    = this->data();
    packet.UniqueNo = PChar->id;
    packet.ActIndex = PChar->targid;
    packet.SendFlg  = SendFlg;

    // Unconditional fields (no SendFlg required, always read by client)
    packet.CostumeId             = PChar->m_Costume;
    packet.BallistaInfo          = 0; // TODO: Ballista scoring info
    packet.Flags4.TrialFlag      = 0; // Trial accounts not implemented.
    packet.Flags4.JobMasterFlag  = PChar->getMod(Mod::SUPERIOR_LEVEL) == 5 && PChar->m_jobMasterDisplay;
    packet.Flags5.GeoIndiSize    = 1;
    packet.Flags5.GeoIndiFlag    = 0;
    packet.Flags5.GeoIndiElement = 0;
    if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_COLURE_ACTIVE))
    {
        packet.Flags5.GeoIndiElement = PChar->StatusEffectContainer->GetStatusEffect(EFFECT_COLURE_ACTIVE)->GetPower();
        packet.Flags5.GeoIndiFlag    = 1;
    }

    if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_WIDENED_COMPASS))
    {
        packet.Flags5.GeoIndiSize = 2;
    }

    packet.ModelHitboxSize   = static_cast<uint8_t>(PChar->modelHitboxSize * 10);
    packet.Flags6.GateId     = 0; // Confrontation zone ID
    packet.Flags6.MountIndex = PChar->m_mountId;
    if (PChar->PPet)
    {
        packet.PetActIndex = PChar->PPet->targid;
    }

    // Position (0x01)
    if (SendFlg.Position)
    {
        packet.dir = PChar->loc.p.rotation;
        packet.x   = PChar->loc.p.x;
        packet.y   = PChar->loc.p.z; // Intentionally swapped
        packet.z   = PChar->loc.p.y; // Intentionally swapped

        packet.Speed     = PChar->UpdateSpeed();
        packet.SpeedBase = PChar->animationSpeed;

        packet.Flags0.MovTime     = PChar->loc.p.moving;
        packet.Flags0.RunMode     = 0;
        packet.Flags0.unknown_1_6 = 0;
        packet.Flags0.GroundFlag  = PChar->wallhackEnabled;
        packet.Flags0.KingFlag    = 0;
        packet.Flags0.facetarget  = PChar->m_TargID;
    }

    // General (0x04)
    if (SendFlg.General)
    {
        const auto [ChocoboIndex, CustomProperties] = mountutils::packetDefinition(PChar);

        packet.Hpp           = PChar->GetHPP();
        packet.server_status = PChar->animation;

        packet.Flags1.MonsterFlag     = false;
        packet.Flags1.HideFlag        = false;
        packet.Flags1.SleepFlag       = 0;
        packet.Flags1.unknown_0_3     = PChar->loc.zone ? PChar->loc.zone->CanUseMisc(MISC_TREASURE) : 0;
        packet.Flags1.unknown_0_4     = 0;
        packet.Flags1.ChocoboIndex    = ChocoboIndex;
        packet.Flags1.CliPosInitFlag  = 0;
        packet.Flags1.GraphSize       = PChar->look.size;
        packet.Flags1.LfgFlag         = PChar->playerConfig.InviteFlg;
        packet.Flags1.AnonymousFlag   = PChar->playerConfig.AnonymityFlg;
        packet.Flags1.YellFlag        = 0;
        packet.Flags1.AwayFlag        = PChar->playerConfig.AwayFlg;
        packet.Flags1.Gender          = PChar->GetGender();
        packet.Flags1.PlayOnelineFlag = 0;
        packet.Flags1.TargetOffFlag   = 0;
        packet.Flags1.TalkUcoffFlag   = 0;
        packet.Flags1.GmLevel         = PChar->visibleGmLevel;
        packet.Flags1.HackMove        = PChar->wallhackEnabled;
        packet.Flags1.InvisFlag       = PChar->m_isGMHidden || PChar->StatusEffectContainer->HasStatusEffectByFlag(EFFECTFLAG_INVISIBLE);
        packet.Flags1.TurnFlag        = 0;
        packet.Flags1.BazaarFlag      = PChar->hasBazaar();
        packet.Flags1.LinkDeadFlag    = PChar->isLinkDead;

        auto* linkshell             = reinterpret_cast<CItemLinkshell*>(PChar->getEquip(SLOT_LINK1));
        packet.Flags1.LinkShellFlag = linkshell ? true : false;
        if (linkshell && linkshell->isType(ITEM_LINKSHELL))
        {
            const lscolor_t LSColor = linkshell->GetLSColor();
            packet.Flags2.r         = (LSColor.R << 4) + 15;
            packet.Flags2.g         = (LSColor.G << 4) + 15;
            packet.Flags2.b         = (LSColor.B << 4) + 15;
        }

        packet.Flags2.PvPFlag       = 0;
        packet.Flags2.ShadowFlag    = 0;
        packet.Flags2.CharmFlag     = PChar->isCharmed;
        packet.Flags2.GmIconFlag    = false;
        packet.Flags2.NamedFlag     = 0;
        packet.Flags2.SingleFlag    = 0;
        packet.Flags2.AutoPartyFlag = false;

        packet.Flags3.TrustFlag        = 0;
        packet.Flags3.LfgMasterFlag    = 0;
        packet.Flags3.PetNewFlag       = 0;
        packet.Flags3.MotStopFlag      = PChar->StatusEffectContainer->HasStatusEffect(EFFECT_TERROR);
        packet.Flags3.CliPriorityFlag  = PChar->priorityRender;
        packet.Flags3.PetFlag          = 0;
        packet.Flags3.BallistaTeam     = static_cast<uint8_t>(PChar->allegiance);
        packet.Flags3.MonStat          = 0;
        packet.Flags3.SilenceFlag      = PChar->m_isGMHidden || PChar->StatusEffectContainer->HasStatusEffect(EFFECT_SNEAK);
        packet.Flags3.NewCharacterFlag = !PChar->playerConfig.NewAdventurerOffFlg;
        packet.Flags3.MentorFlag       = PChar->playerConfig.MentorFlg;

        // Chocobo customization (only used by personal/noble chocobos)
        packet.CustomProperties[0] = CustomProperties[0];
        packet.CustomProperties[1] = CustomProperties[1];
    }

    // --------------------------------
    // Model (0x10)
    // --------------------------------
    if (SendFlg.Model)
    {
        look_t* look = PChar->getStyleLocked() ? &PChar->mainlook : &PChar->look;

        packet.GrapIDTbl[0] = look->modelid;
        packet.GrapIDTbl[1] = PChar->playerConfig.DisplayHeadOffFlg ? 0x0 : look->head + 0x1000;
        packet.GrapIDTbl[2] = look->body + 0x2000;
        packet.GrapIDTbl[3] = look->hands + 0x3000;
        packet.GrapIDTbl[4] = look->legs + 0x4000;
        packet.GrapIDTbl[5] = look->feet + 0x5000;
        packet.GrapIDTbl[6] = look->main + 0x6000;
        packet.GrapIDTbl[7] = look->sub + 0x7000;
        packet.GrapIDTbl[8] = look->ranged + 0x8000;

        if (PChar->m_Costume2 != 0)
        {
            packet.GrapIDTbl[0] = look->race << 8 | PChar->m_Costume2;
            packet.GrapIDTbl[8] = 0xFFFF;
        }

        if (PChar->m_PMonstrosity != nullptr)
        {
            packet.MonstrosityFlags   = 0x8000 | PChar->m_PMonstrosity->Species;
            packet.MonstrosityNameId1 = PChar->m_PMonstrosity->NamePrefix1;
            packet.MonstrosityNameId2 = PChar->m_PMonstrosity->NamePrefix2;

            packet.GrapIDTbl[0] = PChar->m_PMonstrosity->Look;
            packet.GrapIDTbl[8] = 0xFFFF;

            if (PChar->m_PMonstrosity->Belligerency && PChar->loc.zone->GetID() != ZONE_FERETORY)
            {
                packet.Flags3.BallistaTeam |= 0x08;
            }
        }
    }

    // --------------------------------
    // Name (0x08)
    // --------------------------------
    if (SendFlg.Name)
    {
        const char* charName     = PChar->getName().c_str();
        size_t      charNameSize = strlen(charName);
        std::memcpy(packet.name, charName, std::min(sizeof(packet.name), charNameSize));
    }

    // --------------------------------
    // Packet sizing (based on retail captures)
    // --------------------------------
    // From 900k+ packet analysis:
    // - Despawn: always 92 bytes
    // - All others: 92 + name_len (consistent with spawn packet size)
    //
    // Retail keeps the same packet size for all updates of an entity,
    // based on the character's name length from their spawn packet.
    constexpr size_t despawnSize = 92;
    constexpr size_t nameBase    = 92;

    if (SendFlg.Despawn)
    {
        this->setSize(despawnSize);
    }
    else
    {
        this->setSize(nameBase + PChar->getName().size());
    }

    // ClaimStatus (0x02) - unused for PCs
    // packet.BtTargetID = 0;

    // --------------------------------
    // Unknown/Unverified fields
    // --------------------------------
    // packet.Flags0.RunMode      - Unknown purpose, always 0
    // packet.Flags1.SleepFlag    - Event/cutscene related?
    // packet.Flags3.CliPriorityFlag - Force render priority, unverified
    // packet.Flags3.LfgMasterFlag   - LFP as job master, not implemented
}
