
#pragma once

enum
{
    JID_NONE                = 0x00,

    JID_EPHRAIM_LORD        = 0x01,
    JID_EIRIKA_LORD         = 0x02,
    JID_EPHRAIM_MASTER_LORD = 0x03,
    JID_EIRIKA_MASTER_LORD  = 0x04,
    JID_CAVALIER            = 0x05,
    JID_CAVALIER_F          = 0x06,
    JID_PALADIN             = 0x07,
    JID_PALADIN_F           = 0x08,
    JID_ARMOR_KNIGHT        = 0x09,
    JID_ARMOR_KNIGHT_F      = 0x0A,
    JID_GENERAL             = 0x0B,
    JID_GENERAL_F           = 0x0C,
    JID_THIEF               = 0x0D,
    JID_MANAKETE            = 0x0E, // TODO: which one?
    JID_MERCENARY           = 0x0F,
    JID_MERCENARY_F         = 0x10,
    JID_HERO                = 0x11,
    JID_HERO_F              = 0x12,
    JID_MYRMIDON            = 0x13,
    JID_MYRMIDON_F          = 0x14,
    JID_SWORDMASTER         = 0x15,
    JID_SWORDMASTER_F       = 0x16,
    JID_ASSASSIN            = 0x17,
    JID_ASSASSIN_F          = 0x18,
    JID_ARCHER              = 0x19,
    JID_ARCHER_F            = 0x1A,
    JID_SNIPER              = 0x1B,
    JID_SNIPER_F            = 0x1C,
    JID_RANGER              = 0x1D,
    JID_RANGER_F            = 0x1E,
    JID_WYVERN_RIDER        = 0x1F,
    JID_WYVERN_RIDER_F      = 0x20,
    JID_WYVERN_LORD         = 0x21,
    JID_WYVERN_LORD_F       = 0x22,
    JID_WYVERN_KNIGHT       = 0x23,
    JID_WYVERN_KNIGHT_F     = 0x24,
    JID_MAGE                = 0x25,
    JID_MAGE_F              = 0x26,
    JID_SAGE                = 0x27,
    JID_SAGE_F              = 0x28,
    JID_MAGE_KNIGHT         = 0x29,
    JID_MAGE_KNIGHT_F       = 0x2A,
    JID_BISHOP              = 0x2B,
    JID_BISHOP_F            = 0x2C,
    JID_SHAMAN              = 0x2D,
    JID_SHAMAN_F            = 0x2E,
    JID_DRUID               = 0x2F,
    JID_DRUID_F             = 0x30,
    JID_SUMMONER            = 0x31,
    JID_SUMMONER_F          = 0x32,
    JID_ROGUE               = 0x33,
    JID_GORGONEGG2          = 0x34,
    JID_GREAT_KNIGHT        = 0x35,
    JID_GREAT_KNIGHT_F      = 0x36,
    JID_RECRUIT_T1          = 0x37,
    JID_JOURNEYMAN_T2       = 0x38,
    JID_PUPIL_T2            = 0x39,
    JID_RECRUIT_T2          = 0x3A,
    JID_MANAKETE_2          = 0x3B, // TODO: which one?
    JID_MANAKETE_MYRRH      = 0x3C,
    JID_JOURNEYMAN          = 0x3D,
    JID_PUPIL               = 0x3E,
    JID_FIGHTER             = 0x3F,
    JID_WARRIOR             = 0x40,
    JID_BRIGAND             = 0x41,
    JID_PIRATE              = 0x42,
    JID_BERSERKER           = 0x43,
    JID_MONK                = 0x44,
    JID_PRIEST              = 0x45,
    JID_BARD                = 0x46,
    JID_RECRUIT             = 0x47,
    JID_PEGASUS_KNIGHT      = 0x48,
    JID_FALCON_KNIGHT       = 0x49,
    JID_CLERIC              = 0x4A,
    JID_TROUBADOUR          = 0x4B,
    JID_VALKYRIE            = 0x4C,
    JID_DANCER              = 0x4D,
    JID_SOLDIER             = 0x4E,
    JID_NECROMANCER         = 0x4F,
    JID_FLEET               = 0x50,
    JID_PHANTOM             = 0x51,
    JID_REVENANT            = 0x52,
    JID_ENTOUMBED           = 0x53,
    JID_BONEWALKER          = 0x54,
    JID_BONEWALKER_BOW      = 0x55,
    JID_WIGHT               = 0x56,
    JID_WIGHT_BOW           = 0x57,
    JID_BAEL                = 0x58,
    JID_ELDER_BAEL          = 0x59,
    JID_CYCLOPS             = 0x5A,
    JID_MAUTHEDOOG          = 0x5B,
    JID_GWYLLGI             = 0x5C,
    JID_TARVOS              = 0x5D,
    JID_MAELDUIN            = 0x5E,
    JID_MOGALL              = 0x5F,
    JID_ARCH_MOGALL         = 0x60,
    JID_GORGON              = 0x61,
    JID_GORGONEGG           = 0x62,
    JID_GARGOYLE            = 0x63,
    JID_DEATHGOYLE          = 0x64,
    JID_DRACO_ZOMBIE        = 0x65,
    JID_DEMON_KING          = 0x66,
    JID_BLST_REGULAR_USED   = 0x67,
    JID_BLST_LONG_USED      = 0x68,
    JID_BLST_KILLER_USED    = 0x69,
    JID_BLST_REGULAR_EMPTY  = 0x6A,
    JID_BLST_LONG_EMPTY     = 0x6B,
    JID_BLST_KILLER_EMPTY   = 0x6C,
    JID_CIVILIAN_M1         = 0x6D,
    JID_CIVILIAN_F1         = 0x6E,
    JID_CIVILIAN_M2         = 0x6F,
    JID_CIVILIAN_F2         = 0x70,
    JID_CIVILIAN_M3         = 0x71,
    JID_CIVILIAN_F3         = 0x72,
    JID_PEER                = 0x73,
    JID_QUEEN               = 0x74,
    JID_PRINCE              = 0x75,
    JID_QUEEN_2             = 0x76,
    JID_UNK78               = 0x78, // TODO: figure out
    JID_FALLEN_PRINCE       = 0x78,
    JID_TENT                = 0x79,
    JID_PONTIFEX            = 0x7A,
    JID_FALLEN_PEER         = 0x7B,
    JID_CYCLOPS_2           = 0x7C,
    JID_ELDER_BAEL_2        = 0x7D,
    JID_JOURNEYMAN_T1       = 0x7E,
    JID_PUPIL_T1            = 0x7F,

    // Hiding the game's misery
    JID_OBSTACLE = JID_EPHRAIM_LORD,
};