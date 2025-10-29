-----------------------------------
-- ACTION IDs
-----------------------------------
xi = xi or {}

---@enum xi.action
xi.action =
{
    NONE                  = 0,
    ATTACK                = 1,
    RANGED_FINISH         = 2,
    WEAPONSKILL_FINISH    = 3,
    MAGIC_FINISH          = 4,
    ITEM_FINISH           = 5,
    JOBABILITY_FINISH     = 6,
    WEAPONSKILL_START     = 7,
    MAGIC_START           = 8,
    ITEM_START            = 9,
    JOBABILITY_START      = 10,
    MOBABILITY_FINISH     = 11,
    RANGED_START          = 12,
    PET_MOBABILITY_FINISH = 13,
    DANCE                 = 14,
    RUN_WARD_EFFUSION     = 15,
    ROAMING               = 16,
    ENGAGE                = 17,
    DISENGAGE             = 18,
    CHANGE_TARGET         = 19,
    FALL                  = 20,
    DROPITEMS             = 21,
    DEATH                 = 22,
    FADE_OUT              = 23,
    DESPAWN               = 24,
    SPAWN                 = 25,
    STUN                  = 26,
    SLEEP                 = 27,
    ITEM_USING            = 28,
    ITEM_INTERRUPT        = 29,
    MAGIC_CASTING         = 30,
    MAGIC_INTERRUPT       = 31,
    RANGED_INTERRUPT      = 32,
    MOBABILITY_START      = 33,
    MOBABILITY_USING      = 34,
    MOBABILITY_INTERRUPT  = 35,
    LEAVE                 = 36,
    RAISE_MENU_SELECTION  = 37,
    JOBABILITY_INTERRUPT  = 38,
}

xi.battle = {}

---@enum xi.battle.resolution
xi.battle.resolution =
{
    HIT   = 0,
    MISS  = 1,
    GUARD = 2,
    PARRY = 3,
    BLOCK = 4,
}

---@enum xi.battle.category
xi.battle.category =
{
    NONE                = 0,
    BASIC_ATTACK        = 1,
    RANGED_FINISH       = 2,
    SKILL_FINISH        = 3,
    MAGIC_FINISH        = 4,
    ITEM_FINISH         = 5,
    ABILITY_FINISH      = 6,
    SKILL_START         = 7,
    MAGIC_START         = 8,
    ITEM_START          = 9,
    ABILITY_START       = 10,
    MONSTERSKILL_FINISH = 11,
    RANGED_START        = 12,
    UNKNOWN             = 13,
    DANCER              = 14,
    RUNE_FENCER         = 15,
}

xi.battle.fourCC =
{
    ATTACK                   = 0x306B7461, -- "atk0"
    SKILL_USE                = 0x65746163, -- "cate"
    SKILL_INTERRUPT          = 0x65747073, -- "spte"
    ITEM_USE                 = 0x74696163, -- "cait"
    ITEM_INTERRUPT           = 0x74697073, -- "spit"
    RANGE_START              = 0x676C6163, -- "calg"
    RANGE_INTERRUPT          = 0x676C7073, -- "splg"
    RANGE_FINISH             = 0x676C6873, -- "shlg"
    WHITE_MAGIC_CAST         = 0x68776163, -- "cawh"
    BLACK_MAGIC_CAST         = 0x6B626163, -- "cabk"
    BLUE_MAGIC_CAST          = 0x6C626163, -- "cabl"
    SONG_MAGIC_CAST          = 0x6F736163, -- "caso"
    NINJUTSU_MAGIC_CAST      = 0x6A6E6163, -- "canj"
    SUMMON_MAGIC_CAST        = 0x6D736163, -- "casm"
    GEOMANCY_MAGIC_CAST      = 0x65676163, -- "cage"
    TRUST_MAGIC_CAST         = 0x61666163, -- "cafa"
    WHITE_MAGIC_INTERRUPT    = 0x68777073, -- "spwh"
    BLACK_MAGIC_INTERRUPT    = 0x6B627073, -- "spbk"
    BLUE_MAGIC_INTERRUPT     = 0x6C627073, -- "spbl"
    SONG_MAGIC_INTERRUPT     = 0x6F737073, -- "spso"
    NINJUTSU_MAGIC_INTERRUPT = 0x6A6E7073, -- "spnj"
    SUMMON_MAGIC_INTERRUPT   = 0x6D737073, -- "spsm"
    GEOMANCY_MAGIC_INTERRUPT = 0x65677073, -- "spge"
    TRUST_MAGIC_INTERRUPT    = 0x61667073, -- "spfa"
}
