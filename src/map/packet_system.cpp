/*
===========================================================================

  Copyright (c) 2010-2015 Darkstar Dev Teams

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

#include "common/async.h"
#include "common/blowfish.h"
#include "common/database.h"
#include "common/logging.h"
#include "common/mmo.h"
#include "common/task_manager.h"
#include "common/timer.h"
#include "common/utils.h"

#include <cstring>
#include <utility>

#include "alliance.h"
#include "enmity_container.h"
#include "ipc_client.h"
#include "item_container.h"
#include "latent_effect_container.h"
#include "linkshell.h"
#include "map_networking.h"
#include "map_server.h"
#include "map_session.h"
#include "monstrosity.h"
#include "packet_system.h"

#include "items.h"
#include "party.h"
#include "recast_container.h"
#include "roe.h"
#include "spell.h"
#include "status_effect_container.h"
#include "trade_container.h"
#include "zone.h"

#include "ai/ai_container.h"

#include "entities/charentity.h"
#include "entities/mobentity.h"
#include "entities/trustentity.h"

#include "items/item_shop.h"

#include "lua/luautils.h"

#include "packets/basic.h"
#include "packets/c2s/0x00c_gameok.h"
#include "packets/c2s/0x00d_netend.h"
#include "packets/c2s/0x00f_clstat.h"
#include "packets/c2s/0x011_zone_transition.h"
#include "packets/c2s/0x015_pos.h"
#include "packets/c2s/0x016_charreq.h"
#include "packets/c2s/0x017_charreq2.h"
#include "packets/c2s/0x01b_friendpass.h"
#include "packets/c2s/0x01c_unknown.h"
#include "packets/c2s/0x01f_gmcommand.h"
#include "packets/c2s/0x028_item_dump.h"
#include "packets/c2s/0x029_item_move.h"
#include "packets/c2s/0x02b_translate.h"
#include "packets/c2s/0x02c_itemsearch.h"
#include "packets/c2s/0x032_item_trade_req.h"
#include "packets/c2s/0x033_item_trade_res.h"
#include "packets/c2s/0x034_item_trade_list.h"
#include "packets/c2s/0x036_item_transfer.h"
#include "packets/c2s/0x037_item_use.h"
#include "packets/c2s/0x03a_item_stack.h"
#include "packets/c2s/0x03b_mannequin_set.h"
#include "packets/c2s/0x03c_black_list.h"
#include "packets/c2s/0x03d_black_edit.h"
#include "packets/c2s/0x041_trophy_entry.h"
#include "packets/c2s/0x042_trophy_absence.h"
#include "packets/c2s/0x04b_fragments.h"
#include "packets/c2s/0x04d_pbx.h"
#include "packets/c2s/0x04e_auc.h"
#include "packets/c2s/0x050_equip_set.h"
#include "packets/c2s/0x051_equipset_set.h"
#include "packets/c2s/0x052_equipset_check.h"
#include "packets/c2s/0x053_lockstyle.h"
#include "packets/c2s/0x058_recipe.h"
#include "packets/c2s/0x059_effectend.h"
#include "packets/c2s/0x05a_reqconquest.h"
#include "packets/c2s/0x05b_eventend.h"
#include "packets/c2s/0x05c_eventendxzy.h"
#include "packets/c2s/0x05d_motion.h"
#include "packets/c2s/0x05e_maprect.h"
#include "packets/c2s/0x060_passwards.h"
#include "packets/c2s/0x061_clistatus.h"
#include "packets/c2s/0x063_dig.h"
#include "packets/c2s/0x064_scenarioitem.h"
#include "packets/c2s/0x066_fishing.h"
#include "packets/c2s/0x070_group_breakup.h"
#include "packets/c2s/0x071_group_strike.h"
#include "packets/c2s/0x074_group_solicit_res.h"
#include "packets/c2s/0x076_group_list_req.h"
#include "packets/c2s/0x077_group_change2.h"
#include "packets/c2s/0x078_group_checkid.h"
#include "packets/c2s/0x083_shop_buy.h"
#include "packets/c2s/0x084_shop_sell_req.h"
#include "packets/c2s/0x085_shop_sell_set.h"
#include "packets/c2s/0x096_combine_ask.h"
#include "packets/c2s/0x09b_chocobo_race_req.h"
#include "packets/c2s/0x0a0_switch_proposal.h"
#include "packets/c2s/0x0a1_switch_vote.h"
#include "packets/c2s/0x0a2_dice.h"
#include "packets/c2s/0x0aa_guild_buy.h"
#include "packets/c2s/0x0ab_guild_buylist.h"
#include "packets/c2s/0x0ac_guild_sell.h"
#include "packets/c2s/0x0ad_guild_selllist.h"
#include "packets/c2s/0x0b5_chat_std.h"
#include "packets/c2s/0x0b6_chat_name.h"
#include "packets/c2s/0x0b7_assist_channel.h"
#include "packets/c2s/0x0be_merits.h"
#include "packets/c2s/0x0bf_job_points_spend.h"
#include "packets/c2s/0x0c0_job_points_req.h"
#include "packets/c2s/0x0d2_map_group.h"
#include "packets/c2s/0x0d3_faq_gmcall.h"
#include "packets/c2s/0x0d4_faq_gmparam.h"
#include "packets/c2s/0x0d5_ack_gmmsg.h"
#include "packets/c2s/0x0d8_dungeon_param.h"
#include "packets/c2s/0x0db_config_language.h"
#include "packets/c2s/0x0dc_config.h"
#include "packets/c2s/0x0dd_equip_inspect.h"
#include "packets/c2s/0x0de_inspect_message.h"
#include "packets/c2s/0x0e0_set_usermsg.h"
#include "packets/c2s/0x0e1_get_lsmsg.h"
#include "packets/c2s/0x0e2_set_lsmsg.h"
#include "packets/c2s/0x0e4_get_lspriv.h"
#include "packets/c2s/0x0e7_reqlogout.h"
#include "packets/c2s/0x0e8_camp.h"
#include "packets/c2s/0x0ea_sit.h"
#include "packets/c2s/0x0eb_reqsubmapnum.h"
#include "packets/c2s/0x0f0_rescue.h"
#include "packets/c2s/0x0f1_buffcancel.h"
#include "packets/c2s/0x0f2_submapchange.h"
#include "packets/c2s/0x0f4_tracking_list.h"
#include "packets/c2s/0x0f5_tracking_start.h"
#include "packets/c2s/0x0f6_tracking_end.h"
#include "packets/c2s/0x0fa_myroom_layout.h"
#include "packets/c2s/0x0fb_myroom_bankin.h"
#include "packets/c2s/0x0fc_myroom_plant_add.h"
#include "packets/c2s/0x0fd_myroom_plant_check.h"
#include "packets/c2s/0x0fe_myroom_plant_crop.h"
#include "packets/c2s/0x0ff_myroom_plant_stop.h"
#include "packets/c2s/0x100_myroom_job.h"
#include "packets/c2s/0x102_extended_job.h"
#include "packets/c2s/0x104_bazaar_exit.h"
#include "packets/c2s/0x105_bazaar_list.h"
#include "packets/c2s/0x106_bazaar_buy.h"
#include "packets/c2s/0x109_bazaar_open.h"
#include "packets/c2s/0x10a_bazaar_itemset.h"
#include "packets/c2s/0x10b_bazaar_close.h"
#include "packets/c2s/0x10c_roe_start.h"
#include "packets/c2s/0x10d_roe_remove.h"
#include "packets/c2s/0x10e_roe_claim.h"
#include "packets/c2s/0x10f_currencies_1.h"
#include "packets/c2s/0x110_fishing_2.h"
#include "packets/c2s/0x113_sitchair.h"
#include "packets/c2s/0x114_map_markers.h"
#include "packets/c2s/0x115_currencies_2.h"
#include "packets/c2s/0x116_unity_menu.h"
#include "packets/c2s/0x117_unity_quest.h"
#include "packets/c2s/0x118_unity_toggle.h"
#include "packets/c2s/0x119_emote_list.h"
#include "packets/c2s/0x11b_mastery_display.h"
#include "packets/c2s/0x11c_party_request.h"
#include "packets/c2s/0x11d_jump.h"
#include "packets/char_recast.h"
#include "packets/char_status.h"
#include "packets/chocobo_digging.h"
#include "packets/downloading_data.h"
#include "packets/inventory_assign.h"
#include "packets/inventory_finish.h"
#include "packets/inventory_item.h"
#include "packets/linkshell_equip.h"
#include "packets/menu_jobpoints.h"
#include "packets/message_basic.h"
#include "packets/message_standard.h"
#include "packets/message_system.h"
#include "packets/party_invite.h"
#include "packets/release.h"
#include "packets/roe_questlog.h"
#include "packets/roe_sparkupdate.h"
#include "packets/roe_update.h"
#include "packets/trade_update.h"
#include "packets/zone_in.h"
#include "packets/zone_visited.h"

#include "utils/battleutils.h"
#include "utils/blacklistutils.h"
#include "utils/charutils.h"
#include "utils/fishingutils.h"
#include "utils/gardenutils.h"
#include "utils/itemutils.h"
#include "utils/jailutils.h"
#include "utils/zoneutils.h"

uint8 PacketSize[512];

std::function<void(MapSession* const, CCharEntity* const, CBasicPacket&)> PacketParser[512];

/************************************************************************
 *                                                                       *
 *  Display the contents of the incoming packet to the console.          *
 *                                                                       *
 ************************************************************************/

void PrintPacket(CBasicPacket& packet)
{
    std::string message;

    for (std::size_t idx = 0U; idx < packet.getSize(); idx++)
    {
        uint8 byte = *packet[idx];
        message.append(fmt::format("{:02x} ", byte));

        if (((idx + 1U) % 16U) == 0U)
        {
            message += "\n";
            ShowDebug(message.c_str());
            message.clear();
        }
    }

    if (!message.empty())
    {
        message += "\n";
        ShowDebug(message.c_str());
    }
}

/************************************************************************
 *                                                                       *
 *  Unknown Packet                                                       *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x000(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    ShowWarning("parse: Unhandled game packet %03hX from user: %s", (data.ref<uint16>(0) & 0x1FF), PChar->getName());
}

/************************************************************************
 *                                                                       *
 *  Non-Implemented Packet                                               *
 *                                                                       *
 ************************************************************************/

void SmallPacket0xFFF_NOT_IMPLEMENTED(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    ShowWarning("parse: SmallPacket is not implemented Type<%03hX>", (data.ref<uint16>(0) & 0x1FF));
}

/************************************************************************
 *                                                                       *
 *  Log Into Zone                                                        *
 *                                                                       *
 *  Update session key and client port between zone transitions.         *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x00A(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;
    data.ref<uint32>(0x5C) = 0;

    if (PSession->blowfish.status == BLOWFISH_ACCEPTED && PChar->status == STATUS_TYPE::NORMAL) // Do nothing if character is zoned in
    {
        ShowWarning("packet_system::SmallPacket0x00A player '%s' attempting to send 0x00A when already logged in", PChar->getName());
        return;
    }

    //
    // Handle out of sync zone correction..
    //
    if (data.ref<uint16_t>(0x02) > 1)
    {
        PSession->server_packet_id = data.ref<uint16_t>(0x02);

        // Clear all pending packets for this character.
        // This incoming 0x00A from the client wants us to set the starting sync count for all new packets to the sync count from 0x02.
        // If we do not do this, all further packets may be ignored by the client and will result in disconnection from the server.
        if (PChar)
        {
            PChar->clearPacketList();
        }
    }

    // No real distinction between these two states in the 0x00A handler --
    // Key is already assumed to be incremented correctly,
    // Pending zone is same process transfer, and waiting is new login or different process.
    if (PSession->blowfish.status == BLOWFISH_PENDING_ZONE || PSession->blowfish.status == BLOWFISH_WAITING) // Call zone in, etc, only once.
    {
        PSession->blowfish.status = BLOWFISH_ACCEPTED;
        PChar->clearPacketList();

        if (PChar->loc.zone != nullptr)
        {
            ShowError(fmt::format("{} sent 0x00A while their original zone wasn't wiped!", PChar->getName()));
            return;
        }

        PSession->shuttingDown = 0;

        uint16 destination = PChar->loc.destination;
        CZone* destZone    = zoneutils::GetZone(destination);

        if (destination >= MAX_ZONEID || destZone == nullptr)
        {
            // TODO: work out how to drop player in moghouse that exits them to the zone they were in before this happened, like we used to.
            ShowWarning("packet_system::SmallPacket0x00A player tried to enter zone that was invalid or out of range");
            ShowWarning("packet_system::SmallPacket0x00A dumping player `%s` to homepoint!", PChar->getName());
            charutils::HomePoint(PChar, true);
            return;
        }

        destZone->IncreaseZoneCounter(PChar);

        // Current zone could either be current zone or destination
        CZone* currentZone = zoneutils::GetZone(PChar->getZone());
        if (currentZone == nullptr)
        {
            ShowWarning("currentZone was null for Zone ID %d.", PChar->getZone());
            return;
        }

        charutils::updateSession(PSession, PChar, currentZone);
        charutils::loadDeathTimestamp(PChar);
        charutils::loadZoningFlag(PChar);
        charutils::SaveCharPosition(PChar);
        charutils::SaveZonesVisited(PChar);
        charutils::SavePlayTime(PChar);

        if (PChar->m_moghouseID != 0)
        {
            PChar->m_charHistory.mhEntrances++;
            charutils::updateMannequins(PChar);
            gardenutils::UpdateGardening(PChar, false);
        }
    }

    // Only release client from "Downloading Data" if the packet sequence came in without a drop on 0x00D
    // It is also possible that the client also never received our packets to release themselves from the loading screen.
    // TODO: Need further research into the relationship between 0x00D and 0x00A, if any.
    if (PChar->loc.zone != nullptr)
    {
        PChar->pushPacket<CDownloadingDataPacket>();
        PChar->pushPacket<CZoneInPacket>(PChar, PChar->currentEvent);
        PChar->pushPacket<CZoneVisitedPacket>(PChar);
    }
}

/************************************************************************
 *                                                                       *
 *  Player Action                                                        *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x01A(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    uint16 TargID = data.ref<uint16>(0x08);
    uint8  action = data.ref<uint8>(0x0A);

    // clang-format off
    position_t actionOffset =
    {
        data.ref<float>(0x10),
        data.ref<float>(0x14),
        data.ref<float>(0x18),
        0, // moving (packet only contains x/y/z)
        0, // rotation (packet only contains x/y/z)
    };
    // clang-format on

    constexpr auto actionToStr = [](uint8 actionIn)
    {
        switch (actionIn)
        {
            case 0x00:
                return "Trigger";
            case 0x02:
                return "Attack";
            case 0x03:
                return "Spellcast";
            case 0x04:
                return "Disengage";
            case 0x05:
                return "Call for Help";
            case 0x07:
                return "Weaponskill";
            case 0x09:
                return "Job Ability";
            case 0x0B:
                return "Homepoint";
            case 0x0C:
                return "Assist";
            case 0x0D:
                return "Raise";
            case 0x0E:
                return "Fishing";
            case 0x0F:
                return "Change Target";
            case 0x10:
                return "Ranged Attack";
            case 0x11:
                return "Chocobo Digging";
            case 0x12:
                return "Dismount";
            case 0x13:
                return "Tractor Menu";
            case 0x14:
                return "Complete Character Update";
            case 0x15:
                return "Ballista - Quarry";
            case 0x16:
                return "Ballista - Sprint";
            case 0x17:
                return "Ballista - Scout";
            case 0x18:
                return "Blockaid";
            case 0x19:
                return "Monstrosity Monster Skill";
            case 0x1A:
                return "Mounts";
            default:
                return "Unknown";
        }
    };

    // Monstrosity: Can't really do anything while under Gestation until you click it off.
    //            : MONs can trigger doors, so we'll handle that later.
    if (PChar->StatusEffectContainer->HasStatusEffect(EFFECT_GESTATION) && action == 0x00)
    {
        return;
    }

    const auto actionStr = fmt::format("Player Action: {}: {} ({}) -> targid: {}", PChar->getName(), actionToStr(action), hex8ToString(action), TargID);
    TracyZoneString(actionStr);
    ShowTrace(actionStr);
    DebugActions(actionStr);

    // Retrigger latents if the previous packet parse in this chunk included equip/equipset
    if (PChar->retriggerLatents)
    {
        for (uint8 equipSlotID = 0; equipSlotID < 16; ++equipSlotID)
        {
            if (PChar->equip[equipSlotID] != 0)
            {
                PChar->PLatentEffectContainer->CheckLatentsEquip(equipSlotID);
            }
        }
        PChar->retriggerLatents = false; // reset as we have retriggered the latents somewhere
    }

    switch (action)
    {
        case 0x00: // trigger
        {
            if (PChar->StatusEffectContainer->HasPreventActionEffect())
            {
                return;
            }

            if (PChar->m_Costume != 0 || PChar->animation == ANIMATION_SYNTH || (PChar->CraftContainer && PChar->CraftContainer->getItemsCount() > 0))
            {
                PChar->pushPacket<CReleasePacket>(PChar, RELEASE_TYPE::STANDARD);
                return;
            }

            CBaseEntity* PNpc = nullptr;
            PNpc              = PChar->GetEntity(TargID, TYPE_NPC | TYPE_MOB);

            // MONs are allowed to use doors, but nothing else
            if (PChar->m_PMonstrosity != nullptr &&
                PNpc->look.size != 0x02 &&
                PChar->getZone() != ZONEID::ZONE_FERETORY &&
                !settings::get<bool>("main.MONSTROSITY_TRIGGER_NPCS"))
            {
                PChar->pushPacket<CReleasePacket>(PChar, RELEASE_TYPE::STANDARD);
                return;
            }

            // NOTE: Moogles inside of mog houses are the exception for not requiring Spawned or Status checks.
            if (PNpc != nullptr && distance(PNpc->loc.p, PChar->loc.p) <= 6.0f && ((PNpc->PAI->IsSpawned() && PNpc->status == STATUS_TYPE::NORMAL) || PChar->m_moghouseID != 0))
            {
                PNpc->PAI->Trigger(PChar);
                PChar->m_charHistory.npcInteractions++;
            }

            // Releasing a trust
            // TODO: 0x0c is set to 0x1, not sure if that is relevant or not.
            if (auto* PTrust = dynamic_cast<CTrustEntity*>(PChar->GetEntity(TargID, TYPE_TRUST)))
            {
                PChar->RemoveTrust(PTrust);
            }

            if (!PChar->isNpcLocked())
            {
                PChar->eventPreparation->reset();
                PChar->pushPacket<CReleasePacket>(PChar, RELEASE_TYPE::STANDARD);
            }
        }
        break;
        case 0x02: // attack
        {
            if (PChar->isMounted())
            {
                PChar->StatusEffectContainer->DelStatusEffectSilent(EFFECT_MOUNTED);
            }

            PChar->PAI->Engage(TargID);
        }
        break;
        case 0x03: // spellcast
        {
            auto spellID = static_cast<SpellID>(data.ref<uint16>(0x0C));
            PChar->PAI->Cast(TargID, spellID);

            // target offset used only for luopan placement as of now
            if (spellID >= SpellID::Geo_Regen && spellID <= SpellID::Geo_Gravity)
            {
                // reset the action offset position to prevent other spells from using previous position data
                PChar->m_ActionOffsetPos = {};

                // Need to set the target position plus offset for positioning correctly
                auto* PTarget = dynamic_cast<CBattleEntity*>(PChar->GetEntity(TargID));

                if (PTarget != nullptr)
                {
                    PChar->m_ActionOffsetPos = {
                        PTarget->loc.p.x + actionOffset.x,
                        PTarget->loc.p.y + actionOffset.y,
                        PTarget->loc.p.z + actionOffset.z,
                        0, // packet only contains x/y/z
                        0, //
                    };
                }
            }
        }
        break;
        case 0x04: // disengage
        {
            if (!PChar->StatusEffectContainer->HasStatusEffect({ EFFECT_CHARM, EFFECT_CHARM_II }))
            {
                PChar->PAI->Disengage();
            }
        }
        break;
        case 0x05: // call for help
        {
            if (PChar->StatusEffectContainer->HasPreventActionEffect())
            {
                return;
            }

            if (auto* PMob = dynamic_cast<CMobEntity*>(PChar->GetBattleTarget()))
            {
                if (!PMob->GetCallForHelpFlag() && PMob->PEnmityContainer->HasID(PChar->id) && !PMob->m_CallForHelpBlocked)
                {
                    PMob->SetCallForHelpFlag(true);
                    PChar->loc.zone->PushPacket(PChar, CHAR_INRANGE_SELF, std::make_unique<CMessageBasicPacket>(PChar, PChar, 0, 0, 19));
                    return;
                }
            }

            PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, 22);
        }
        break;
        case 0x07: // weaponskill
        {
            if (!PChar->PAI->IsEngaged() && settings::get<bool>("map.PREVENT_UNENGAGED_WS")) // Prevent Weaponskill usage if player isn't engaged.
            {
                PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_UNABLE_TO_USE_WS);
                return;
            }

            uint16 WSkillID = data.ref<uint16>(0x0C);
            PChar->PAI->WeaponSkill(TargID, WSkillID);
        }
        break;
        case 0x09: // jobability
        {
            uint16 JobAbilityID     = data.ref<uint16>(0x0C);
            uint8  currentAnimation = PChar->animation;

            if (currentAnimation != ANIMATION_NONE && currentAnimation != ANIMATION_ATTACK)
            {
                ShowWarning("SmallPacket0x01A: Player %s trying to use a Job Ability from invalid state", PChar->getName());
                return;
            }

            // Don't allow BST to use ready before level 25
            if (PChar->PPet != nullptr && (!charutils::hasAbility(PChar, ABILITY_READY) || !PChar->PPet->PAI->IsEngaged()))
            {
                if (JobAbilityID >= ABILITY_FOOT_KICK && JobAbilityID <= ABILITY_PENTAPECK) // Is this a BST ability?
                {
                    PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_UNABLE_TO_USE_JA2);
                    return;
                }
            }

            PChar->PAI->Ability(TargID, JobAbilityID);
        }
        break;
        case 0x0B: // homepoint
        {
            if (!PChar->isDead())
            {
                return;
            }

            if (PChar->m_PMonstrosity != nullptr)
            {
                auto type = data.ref<uint8>(0x0C);
                monstrosity::HandleDeathMenu(PChar, type);
                return;
            }

            PChar->setCharVar("expLost", 0);
            charutils::HomePoint(PChar, true);
        }
        break;
        case 0x0C: // assist
        {
            battleutils::assistTarget(PChar, TargID);
        }
        break;
        case 0x0D: // raise menu
        {
            if (!PChar->m_hasRaise)
            {
                return;
            }

            if (data.ref<uint8>(0x0C) == 0)
            { // ACCEPTED RAISE
                PChar->Raise();
            }
            else
            {
                PChar->m_hasRaise = 0;
            }
        }
        break;
        case 0x0E: // Fishing
        {
            if (PChar->StatusEffectContainer->HasPreventActionEffect())
            {
                return;
            }

            fishingutils::StartFishing(PChar);
        }
        break;
        case 0x0F: // change target
        {
            PChar->PAI->ChangeTarget(TargID);
        }
        break;
        case 0x10: // Ranged Attack
        {
            uint8 currentAnimation = PChar->animation;
            if (currentAnimation != ANIMATION_NONE && currentAnimation != ANIMATION_ATTACK)
            {
                ShowWarning("SmallPacket0x01A: Player %s trying to Ranged Attack from invalid state", PChar->getName());
                return;
            }

            PChar->PAI->RangedAttack(TargID);
        }
        break;
        case 0x11: // chocobo digging
        {
            // Mounted Check.
            if (!PChar->isMounted())
            {
                return;
            }

            // Gysahl Green Check.
            uint8 slotID = PChar->getStorage(LOC_INVENTORY)->SearchItem(4545);
            if (slotID == ERROR_SLOTID)
            {
                PChar->pushPacket<CMessageSystemPacket>(4545, 0, MsgStd::YouDontHaveAny);
                return;
            }

            // Consume Gysahl Green and push animation on dig attempt.
            if (luautils::OnChocoboDig(PChar))
            {
                charutils::UpdateItem(PChar, LOC_INVENTORY, slotID, -1);
                PChar->pushPacket<CInventoryFinishPacket>();
                PChar->loc.zone->PushPacket(PChar, CHAR_INRANGE_SELF, std::make_unique<CChocoboDiggingPacket>(PChar));
            }
        }
        break;
        case 0x12: // dismount
        {
            if (PChar->StatusEffectContainer->HasPreventActionEffect() || !PChar->isMounted())
            {
                return;
            }

            PChar->animation = ANIMATION_NONE;
            PChar->updatemask |= UPDATE_HP;
            PChar->StatusEffectContainer->DelStatusEffectSilent(EFFECT_MOUNTED);

            // Workaround for a bug where dismounting out of update range would cause the character to stop rendering.
            PChar->loc.zone->UpdateEntityPacket(PChar, ENTITY_UPDATE, UPDATE_HP);
        }
        break;
        case 0x13: // tractor menu
        {
            if (data.ref<uint8>(0x0C) == 0 && PChar->m_hasTractor != 0) // ACCEPTED TRACTOR
            {
                PChar->loc.p           = PChar->m_StartActionPos;
                PChar->loc.destination = PChar->getZone();
                PChar->status          = STATUS_TYPE::DISAPPEAR;
                PChar->loc.boundary    = 0;
                PChar->clearPacketList();
                charutils::SendToZone(PChar, PChar->loc.destination);
            }

            PChar->m_hasTractor = 0;
        }
        break;
        case 0x14: // complete character update
        {
            if (PChar->m_moghouseID != 0) // TODO: For now this is only in the moghouse
            {
                PChar->loc.zone->SpawnConditionalNPCs(PChar);
            }
            else
            {
                PChar->requestedInfoSync = true;
                PChar->loc.zone->SpawnNPCs(PChar);
                PChar->loc.zone->SpawnMOBs(PChar);
                PChar->loc.zone->SpawnTRUSTs(PChar);
            }
        }
        break;
        case 0x15: // ballista - quarry
        case 0x16: // ballista - sprint
        case 0x17: // ballista - scout
            break;
        case 0x18: // blockaid
        {
            if (!PChar->StatusEffectContainer->HasStatusEffect(EFFECT_ALLIED_TAGS))
            {
                uint8 type = data.ref<uint8>(0x0C);

                if (type == 0x00 && PChar->getBlockingAid()) // /blockaid off
                {
                    // Blockaid canceled
                    PChar->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::BlockaidCanceled);
                    PChar->setBlockingAid(false);
                }
                else if (type == 0x01 && !PChar->getBlockingAid()) // /blockaid on
                {
                    // Blockaid activated
                    PChar->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::BlockaidActivated);
                    PChar->setBlockingAid(true);
                }
                else if (type == 0x02) // /blockaid
                {
                    // Blockaid is currently active/inactive
                    PChar->pushPacket<CMessageSystemPacket>(0, 0, PChar->getBlockingAid() ? MsgStd::BlockaidCurrentlyActive : MsgStd::BlockaidCurrentlyInactive);
                }
            }
            else
            {
                PChar->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::CannotUseCommandAtTheMoment);
            }
        }
        break;
        case 0x19: // Monstrosity Monster Skill
        {
            monstrosity::HandleMonsterSkillActionPacket(PChar, data);
        }
        break;
        case 0x1A: // mounts
        {
            uint8 MountID = data.ref<uint8>(0x0C);

            if (PChar->animation != ANIMATION_NONE)
            {
                PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, 71);
            }
            else if (!PChar->loc.zone->CanUseMisc(MISC_MOUNT))
            {
                PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_CANNOT_USE_IN_AREA);
            }
            else if (PChar->GetMLevel() < 20)
            {
                PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 20, 0, 773);
            }
            else if (charutils::hasKeyItem(PChar, 3072 + MountID))
            {
                if (PChar->PRecastContainer->HasRecast(RECAST_ABILITY, 256, 60s))
                {
                    PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, 94);

                    // add recast timer
                    // PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, 202);
                    return;
                }

                if (PChar->hasEnmityEXPENSIVE())
                {
                    PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, 0, 0, MSGBASIC_YOUR_MOUNT_REFUSES);
                    return;
                }

                PChar->StatusEffectContainer->AddStatusEffect(new CStatusEffect(
                                                                  EFFECT_MOUNTED,
                                                                  EFFECT_MOUNTED,
                                                                  MountID ? ++MountID : 0,
                                                                  0s,
                                                                  30min,
                                                                  0,
                                                                  0x40), // previously known as nameflag "FLAG_CHOCOBO"
                                                              EffectNotice::Silent);

                PChar->PRecastContainer->Add(RECAST_ABILITY, 256, 60s);
                PChar->pushPacket<CCharRecastPacket>(PChar);

                luautils::OnPlayerMount(PChar);
            }
        }
        break;
        default:
        {
            ShowWarningFmt("CLIENT {} PERFORMING UNHANDLED ACTION {} ({})", PChar->getName(), actionStr, hex8ToString(action));
            return;
        }
        break;
    }
}

/************************************************************************
 *                                                                       *
 *  /volunteer packet                                                    *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x01E(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    // It sends out a packet of type 0x1E, where the body is a 4 - byte aligned string
    //
    // "/volunteer Volunteer what" without anything targeted results in:
    // 1E0A6405566F6C756E7465657220776861740000 -> Volunteer what\0\0
    //
    // "/volunteer I choose you" with a Savanna Rarab targeted results in:
    // 1E127505492063686F6F736520796F7520543120536176616E6E61205261726162000000 -> I choose you T1 Savanna Rarab\0\0\0
    //
    // "/volunteer hello" with no target -> 1e 06 17 00 68 65 6c 6c 6f 00 00 00
    // "/volunteer test" with no target -> 1e 06 92 00 74 65 73 74 00 00 00 00
    //
    // id - length - seq - 00 - content -- null terminators/padding

    const uint8 HEADER_LENGTH = 4;

    // clang-format off
    std::vector<char> chars;
    std::for_each(data[HEADER_LENGTH], data[HEADER_LENGTH] + (data.getSize() - HEADER_LENGTH), [&](char ch)
    {
        if (isascii(ch) && ch != '\0')
        {
            chars.emplace_back(ch);
        }
    });
    // clang-format on
    auto str = std::string(chars.begin(), chars.end());
    luautils::OnPlayerVolunteer(PChar, str);
}

/************************************************************************
 *                                                                       *
 *  Party Invite                                                         *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x06E(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    // Alias for clarity
    auto* PInviter = PChar;

    uint32 inviteeCharId = data.ref<uint32>(0x04);
    uint16 inviteeTargId = data.ref<uint16>(0x08);

    // cannot invite yourself
    if (PInviter->id == inviteeCharId)
    {
        return;
    }

    if (jailutils::InPrison(PInviter))
    {
        // Initiator is in prison.  Send error message.
        PInviter->pushPacket<CMessageBasicPacket>(PInviter, PInviter, 0, 0, MSGBASIC_CANNOT_USE_IN_AREA);
        return;
    }

    // Block invite if target has blacklisted the initiator
    if (blacklistutils::IsBlacklisted(inviteeCharId, PInviter->id))
    {
        return;
    }

    switch (data.ref<uint8>(0x0A))
    {
        case INVITE_PARTY: // party - must by party leader or solo
        {
            if (PInviter->PParty == nullptr || PInviter->PParty->GetLeader() == PInviter)
            {
                if (PInviter->PParty && PInviter->PParty->IsFull())
                {
                    PInviter->pushPacket<CMessageStandardPacket>(PInviter, 0, 0, MsgStd::CannotInvite);
                    break;
                }

                CCharEntity* PInvitee = nullptr;

                if (inviteeTargId != 0)
                {
                    CBaseEntity* PEntity = PInviter->GetEntity(inviteeTargId, TYPE_PC);
                    if (PEntity && PEntity->id == inviteeCharId)
                    {
                        PInvitee = (CCharEntity*)PEntity;
                    }
                }
                else
                {
                    PInvitee = zoneutils::GetChar(inviteeCharId);
                }

                if (PInvitee)
                {
                    ShowDebug("%s sent party invite to %s", PInviter->getName(), PInvitee->getName());

                    // make sure invitee isn't dead or in jail, they aren't a party member and don't already have an invite pending, and your party is not full
                    if (PInvitee->isDead() || jailutils::InPrison(PInvitee) || PInvitee->InvitePending.id != 0 || PInvitee->PParty != nullptr)
                    {
                        ShowDebug("%s is dead, in jail, has a pending invite, or is already in a party", PInvitee->getName());
                        PInviter->pushPacket<CMessageStandardPacket>(PInviter, 0, 0, MsgStd::CannotInvite);
                        break;
                    }

                    // check /blockaid
                    if (PInvitee->getBlockingAid())
                    {
                        ShowDebug("%s is blocking party invites", PInvitee->getName());
                        // Target is blocking assistance
                        PInviter->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::TargetIsCurrentlyBlocking);
                        // Interaction was blocked
                        PInvitee->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::BlockedByBlockaid);
                        // You cannot invite that person at this time.
                        PInviter->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::CannotInvite);
                        break;
                    }

                    if (PInvitee->StatusEffectContainer->HasStatusEffect(EFFECT_LEVEL_SYNC))
                    {
                        ShowDebug("%s has level sync, unable to send invite", PInvitee->getName());
                        PInviter->pushPacket<CMessageStandardPacket>(PInviter, 0, 0, MsgStd::CannotInviteLevelSync);
                        break;
                    }

                    PInvitee->InvitePending.id     = PInviter->id;
                    PInvitee->InvitePending.targid = PInviter->targid;

                    PInvitee->pushPacket<CPartyInvitePacket>(inviteeCharId, inviteeTargId, PInviter->getName(), INVITE_PARTY);

                    ShowDebug("Sent party invite packet to %s", PInvitee->getName());

                    if (PInviter->PParty && PInviter->PParty->GetSyncTarget())
                    {
                        PInvitee->pushPacket<CMessageStandardPacket>(PInvitee, 0, 0, MsgStd::LevelSyncWarning);
                    }
                }
                else
                {
                    // on another server (hopefully)
                    message::send(ipc::PartyInvite{
                        .inviteeId     = inviteeCharId,
                        .inviteeTargId = inviteeTargId,
                        .inviterId     = PInviter->id,
                        .inviterTargId = PInviter->targid,
                        .inviterName   = PInviter->getName(),
                        .inviteType    = INVITE_PARTY,
                    });
                }
            }
            else // in party but not leader, cannot invite
            {
                ShowDebug("%s is not party leader, cannot send invite", PInviter->getName());
                PInviter->pushPacket<CMessageStandardPacket>(PInviter, 0, 0, MsgStd::NotPartyLeader);
            }
        }
        break;
        case INVITE_ALLIANCE: // alliance - must be unallied party leader or alliance leader of a non-full alliance
        {
            if (PInviter->PParty && PInviter->PParty->GetLeader() == PInviter &&
                (PInviter->PParty->m_PAlliance == nullptr ||
                 (PInviter->PParty->m_PAlliance->getMainParty() == PInviter->PParty && !PInviter->PParty->m_PAlliance->isFull())))
            {
                CCharEntity* PInvitee = nullptr;

                if (inviteeTargId != 0)
                {
                    CBaseEntity* PEntity = PInviter->GetEntity(inviteeTargId, TYPE_PC);
                    if (PEntity && PEntity->id == inviteeCharId)
                    {
                        PInvitee = (CCharEntity*)PEntity;
                    }
                }
                else
                {
                    PInvitee = zoneutils::GetChar(inviteeCharId);
                }

                if (PInvitee)
                {
                    ShowDebug("%s sent alliance invite to %s", PInviter->getName(), PInvitee->getName());

                    // check /blockaid
                    if (PInvitee->getBlockingAid())
                    {
                        ShowDebug("%s is blocking alliance invites", PInvitee->getName());
                        // Target is blocking assistance
                        PInviter->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::TargetIsCurrentlyBlocking);
                        // Interaction was blocked
                        PInvitee->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::BlockedByBlockaid);
                        // You cannot invite that person at this time.
                        PInviter->pushPacket<CMessageSystemPacket>(0, 0, MsgStd::CannotInvite);
                        break;
                    }

                    // make sure intvitee isn't dead or in jail, they are an unallied party leader and don't already have an invite pending
                    if (PInvitee->isDead() || jailutils::InPrison(PInvitee) || PInvitee->InvitePending.id != 0 || PInvitee->PParty == nullptr ||
                        PInvitee->PParty->GetLeader() != PInvitee || PInvitee->PParty->m_PAlliance)
                    {
                        ShowDebug("%s is dead, in jail, has a pending invite, or is already in a party/alliance", PInvitee->getName());
                        PInviter->pushPacket<CMessageStandardPacket>(PInviter, 0, 0, MsgStd::CannotInvite);
                        break;
                    }

                    if (PInvitee->StatusEffectContainer->HasStatusEffect(EFFECT_LEVEL_SYNC))
                    {
                        ShowDebug("%s has level sync, unable to send invite", PInvitee->getName());
                        PInviter->pushPacket<CMessageStandardPacket>(PInviter, 0, 0, MsgStd::CannotInviteLevelSync);
                        break;
                    }

                    PInvitee->InvitePending.id     = PInviter->id;
                    PInvitee->InvitePending.targid = PInviter->targid;

                    PInvitee->pushPacket<CPartyInvitePacket>(inviteeCharId, inviteeTargId, PInviter->getName(), INVITE_ALLIANCE);

                    ShowDebug("Sent party invite packet to %s", PInvitee->getName());
                }
                else
                {
                    // on another server (hopefully)
                    message::send(ipc::PartyInvite{
                        .inviteeId     = inviteeCharId,
                        .inviteeTargId = inviteeTargId,
                        .inviterId     = PInviter->id,
                        .inviterTargId = PInviter->targid,
                        .inviterName   = PInviter->getName(),
                        .inviteType    = INVITE_ALLIANCE,
                    });
                }
            }
        }
        break;
        default:
        {
            ShowError("SmallPacket0x06E : unknown byte <%.2X>", data.ref<uint8>(0x0A));
        }
        break;
    }
}

/************************************************************************
 *                                                                       *
 *  Party / Alliance Command 'Leave'                                     *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x06F(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    if (PChar->PParty)
    {
        switch (data.ref<uint8>(0x04))
        {
            case INVITE_PARTY: // party - anyone may remove themself from party regardless of leadership or alliance
            {
                if (PChar->PParty->m_PAlliance &&
                    PChar->PParty->HasOnlyOneMember()) // single member alliance parties must be removed from alliance before disband
                {
                    ShowDebug("%s party size is one", PChar->getName());

                    if (PChar->PParty->m_PAlliance->hasOnlyOneParty()) // if there is only 1 party then dissolve alliance
                    {
                        ShowDebug("%s alliance size is one party", PChar->getName());

                        PChar->PParty->m_PAlliance->dissolveAlliance();
                        ShowDebug("%s alliance is dissolved", PChar->getName());
                    }
                    else
                    {
                        ShowDebug("Removing %s party from alliance", PChar->getName());

                        PChar->PParty->m_PAlliance->removeParty(PChar->PParty);
                        ShowDebug("%s party is removed from alliance", PChar->getName());
                    }
                }
                ShowDebug("Removing %s from party", PChar->getName());

                PChar->PParty->RemoveMember(PChar);
                ShowDebug("%s is removed from party", PChar->getName());
            }
            break;
            case INVITE_ALLIANCE: // alliance - any party leader in alliance may remove their party
            {
                if (PChar->PParty->m_PAlliance && PChar->PParty->GetLeader() == PChar)
                {
                    ShowDebug("%s is leader of a party in an alliance", PChar->getName());
                    if (PChar->PParty->m_PAlliance->hasOnlyOneParty()) // if there is only 1 party then dissolve alliance
                    {
                        ShowDebug("One party in alliance, %s wants to dissolve the alliance", PChar->getName());

                        PChar->PParty->m_PAlliance->dissolveAlliance();
                        ShowDebug("%s has dissolved the alliance", PChar->getName());
                    }
                    else
                    {
                        ShowDebug("%s wants to remove their party from the alliance", PChar->getName());

                        PChar->PParty->m_PAlliance->removeParty(PChar->PParty);
                        ShowDebug("%s party is removed from the alliance", PChar->getName());
                    }
                }
            }
            break;
            default:
            {
                ShowError("SmallPacket0x06F : unknown byte <%.2X>", data.ref<uint8>(0x04));
            }
            break;
        }
    }
}

/************************************************************************
 *                                                                       *
 *  Create Linkpearl                                                     *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x0C3(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    uint8           lsNum          = data.ref<uint8>(0x05);
    CItemLinkshell* PItemLinkshell = (CItemLinkshell*)PChar->getEquip(SLOT_LINK1);
    if (lsNum == 2)
    {
        PItemLinkshell = (CItemLinkshell*)PChar->getEquip(SLOT_LINK2);
    }

    if (PItemLinkshell != nullptr && PItemLinkshell->isType(ITEM_LINKSHELL) &&
        (PItemLinkshell->GetLSType() == LSTYPE_PEARLSACK || PItemLinkshell->GetLSType() == LSTYPE_LINKSHELL))
    {
        CItemLinkshell* PItemLinkPearl = (CItemLinkshell*)itemutils::GetItem(ITEMID::LINKPEARL);
        if (PItemLinkPearl)
        {
            PItemLinkPearl->setQuantity(1);
            std::memcpy(PItemLinkPearl->m_extra, PItemLinkshell->m_extra, 24);
            PItemLinkPearl->SetLSType(LSTYPE_LINKPEARL);
            charutils::AddItem(PChar, LOC_INVENTORY, PItemLinkPearl);
        }
    }
}

/************************************************************************
 *                                                                       *
 *  Create Linkshell (Also equips the linkshell.)                        *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x0C4(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    uint8 SlotID     = data.ref<uint8>(0x06);
    uint8 LocationID = data.ref<uint8>(0x07);
    uint8 action     = data.ref<uint8>(0x08);
    uint8 lsNum      = data.ref<uint8>(0x1B);

    CItemLinkshell* PItemLinkshell = (CItemLinkshell*)PChar->getStorage(LocationID)->GetItem(SlotID);

    if (PItemLinkshell != nullptr && PItemLinkshell->isType(ITEM_LINKSHELL))
    {
        // Create new linkshell
        if (PItemLinkshell->getID() == ITEMID::NEW_LINKSHELL)
        {
            uint32 LinkshellID    = 0;
            uint16 LinkshellColor = data.ref<uint16>(0x04);

            char DecodedName[DecodeStringLength];
            char EncodedName[LinkshellStringLength];

            std::memset(&DecodedName, 0, sizeof(DecodedName));
            std::memset(&EncodedName, 0, sizeof(EncodedName));

            const auto incomingName = db::escapeString(asStringFromUntrustedSource(data[0x0C], 20));

            DecodeStringLinkshell(incomingName.data(), DecodedName);
            EncodeStringLinkshell(DecodedName, EncodedName);

            // TODO: Check if a linebreak is needed

            LinkshellID = linkshell::RegisterNewLinkshell(DecodedName, LinkshellColor);
            if (LinkshellID != 0)
            {
                destroy(PItemLinkshell);
                PItemLinkshell = (CItemLinkshell*)itemutils::GetItem(ITEMID::LINKSHELL);
                if (PItemLinkshell == nullptr)
                {
                    return;
                }

                PItemLinkshell->setQuantity(1);
                PChar->getStorage(LocationID)->InsertItem(PItemLinkshell, SlotID);
                PItemLinkshell->SetLSID(LinkshellID);
                PItemLinkshell->SetLSType(LSTYPE_LINKSHELL);
                PItemLinkshell->setSignature(EncodedName); // because apparently the format from the packet isn't right, and is missing terminators
                PItemLinkshell->SetLSColor(LinkshellColor);

                const auto rset = db::preparedStmt("UPDATE char_inventory SET signature = ?, extra = ?, itemId = 513 WHERE charid = ? AND location = 0 AND slot = ? LIMIT 1",
                                                   DecodedName, PItemLinkshell->m_extra, PChar->id, SlotID);
                if (rset && rset->rowsAffected())
                {
                    PChar->pushPacket<CInventoryItemPacket>(PItemLinkshell, LocationID, SlotID);
                }
            }
            else
            {
                PChar->pushPacket<CMessageStandardPacket>(MsgStd::LinkshellUnavailable);
                // DE
                // 20
                // 1D
                return;
            }
        }
        else
        {
            SLOTTYPE    slot         = SLOT_LINK1;
            CLinkshell* OldLinkshell = PChar->PLinkshell1;
            if (lsNum == 2)
            {
                slot         = SLOT_LINK2;
                OldLinkshell = PChar->PLinkshell2;
            }
            switch (action)
            {
                case 0: // unequip linkshell
                {
                    linkshell::DelOnlineMember(PChar, PItemLinkshell);

                    PItemLinkshell->setSubType(ITEM_UNLOCKED);

                    PChar->equip[slot]    = 0;
                    PChar->equipLoc[slot] = 0;
                    if (lsNum == 1)
                    {
                        PChar->updatemask |= UPDATE_HP;
                    }

                    PChar->pushPacket<CInventoryAssignPacket>(PItemLinkshell, INV_NORMAL);
                }
                break;
                case 1: // equip linkshell
                {
                    const auto rset = db::preparedStmt("SELECT broken FROM linkshells WHERE linkshellid = ? LIMIT 1", PItemLinkshell->GetLSID());
                    if (rset && rset->rowsCount() && rset->next() && rset->get<uint8>("broken") == 1)
                    {
                        // if the linkshell has been broken, break the item
                        PItemLinkshell->SetLSType(LSTYPE_BROKEN);

                        db::preparedStmt("UPDATE char_inventory SET extra = ? WHERE charid = ? AND location = ? AND slot = ? LIMIT 1",
                                         PItemLinkshell->m_extra, PChar->id, PItemLinkshell->getLocationID(), PItemLinkshell->getSlotID());

                        PChar->pushPacket<CInventoryItemPacket>(PItemLinkshell, PItemLinkshell->getLocationID(), PItemLinkshell->getSlotID());
                        PChar->pushPacket<CInventoryFinishPacket>();
                        PChar->pushPacket<CMessageStandardPacket>(MsgStd::LinkshellNoLongerExists);
                        return;
                    }
                    if (PItemLinkshell->GetLSID() == 0)
                    {
                        PChar->pushPacket<CMessageStandardPacket>(MsgStd::LinkshellNoLongerExists);
                        return;
                    }
                    if (OldLinkshell != nullptr) // switching linkshell group
                    {
                        CItemLinkshell* POldItemLinkshell = (CItemLinkshell*)PChar->getEquip(slot);

                        if (POldItemLinkshell != nullptr && POldItemLinkshell->isType(ITEM_LINKSHELL))
                        {
                            linkshell::DelOnlineMember(PChar, POldItemLinkshell);

                            POldItemLinkshell->setSubType(ITEM_UNLOCKED);
                            PChar->pushPacket<CInventoryAssignPacket>(POldItemLinkshell, INV_NORMAL);
                        }
                    }
                    linkshell::AddOnlineMember(PChar, PItemLinkshell, lsNum);

                    PItemLinkshell->setSubType(ITEM_LOCKED);

                    PChar->equip[slot]    = SlotID;
                    PChar->equipLoc[slot] = LocationID;
                    if (lsNum == 1)
                    {
                        PChar->updatemask |= UPDATE_HP;
                    }

                    PChar->pushPacket<CInventoryAssignPacket>(PItemLinkshell, INV_LINKSHELL);
                }
                break;
            }
            charutils::SaveCharStats(PChar);
            charutils::SaveCharEquip(PChar);

            PChar->pushPacket<CLinkshellEquipPacket>(PChar, lsNum);
            PChar->pushPacket<CInventoryItemPacket>(PItemLinkshell, LocationID, SlotID);
        }
        PChar->pushPacket<CInventoryFinishPacket>();
        PChar->pushPacket<CCharStatusPacket>(PChar);
    }
}

/************************************************************************
 *                                                                       *
 *  Mog House actions                                                    *
 *                                                                       *
 ************************************************************************/

void SmallPacket0x0CB(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    auto operation = data.ref<uint8>(0x04);
    if (operation == 1)
    {
        // open mog house

        // NOTE: If you zone or move floors while in the MH and you have someone visiting, they will be booted.
        // NOTE: When you zone or move floors your "open MH" flag will be reset.
    }
    else if (operation == 2)
    {
        // close mog house
    }
    else if (operation == 5)
    {
        // remodel mog house
        auto type = data.ref<uint8>(0x06); // Sandy: 103, Bastok: 104, Windy: 105, Patio: 106

        if (type == 106 && !charutils::hasKeyItem(PChar, 3051))
        {
            ShowWarning(fmt::format("Player {} is trying to remodel to MH2F to Patio without owning the KI to unlock it.", PChar->getName()));
            return;
        }

        // 0x0080: This bit and the next track which 2F decoration style is being used (0: SANDORIA, 1: BASTOK, 2: WINDURST, 3: PATIO)
        // 0x0100: ^ As above

        // Extract original model and add 103 so it's in line with what comes in with the packet.
        uint16 oldType = (uint8)(((PChar->profile.mhflag & 0x0100) + (PChar->profile.mhflag & 0x0080)) >> 7) + 103;

        // Clear bits first
        PChar->profile.mhflag &= ~(0x0080);
        PChar->profile.mhflag &= ~(0x0100);

        // Write new model bits
        PChar->profile.mhflag |= ((type - 103) << 7);
        charutils::SaveCharStats(PChar);

        // TODO: Send message on successful remodel

        // If the model changes AND you're on MH2F; force a rezone so the model change can take effect.
        if (type != oldType && PChar->profile.mhflag & 0x0040)
        {
            auto zoneid = PChar->getZone();

            PChar->loc.destination = zoneid;
            PChar->status          = STATUS_TYPE::DISAPPEAR;

            PChar->clearPacketList();
            charutils::SendToZone(PChar, zoneid);
        }
    }
    else
    {
        ShowWarning("SmallPacket0x0CB : unknown byte <%.2X>", data.ref<uint8>(0x04));
    }
}

/************************************************************************
 *                                                                        *
 *  Roe Quest Log Request                                                 *
 *                                                                        *
 ************************************************************************/

void SmallPacket0x112(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;
    // Send spark updates
    PChar->pushPacket<CRoeSparkUpdatePacket>(PChar);

    if (settings::get<bool>("main.ENABLE_ROE"))
    {
        // Current RoE quests
        PChar->pushPacket<CRoeUpdatePacket>(PChar);

        // Players logging in to a new timed record get one-time message
        if (PChar->m_eminenceCache.notifyTimedRecord)
        {
            PChar->m_eminenceCache.notifyTimedRecord = false;
            PChar->pushPacket<CMessageBasicPacket>(PChar, PChar, roeutils::GetActiveTimedRecord(), 0, MSGBASIC_ROE_TIMED);
        }

        // 4-part Eminence Completion bitmap
        for (int i = 0; i < 4; i++)
        {
            PChar->pushPacket<CRoeQuestLogPacket>(PChar, i);
        }
    }
}

template <typename T>
void ValidatedPacketHandler(MapSession* const PSession, CCharEntity* const PChar, CBasicPacket& data)
{
    TracyZoneScoped;

    const T* packet = data.as<T>();

    if (const auto result = packet->validate(PSession, PChar); result.valid())
    {
        packet->process(PSession, PChar);
    }
    else
    {
        ShowWarningFmt("Invalid {} packet from {}: {} ", packet->getName(), PChar->name, result.errorString());
    }
}

/************************************************************************
 *                                                                       *
 *  Packet Array Initialization                                          *
 *                                                                       *
 ************************************************************************/

void PacketParserInitialize()
{
    TracyZoneScoped;
    for (uint16 i = 0; i < 512; ++i)
    {
        PacketSize[i]   = 0;
        PacketParser[i] = &SmallPacket0x000;
    }
    // clang-format off
    PacketSize[0x00A] = 0x2E; PacketParser[0x00A] = &SmallPacket0x00A;
    PacketSize[0x00C] = 0x0C; PacketParser[0x00C] = &ValidatedPacketHandler<GP_CLI_COMMAND_GAMEOK>;
    PacketSize[0x00D] = 0x08; PacketParser[0x00D] = &ValidatedPacketHandler<GP_CLI_COMMAND_NETEND>;
    PacketSize[0x00F] = 0x24; PacketParser[0x00F] = &ValidatedPacketHandler<GP_CLI_COMMAND_CLSTAT>;
    PacketSize[0x011] = 0x06; PacketParser[0x011] = &ValidatedPacketHandler<GP_CLI_COMMAND_ZONE_TRANSITION>;
    PacketSize[0x015] = 0x20; PacketParser[0x015] = &ValidatedPacketHandler<GP_CLI_COMMAND_POS>;
    PacketSize[0x016] = 0x08; PacketParser[0x016] = &ValidatedPacketHandler<GP_CLI_COMMAND_CHARREQ>;
    PacketSize[0x017] = 0x14; PacketParser[0x017] = &ValidatedPacketHandler<GP_CLI_COMMAND_CHARREQ2>;
    PacketSize[0x01A] = 0x0E; PacketParser[0x01A] = &SmallPacket0x01A;
    PacketSize[0x01B] = 0x1C; PacketParser[0x01B] = &ValidatedPacketHandler<GP_CLI_COMMAND_FRIENDPASS>;
    PacketSize[0x01C] = 0x0C; PacketParser[0x01C] = &ValidatedPacketHandler<GP_CLI_COMMAND_UNKNOWN>;
    PacketSize[0x01E] = 0x00; PacketParser[0x01E] = &SmallPacket0x01E;
    PacketSize[0x01F] = 0x00; PacketParser[0x01F] = &ValidatedPacketHandler<GP_CLI_COMMAND_GMCOMMAND>;
    PacketSize[0x028] = 0x0C; PacketParser[0x028] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_DUMP>;
    PacketSize[0x029] = 0x0C; PacketParser[0x029] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_MOVE>;
    PacketSize[0x02B] = 0x00; PacketParser[0x02B] = &ValidatedPacketHandler<GP_CLI_COMMAND_TRANSLATE>;
    PacketSize[0x02C] = 0x00; PacketParser[0x02C] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEMSEARCH>;
    PacketSize[0x032] = 0x0C; PacketParser[0x032] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_TRADE_REQ>;
    PacketSize[0x033] = 0x0C; PacketParser[0x033] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_TRADE_RES>;
    PacketSize[0x034] = 0x0C; PacketParser[0x034] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_TRADE_LIST>;
    PacketSize[0x036] = 0x40; PacketParser[0x036] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_TRANSFER>;
    PacketSize[0x037] = 0x14; PacketParser[0x037] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_USE>;
    PacketSize[0x03A] = 0x08; PacketParser[0x03A] = &ValidatedPacketHandler<GP_CLI_COMMAND_ITEM_STACK>;
    PacketSize[0x03B] = 0x20; PacketParser[0x03B] = &ValidatedPacketHandler<GP_CLI_COMMAND_MANNEQUIN_SET>;
    PacketSize[0x03C] = 0x1C; PacketParser[0x03C] = &ValidatedPacketHandler<GP_CLI_COMMAND_BLACK_LIST>;
    PacketSize[0x03D] = 0x1C; PacketParser[0x03D] = &ValidatedPacketHandler<GP_CLI_COMMAND_BLACK_EDIT>;
    PacketSize[0x041] = 0x00; PacketParser[0x041] = &ValidatedPacketHandler<GP_CLI_COMMAND_TROPHY_ENTRY>;
    PacketSize[0x042] = 0x06; PacketParser[0x042] = &ValidatedPacketHandler<GP_CLI_COMMAND_TROPHY_ABSENCE>;
    PacketSize[0x04B] = 0x18; PacketParser[0x04B] = &ValidatedPacketHandler<GP_CLI_COMMAND_FRAGMENTS>;
    PacketSize[0x04D] = 0x20; PacketParser[0x04D] = &ValidatedPacketHandler<GP_CLI_COMMAND_PBX>;
    PacketSize[0x04E] = 0x3C; PacketParser[0x04E] = &ValidatedPacketHandler<GP_CLI_COMMAND_AUC>;
    PacketSize[0x050] = 0x08; PacketParser[0x050] = &ValidatedPacketHandler<GP_CLI_COMMAND_EQUIP_SET>;
    PacketSize[0x051] = 0x48; PacketParser[0x051] = &ValidatedPacketHandler<GP_CLI_COMMAND_EQUIPSET_SET>;
    PacketSize[0x052] = 0x4C; PacketParser[0x052] = &ValidatedPacketHandler<GP_CLI_COMMAND_EQUIPSET_CHECK>;
    PacketSize[0x053] = 0x88; PacketParser[0x053] = &ValidatedPacketHandler<GP_CLI_COMMAND_LOCKSTYLE>;
    PacketSize[0x058] = 0x0A; PacketParser[0x058] = &ValidatedPacketHandler<GP_CLI_COMMAND_RECIPE>;
    PacketSize[0x059] = 0x10; PacketParser[0x059] = &ValidatedPacketHandler<GP_CLI_COMMAND_EFFECTEND>;
    PacketSize[0x05A] = 0x00; PacketParser[0x05A] = &ValidatedPacketHandler<GP_CLI_COMMAND_REQCONQUEST>;
    PacketSize[0x05B] = 0x14; PacketParser[0x05B] = &ValidatedPacketHandler<GP_CLI_COMMAND_EVENTEND>;
    PacketSize[0x05C] = 0x20; PacketParser[0x05C] = &ValidatedPacketHandler<GP_CLI_COMMAND_EVENTENDXZY>;
    PacketSize[0x05D] = 0x10; PacketParser[0x05D] = &ValidatedPacketHandler<GP_CLI_COMMAND_MOTION>;
    PacketSize[0x05E] = 0x18; PacketParser[0x05E] = &ValidatedPacketHandler<GP_CLI_COMMAND_MAPRECT>;
    PacketSize[0x060] = 0x1C; PacketParser[0x060] = &ValidatedPacketHandler<GP_CLI_COMMAND_PASSWARDS>;
    PacketSize[0x061] = 0x06; PacketParser[0x061] = &ValidatedPacketHandler<GP_CLI_COMMAND_CLISTATUS>;
    PacketSize[0x063] = 0x10; PacketParser[0x063] = &ValidatedPacketHandler<GP_CLI_COMMAND_DIG>;
    PacketSize[0x064] = 0x4C; PacketParser[0x064] = &ValidatedPacketHandler<GP_CLI_COMMAND_SCENARIOITEM>;
    PacketSize[0x066] = 0x0A; PacketParser[0x066] = &ValidatedPacketHandler<GP_CLI_COMMAND_FISHING>;
    PacketSize[0x06E] = 0x06; PacketParser[0x06E] = &SmallPacket0x06E;
    PacketSize[0x06F] = 0x00; PacketParser[0x06F] = &SmallPacket0x06F;
    PacketSize[0x070] = 0x06; PacketParser[0x070] = &ValidatedPacketHandler<GP_CLI_COMMAND_GROUP_BREAKUP>;
    PacketSize[0x071] = 0x1C; PacketParser[0x071] = &ValidatedPacketHandler<GP_CLI_COMMAND_GROUP_STRIKE>;
    PacketSize[0x074] = 0x06; PacketParser[0x074] = &ValidatedPacketHandler<GP_CLI_COMMAND_GROUP_SOLICIT_RES>;
    PacketSize[0x076] = 0x06; PacketParser[0x076] = &ValidatedPacketHandler<GP_CLI_COMMAND_GROUP_LIST_REQ>;
    PacketSize[0x077] = 0x16; PacketParser[0x077] = &ValidatedPacketHandler<GP_CLI_COMMAND_GROUP_CHANGE2>;
    PacketSize[0x078] = 0x04; PacketParser[0x078] = &ValidatedPacketHandler<GP_CLI_COMMAND_GROUP_CHECKID>;
    PacketSize[0x083] = 0x10; PacketParser[0x083] = &ValidatedPacketHandler<GP_CLI_COMMAND_SHOP_BUY>;
    PacketSize[0x084] = 0x0C; PacketParser[0x084] = &ValidatedPacketHandler<GP_CLI_COMMAND_SHOP_SELL_REQ>;
    PacketSize[0x085] = 0x06; PacketParser[0x085] = &ValidatedPacketHandler<GP_CLI_COMMAND_SHOP_SELL_SET>;
    PacketSize[0x096] = 0x22; PacketParser[0x096] = &ValidatedPacketHandler<GP_CLI_COMMAND_COMBINE_ASK>;
    PacketSize[0x09B] = 0x0C; PacketParser[0x09B] = &ValidatedPacketHandler<GP_CLI_COMMAND_CHOCOBO_RACE_REQ>;
    PacketSize[0x0A0] = 0x00; PacketParser[0x0A0] = &ValidatedPacketHandler<GP_CLI_COMMAND_SWITCH_PROPOSAL>;
    PacketSize[0x0A1] = 0x00; PacketParser[0x0A1] = &ValidatedPacketHandler<GP_CLI_COMMAND_SWITCH_VOTE>;
    PacketSize[0x0A2] = 0x08; PacketParser[0x0A2] = &ValidatedPacketHandler<GP_CLI_COMMAND_DICE>;
    PacketSize[0x0AA] = 0x00; PacketParser[0x0AA] = &ValidatedPacketHandler<GP_CLI_COMMAND_GUILD_BUY>;
    PacketSize[0x0AB] = 0x00; PacketParser[0x0AB] = &ValidatedPacketHandler<GP_CLI_COMMAND_GUILD_BUYLIST>;
    PacketSize[0x0AC] = 0x00; PacketParser[0x0AC] = &ValidatedPacketHandler<GP_CLI_COMMAND_GUILD_SELL>;
    PacketSize[0x0AD] = 0x00; PacketParser[0x0AD] = &ValidatedPacketHandler<GP_CLI_COMMAND_GUILD_SELLLIST>;
    PacketSize[0x0B5] = 0x00; PacketParser[0x0B5] = &ValidatedPacketHandler<GP_CLI_COMMAND_CHAT_STD>;
    PacketSize[0x0B6] = 0x00; PacketParser[0x0B6] = &ValidatedPacketHandler<GP_CLI_COMMAND_CHAT_NAME>;
    PacketSize[0x0B7] = 0x00; PacketParser[0x0B7] = &ValidatedPacketHandler<GP_CLI_COMMAND_ASSIST_CHANNEL>;
    PacketSize[0x0BE] = 0x0C; PacketParser[0x0BE] = &ValidatedPacketHandler<GP_CLI_COMMAND_MERITS>;
    PacketSize[0x0BF] = 0x04; PacketParser[0x0BF] = &ValidatedPacketHandler<GP_CLI_COMMAND_JOB_POINTS_SPEND>;
    PacketSize[0x0C0] = 0x00; PacketParser[0x0C0] = &ValidatedPacketHandler<GP_CLI_COMMAND_JOB_POINTS_REQ>;
    PacketSize[0x0C3] = 0x00; PacketParser[0x0C3] = &SmallPacket0x0C3;
    PacketSize[0x0C4] = 0x0E; PacketParser[0x0C4] = &SmallPacket0x0C4;
    PacketSize[0x0CB] = 0x04; PacketParser[0x0CB] = &SmallPacket0x0CB;
    PacketSize[0x0D2] = 0x04; PacketParser[0x0D2] = &ValidatedPacketHandler<GP_CLI_COMMAND_MAP_GROUP>;
    PacketSize[0x0D3] = 0x00; PacketParser[0x0D3] = &ValidatedPacketHandler<GP_CLI_COMMAND_FAQ_GMCALL>;
    PacketSize[0x0D4] = 0x04; PacketParser[0x0D4] = &ValidatedPacketHandler<GP_CLI_COMMAND_FAQ_GMPARAM>;
    PacketSize[0x0D5] = 0x08; PacketParser[0x0D5] = &ValidatedPacketHandler<GP_CLI_COMMAND_ACK_GMMSG>;
    PacketSize[0x0D8] = 0x00; PacketParser[0x0D8] = &ValidatedPacketHandler<GP_CLI_COMMAND_DUNGEON_PARAM>;
    PacketSize[0x0DB] = 0x28; PacketParser[0x0DB] = &ValidatedPacketHandler<GP_CLI_COMMAND_CONFIG_LANGUAGE>;
    PacketSize[0x0DC] = 0x14; PacketParser[0x0DC] = &ValidatedPacketHandler<GP_CLI_COMMAND_CONFIG>;
    PacketSize[0x0DD] = 0x0C; PacketParser[0x0DD] = &ValidatedPacketHandler<GP_CLI_COMMAND_EQUIP_INSPECT>;
    PacketSize[0x0DE] = 0x40; PacketParser[0x0DE] = &ValidatedPacketHandler<GP_CLI_COMMAND_INSPECT_MESSAGE>;
    PacketSize[0x0E0] = 0x00; PacketParser[0x0E0] = &ValidatedPacketHandler<GP_CLI_COMMAND_SET_USERMSG>;
    PacketSize[0x0E1] = 0x00; PacketParser[0x0E1] = &ValidatedPacketHandler<GP_CLI_COMMAND_GET_LSMSG>;
    PacketSize[0x0E2] = 0x00; PacketParser[0x0E2] = &ValidatedPacketHandler<GP_CLI_COMMAND_SET_LSMSG>;
    PacketSize[0x0E4] = 0x00; PacketParser[0x0E4] = &ValidatedPacketHandler<GP_CLI_COMMAND_GET_LSPRIV>;
    PacketSize[0x0E7] = 0x04; PacketParser[0x0E7] = &ValidatedPacketHandler<GP_CLI_COMMAND_REQLOGOUT>;
    PacketSize[0x0E8] = 0x04; PacketParser[0x0E8] = &ValidatedPacketHandler<GP_CLI_COMMAND_CAMP>;
    PacketSize[0x0EA] = 0x04; PacketParser[0x0EA] = &ValidatedPacketHandler<GP_CLI_COMMAND_SIT>;
    PacketSize[0x0EB] = 0x00; PacketParser[0x0EB] = &ValidatedPacketHandler<GP_CLI_COMMAND_REQSUBMAPNUM>;
    PacketSize[0x0F0] = 0x04; PacketParser[0x0F0] = &ValidatedPacketHandler<GP_CLI_COMMAND_RESCUE>;
    PacketSize[0x0F1] = 0x04; PacketParser[0x0F1] = &ValidatedPacketHandler<GP_CLI_COMMAND_BUFFCANCEL>;
    PacketSize[0x0F2] = 0x04; PacketParser[0x0F2] = &ValidatedPacketHandler<GP_CLI_COMMAND_SUBMAPCHANGE>;
    PacketSize[0x0F4] = 0x04; PacketParser[0x0F4] = &ValidatedPacketHandler<GP_CLI_COMMAND_TRACKING_LIST>;
    PacketSize[0x0F5] = 0x00; PacketParser[0x0F5] = &ValidatedPacketHandler<GP_CLI_COMMAND_TRACKING_START>;
    PacketSize[0x0F6] = 0x00; PacketParser[0x0F6] = &ValidatedPacketHandler<GP_CLI_COMMAND_TRACKING_END>;
    PacketSize[0x0FA] = 0x00; PacketParser[0x0FA] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_LAYOUT>;
    PacketSize[0x0FB] = 0x00; PacketParser[0x0FB] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_BANKIN>;
    PacketSize[0x0FC] = 0x00; PacketParser[0x0FC] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_PLANT_ADD>;
    PacketSize[0x0FD] = 0x00; PacketParser[0x0FD] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_PLANT_CHECK>;
    PacketSize[0x0FE] = 0x00; PacketParser[0x0FE] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_PLANT_CROP>;
    PacketSize[0x0FF] = 0x00; PacketParser[0x0FF] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_PLANT_STOP>;
    PacketSize[0x100] = 0x04; PacketParser[0x100] = &ValidatedPacketHandler<GP_CLI_COMMAND_MYROOM_JOB>;
    PacketSize[0x102] = 0x52; PacketParser[0x102] = &ValidatedPacketHandler<GP_CLI_COMMAND_EXTENDED_JOB>;
    PacketSize[0x104] = 0x02; PacketParser[0x104] = &ValidatedPacketHandler<GP_CLI_COMMAND_BAZAAR_EXIT>;
    PacketSize[0x105] = 0x06; PacketParser[0x105] = &ValidatedPacketHandler<GP_CLI_COMMAND_BAZAAR_LIST>;
    PacketSize[0x106] = 0x06; PacketParser[0x106] = &ValidatedPacketHandler<GP_CLI_COMMAND_BAZAAR_BUY>;
    PacketSize[0x109] = 0x00; PacketParser[0x109] = &ValidatedPacketHandler<GP_CLI_COMMAND_BAZAAR_OPEN>;
    PacketSize[0x10A] = 0x06; PacketParser[0x10A] = &ValidatedPacketHandler<GP_CLI_COMMAND_BAZAAR_ITEMSET>;
    PacketSize[0x10B] = 0x00; PacketParser[0x10B] = &ValidatedPacketHandler<GP_CLI_COMMAND_BAZAAR_CLOSE>;
    PacketSize[0x10C] = 0x04; PacketParser[0x10C] = &ValidatedPacketHandler<GP_CLI_COMMAND_ROE_START>;
    PacketSize[0x10D] = 0x04; PacketParser[0x10D] = &ValidatedPacketHandler<GP_CLI_COMMAND_ROE_REMOVE>;
    PacketSize[0x10E] = 0x04; PacketParser[0x10E] = &ValidatedPacketHandler<GP_CLI_COMMAND_ROE_CLAIM>;
    PacketSize[0x10F] = 0x02; PacketParser[0x10F] = &ValidatedPacketHandler<GP_CLI_COMMAND_CURRENCIES_1>;
    PacketSize[0x110] = 0x0A; PacketParser[0x110] = &ValidatedPacketHandler<GP_CLI_COMMAND_FISHING_2>;
    PacketSize[0x112] = 0x00; PacketParser[0x112] = &SmallPacket0x112;
    PacketSize[0x113] = 0x06; PacketParser[0x113] = &ValidatedPacketHandler<GP_CLI_COMMAND_SITCHAIR>;
    PacketSize[0x114] = 0x00; PacketParser[0x114] = &ValidatedPacketHandler<GP_CLI_COMMAND_MAP_MARKERS>;
    PacketSize[0x115] = 0x02; PacketParser[0x115] = &ValidatedPacketHandler<GP_CLI_COMMAND_CURRENCIES_2>;
    PacketSize[0x116] = 0x00; PacketParser[0x116] = &ValidatedPacketHandler<GP_CLI_COMMAND_UNITY_MENU>;
    PacketSize[0x117] = 0x00; PacketParser[0x117] = &ValidatedPacketHandler<GP_CLI_COMMAND_UNITY_QUEST>;
    PacketSize[0x118] = 0x00; PacketParser[0x118] = &ValidatedPacketHandler<GP_CLI_COMMAND_UNITY_TOGGLE>;
    PacketSize[0x119] = 0x00; PacketParser[0x119] = &ValidatedPacketHandler<GP_CLI_COMMAND_EMOTE_LIST>;
    PacketSize[0x11B] = 0x00; PacketParser[0x11B] = &ValidatedPacketHandler<GP_CLI_COMMAND_MASTERY_DISPLAY>;
    PacketSize[0x11C] = 0x08; PacketParser[0x11C] = &ValidatedPacketHandler<GP_CLI_COMMAND_PARTY_REQUEST>;
    PacketSize[0x11D] = 0x00; PacketParser[0x11D] = &ValidatedPacketHandler<GP_CLI_COMMAND_JUMP>;
    // clang-format on
}
