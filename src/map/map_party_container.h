//
// Created by sruon on 5/17/2025.
//

#pragma once

#include "common/ipc_structs.h"
#include "entities/charentity.h"
#include "utils/zoneutils.h"
#include "map_party.h"


class PartyContainer
{
public:
    void updateParty(const ipc::PartyUpdate& message);

private:
    std::unordered_map<uint32, std::unique_ptr<CCharParty>> parties_;
};
