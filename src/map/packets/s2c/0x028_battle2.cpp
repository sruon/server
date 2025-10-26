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

#include "0x028_battle2.h"

#include "action/action.h"
#include "entities/charentity.h"
#include "enums/action/category.h"
#include "enums/four_cc.h"

// Packs the action into the format expected by the FFXI client
// Use 'actionparse' by atom0s as reference when making changes.
void GP_SERV_COMMAND_BATTLE2::bitpackAction(const Action& action)
{
    const auto packet = buffer_.data();

    // Start at bit 8 (skip first 5 bytes)
    uint32_t bitOffset = 8 * 5;

    const uint8_t trg_sum = std::min<uint8_t>(static_cast<uint8_t>(action.targetCount()), 64);

    // Pack header fields in exact order from XiPackets
    bitOffset = packBitsBE(packet, action.actorId(), bitOffset, 32);                        // Caster ID
    bitOffset = packBitsBE(packet, trg_sum, bitOffset, 6);                                  // Target count
    bitOffset = packBitsBE(packet, 0, bitOffset, 4);                                        // res_sum is always 0!
    bitOffset = packBitsBE(packet, static_cast<uint32_t>(action.category()), bitOffset, 4); // Command type
    bitOffset = packBitsBE(packet,
                           std::visit([](auto&& arg) -> uint32_t
                                      { return static_cast<uint32_t>(arg); }, action.argument()),
                           bitOffset, 32);
    // Command argument
    bitOffset = packBitsBE(packet, action.info(), bitOffset, 32); // Action info

    // Pack targets
    uint8_t targetCount = 0;
    for (const auto& target : action.targets())
    {
        if (targetCount >= 64)
            break;
        targetCount++;

        // Pack target header
        bitOffset                 = packBitsBE(packet, target.targetId(), bitOffset, 32);
        const uint8_t resultCount = std::min<uint8_t>(static_cast<uint8_t>(target.resultCount()), 8);
        bitOffset                 = packBitsBE(packet, resultCount, bitOffset, 4);

        // Pack results for this target
        uint8_t resultIdx = 0;
        for (const auto& result : target.results())
        {
            if (resultIdx >= 8)
                break;
            resultIdx++;

            // Core result fields (85 bits)
            bitOffset = packBitsBE(packet, static_cast<uint8_t>(result.resolution()), bitOffset, 3);
            bitOffset = packBitsBE(packet, result.kind(), bitOffset, 2);
            bitOffset = packBitsBE(packet, std::visit([](auto&& arg) -> uint16_t
                                                      { return static_cast<uint16_t>(arg); }, result.subKind()),
                                   bitOffset, 12);
            bitOffset = packBitsBE(packet,
                                   std::visit([](auto&& arg) -> uint8_t
                                              { return static_cast<uint8_t>(arg); }, result.info()),
                                   bitOffset, 5);
            bitOffset = packBitsBE(packet, static_cast<uint8_t>(result.distortion()), bitOffset, 2);
            bitOffset = packBitsBE(packet, static_cast<uint8_t>(result.knockback()), bitOffset, 3);
            bitOffset = packBitsBE(packet, result.value(), bitOffset, 17);
            bitOffset = packBitsBE(packet, result.message(), bitOffset, 10);
            bitOffset = packBitsBE(packet, static_cast<uint32_t>(result.modifier()), bitOffset, 31);

            // Conditional proc block
            bitOffset = packBitsBE(packet, result.hasProc() ? 1 : 0, bitOffset, 1);
            if (result.hasProc())
            {
                auto proc = result.proc();
                bitOffset = packBitsBE(packet, std::visit([](auto&& arg) -> uint8_t
                                                          { return static_cast<uint8_t>(arg); }, proc.type),
                                       bitOffset, 6);
                bitOffset = packBitsBE(packet, proc.info, bitOffset, 4);
                bitOffset = packBitsBE(packet, proc.value, bitOffset, 17);
                bitOffset = packBitsBE(packet, proc.message, bitOffset, 10);
            }

            // Conditional reaction block
            bitOffset = packBitsBE(packet, result.hasReaction() ? 1 : 0, bitOffset, 1);
            if (result.hasReaction())
            {
                auto react = result.reaction();
                bitOffset  = packBitsBE(packet, static_cast<uint8_t>(react.type), bitOffset, 6);
                bitOffset  = packBitsBE(packet, react.info, bitOffset, 4);
                bitOffset  = packBitsBE(packet, react.value, bitOffset, 14);
                bitOffset  = packBitsBE(packet, react.message, bitOffset, 10);
            }
        }
    }

    // Client ignores it but we'll set it anyway.
    const uint8_t workSize = (bitOffset >> 3) + (bitOffset % 8 != 0);
    ref<uint8>(0x04)       = workSize;

    setSize(workSize + 1);
}

void GP_SERV_COMMAND_BATTLE2::normalize(Action& action) const
{
    // We'll set certain values before bitpacking the data
    // This is only for UNIVERSAL RULES explicitly documented on XiPackets
    // No one-offs, hacks or any other unmaintainable mess
    // These can go in the ability/skill/spell specific lua.

    // Callers put the cooldowns in info but this is never sent except for magic finishes.
    // Relevant information come through other packets for JAs
    if (action.category() != ActionCategory::MagicFinish)
    {
        action.setInfo(0);
    }

    switch (action.category())
    {
        case ActionCategory::BasicAttack:
        {
            // cmd_arg is always "atk0"
            action.setArgument(FourCC::Attack);

            // result.kind is always 1
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 1;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::RangedFinish:
        {
            // cmd_arg is always "shlg"
            action.setArgument(FourCC::RangeFinish);

            // result.kind is always 2
            // Note: XiPackets claim this is always 1
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 2;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::SkillFinish:
        {
            // result.kind is always 3
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 3;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::MagicFinish:
        {
            // result.kind is always 0
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 0;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::ItemFinish:
        {
            // result.kind is always 1
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 1;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::AbilityFinish:
        {
            // result.kind is always 2
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 2;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::SkillStart:
        {
            // result.kind is always 1
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 1;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::ItemStart:
        {
            // result.kind is always 1
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.kind = 1;
            // });
            // clang-format on
            break;
        }
        case ActionCategory::AbilityStart:
        {
            break;
        }
        case ActionCategory::MobSkillFinish:
        {
            // result.kind is always 2 for monster skills
            // result.kind is always 3 for trust attacks
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.setKind(2);
            //     // TODO: Trust
            // });
            // clang-format on
            break;
        }
        case ActionCategory::RangedStart:
        {
            // result.kind is always 0
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.setKind(0);
            // });
            // clang-format on
            break;
        }
        case ActionCategory::Unknown:
        {
            break;
        }
        case ActionCategory::Dancer:
        {
            // result.kind is always 2
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.setKind(2);
            // });
            // clang-format on
            break;
        }
        case ActionCategory::RuneFencer:
        {
            // result.kind is always 3
            // clang-format off
            // action.ForAllResults([](ActionResult& result)
            // {
            //    result.setKind(3);
            // });
            // clang-format on
            break;
        }
        default:
            break;
    }
}

GP_SERV_COMMAND_BATTLE2::GP_SERV_COMMAND_BATTLE2(Action& action)
{
    normalize(action);
    bitpackAction(action);
}
