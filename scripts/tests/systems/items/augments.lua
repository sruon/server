local augment =
{
    ACCURACY       = 23,
    ATTACK_DOWN    = 26,
    HASTE          = 49,
    EXP_POINT      = 72,
    PET_ACC_RACC   = 96,
    AVATAR_MAG_ATK = 120,
    ACC_RACC       = 129,
}

describe('Augments', function()
    ---@type CClientEntityPair
    local player

    ---@param augments { id: integer, value: integer }[]
    local function addRing(augments)
        local item = player:addItem({ id = xi.item.BRASS_RING, quantity = 1 })
        assert(item)

        item:setExData(
            {
                augmentKind    = xi.augment.kind.HAS_AUGMENTS,
                augmentSubKind = xi.augment.subKind.STANDARD,
                augments       = augments,
            })

        return item
    end

    before_each(function()
        player = xi.test.world:spawnPlayer(
            {
                job   = xi.job.SMN,
                level = 75,
                zone  = xi.zone.WEST_RONFAURE,
            })
    end)

    it('adds the augment value away from zero', function()
        local ring = addRing({ { id = augment.ACCURACY, value = 4 }, { id = augment.ATTACK_DOWN, value = 2 } })

        assert(ring:getMod(xi.mod.ACC) == 5)
        assert(ring:getMod(xi.mod.ATT) == -3)
    end)

    it('grants every mod of a combined augment', function()
        local ring = addRing({ { id = augment.ACC_RACC, value = 3 } })

        assert(ring:getMod(xi.mod.ACC) == 4)
        assert(ring:getMod(xi.mod.RACC) == 4)
    end)

    it('scales the total by the multiplier', function()
        local ring = addRing({ { id = augment.HASTE, value = 2 } })

        assert(ring:getMod(xi.mod.HASTE_GEAR) == 300)
    end)

    it('marks the item augmented for an ID with no mods', function()
        local ring = addRing({ { id = augment.EXP_POINT, value = 5 } })

        assert(ring:isSubType(xi.itemSubType.AUGMENTED))
        assert(ring:getMod(xi.mod.EXP_BONUS) == 0)
    end)

    it('gives pet augments to the pet, not the wearer', function()
        addRing({ { id = augment.PET_ACC_RACC, value = 4 }, { id = augment.AVATAR_MAG_ATK, value = 2 } })
        player:spawnPet(xi.petId.CARBUNCLE)
        local pet = player:getPet()
        assert(pet)

        local wearerAccuracy = player:getMod(xi.mod.ACC)
        local accuracy       = pet:getMod(xi.mod.ACC)
        local rangedAccuracy = pet:getMod(xi.mod.RACC)
        local magicAttack    = pet:getMod(xi.mod.MATT)

        player:equipItem(xi.item.BRASS_RING, nil, xi.slot.RING1)
        assert(player:getEquippedItem(xi.slot.RING1))

        assert(player:getMod(xi.mod.ACC) == wearerAccuracy)
        assert(pet:getMod(xi.mod.ACC) == accuracy + 5)
        assert(pet:getMod(xi.mod.RACC) == rangedAccuracy + 5)
        assert(pet:getMod(xi.mod.MATT) == magicAttack + 3)
    end)
end)
