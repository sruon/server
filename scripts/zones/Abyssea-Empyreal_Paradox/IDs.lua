-----------------------------------
-- Area: Abyssea-Empyreal_Paradox
-----------------------------------
zones = zones or {}

zones[xi.zone.ABYSSEA_EMPYREAL_PARADOX] =
{
    text =
    {
        ITEM_CANNOT_BE_OBTAINED       = 6384, -- You cannot obtain the <item>. Come back after sorting your inventory.
        ITEM_OBTAINED                 = 6390, -- Obtained: <item>.
        GIL_OBTAINED                  = 6391, -- Obtained <number> gil.
        KEYITEM_OBTAINED              = 6393, -- Obtained key item: <keyitem>.
        CRUOR_TOTAL                   = 6988, -- Obtained <number> cruor. (Total: <number>)
        CARRIED_OVER_POINTS           = 7001, -- You have carried over <number> login point[/s].
        LOGIN_CAMPAIGN_UNDERWAY       = 7002, -- The [/January/February/March/April/May/June/July/August/September/October/November/December] <number> Login Campaign is currently underway!
        LOGIN_NUMBER                  = 7003, -- In celebration of your most recent login (login no. <number>), we have provided you with <number> points! You currently have a total of <number> points.
        MEMBERS_LEVELS_ARE_RESTRICTED = 7023, -- Your party is unable to participate because certain members' levels are restricted.
        ATTACK_STAGGERS_THE_FIEND     = 7231, -- <name>'s attack staggers the fiend!
        YELLOW_STAGGER                = 7232, -- The fiend is unable to cast magic.
        BLUE_STAGGER                  = 7233, -- The fiend is unable to use special attacks.
        RED_STAGGER                   = 7234, -- The fiend is frozen in its tracks.
        YELLOW_WEAKNESS               = 7235, -- The fiend appears vulnerable to [/fire/ice/wind/earth/lightning/water/light/darkness] elemental magic!
        BLUE_WEAKNESS                 = 7236, -- The fiend appears vulnerable to [/hand-to-hand/dagger/sword/great sword/axe/great axe/scythe/polearm/katana/great katana/club/staff/archery/marksmanship] weapon skills!
        RED_WEAKNESS                  = 7237, -- The fiend appears vulnerable to [/fire/ice/wind/earth/lightning/water/light/darkness] elemental weapon skills!
        CRUOR_OBTAINED                = 7410, -- <name> obtained <number> cruor.
        PARTY_MEMBERS_HAVE_FALLEN     = 8060, -- All party members have fallen in battle. Now leaving the battlefield.
        THE_PARTY_WILL_BE_REMOVED     = 8067, -- If all party members' HP are still zero after # minute[/s], the party will be removed from the battlefield.
    },
    mob =
    {
    },
    npc =
    {
    },
}

return zones[xi.zone.ABYSSEA_EMPYREAL_PARADOX]
