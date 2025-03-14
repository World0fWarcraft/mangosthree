/**
 * ScriptDev3 is an extension for mangos providing enhanced features for
 * area triggers, creatures, game objects, instances, items, and spells beyond
 * the default database scripting in mangos.
 *
 * Copyright (C) 2006-2013 ScriptDev2 <http://www.scriptdev2.com/>
 * Copyright (C) 2014-2025 MaNGOS <https://www.getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#ifndef DEF_NAXXRAMAS_H
#define DEF_NAXXRAMAS_H

enum
{
    MAX_ENCOUNTER               = 16,

    // A few instance-script related texts
    SAY_THADDIUS_GREET          = -1533029,

    // Kel'Thuzad
    SAY_KELTHUZAD_CAT_DIED      = -1533089,
    // Kel'Thuzad's taunts after killing Wing Bosses
    SAY_KELTHUZAD_TAUNT1        = -1533090,
    SAY_KELTHUZAD_TAUNT2        = -1533091,
    SAY_KELTHUZAD_TAUNT3        = -1533092,
    SAY_KELTHUZAD_TAUNT4        = -1533093,
    // Dialogues with Lich King
    SAY_SAPP_DIALOG1            = -1533084,
    SAY_SAPP_DIALOG2_LICH       = -1533085,
    SAY_SAPP_DIALOG3            = -1533086,
    SAY_SAPP_DIALOG4_LICH       = -1533087,
    SAY_SAPP_DIALOG5            = -1533088,
    // Horsemen dialogue texts
    SAY_BLAU_TAUNT1             = -1533045,
    SAY_BLAU_TAUNT2             = -1533046,
    SAY_BLAU_TAUNT3             = -1533047,             // NYI - requires additiona research
    SAY_MORG_TAUNT1             = -1533071,
    SAY_MORG_TAUNT2             = -1533072,
    SAY_MORG_TAUNT3             = -1533073,             // NYI - requires additiona research
    SAY_KORT_TAUNT1             = -1533052,
    SAY_KORT_TAUNT2             = -1533053,
    SAY_KORT_TAUNT3             = -1533054,             // NYI - requires additiona research
    SAY_ZELI_TAUNT1             = -1533059,
    SAY_ZELI_TAUNT2             = -1533060,
    SAY_ZELI_TAUNT3             = -1533061,             // NYI - requires additiona research

    TYPE_ANUB_REKHAN            = 0,
    TYPE_FAERLINA               = 1,
    TYPE_MAEXXNA                = 2,

    TYPE_NOTH                   = 3,
    TYPE_HEIGAN                 = 4,
    TYPE_LOATHEB                = 5,

    TYPE_RAZUVIOUS              = 6,
    TYPE_GOTHIK                 = 7,
    TYPE_FOUR_HORSEMEN          = 8,

    TYPE_PATCHWERK              = 9,
    TYPE_GROBBULUS              = 10,
    TYPE_GLUTH                  = 11,
    TYPE_THADDIUS               = 12,

    TYPE_SAPPHIRON              = 13,
    TYPE_KELTHUZAD              = 14,
    TYPE_SIGNAL_1               = MAX_ENCOUNTER,
    TYPE_SIGNAL_2               = MAX_ENCOUNTER + 1,
    TYPE_SIGNAL_3               = MAX_ENCOUNTER + 2,
    TYPE_SIGNAL_4               = MAX_ENCOUNTER + 3,
    TYPE_SIGNAL_5               = MAX_ENCOUNTER + 4,
    TYPE_SIGNAL_6               = MAX_ENCOUNTER + 5,
    TYPE_SIGNAL_7               = MAX_ENCOUNTER + 6,
    TYPE_SIGNAL_8               = MAX_ENCOUNTER + 7,
    TYPE_SIGNAL_9               = MAX_ENCOUNTER + 8,
    TYPE_SIGNAL_10              = MAX_ENCOUNTER + 9,
    TYPE_SIGNAL_11              = MAX_ENCOUNTER + 10,

    MAX_HEIGAN_TRAP_AREAS       = 4,

    NPC_ANUB_REKHAN             = 15956,
    NPC_FAERLINA                = 15953,

    NPC_THADDIUS                = 15928,
    NPC_STALAGG                 = 15929,
    NPC_FEUGEN                  = 15930,
    NPC_TESLA_COIL              = 16218,

    NPC_MOGRAINE                = 16062,
    NPC_ZELIEK                  = 16063,
    NPC_THANE                   = 16064,
    NPC_BLAUMEUX                = 16065,

    NPC_SPIRIT_OF_MOGRAINE      = 16775,
    NPC_SPIRIT_OF_BLAUMEUX      = 16776,
    NPC_SPIRIT_OF_ZELIREK       = 16777,
    NPC_SPIRIT_OF_KORTHAZZ      = 16778,

    NPC_SAPPHIRON               = 15989,
    NPC_KELTHUZAD               = 15990,
    NPC_THE_LICHKING            = 16980,
    NPC_MR_BIGGLESWORTH         = 16998,

    // Gothik
    NPC_GOTHIK                  = 16060,
    NPC_SUB_BOSS_TRIGGER        = 16137,                    // summon locations
    NPC_UNREL_TRAINEE           = 16124,
    NPC_UNREL_DEATH_KNIGHT      = 16125,
    NPC_UNREL_RIDER             = 16126,
    NPC_SPECT_TRAINEE           = 16127,
    NPC_SPECT_DEATH_KNIGHT      = 16148,
    NPC_SPECT_RIDER             = 16150,
    NPC_SPECT_HORSE             = 16149,

    // Kel'Thuzad
    NPC_SOLDIER_FROZEN          = 16427,
    NPC_UNSTOPPABLE_ABOM        = 16428,
    NPC_SOUL_WEAVER             = 16429,
    NPC_GUARDIAN                = 16441,

    // Arachnid Quarter
    GO_ARAC_ANUB_DOOR           = 181126,                   // encounter door
    GO_ARAC_ANUB_GATE           = 181195,                   // open after boss is dead
    GO_ARAC_FAER_WEB            = 181235,                   // encounter door
    GO_ARAC_FAER_DOOR           = 194022,                   // after faerlina, to outer ring
    GO_ARAC_MAEX_INNER_DOOR     = 181197,                   // encounter door
    GO_ARAC_MAEX_OUTER_DOOR     = 181209,                   // right before maex

    // Plague Quarter
    GO_PLAG_SLIME01_DOOR        = 181198,                   // not used
    GO_PLAG_SLIME02_DOOR        = 181199,                   // not used
    GO_PLAG_NOTH_ENTRY_DOOR     = 181200,                   // encounter door
    GO_PLAG_NOTH_EXIT_DOOR      = 181201,                   // exit, open when boss dead
    GO_PLAG_HEIG_ENTRY_DOOR     = 181202,
    GO_PLAG_HEIG_EXIT_DOOR      = 181203,                   // exit door - not used here
    GO_PLAG_HEIG_EXIT_HALLWAY   = 181496,                   // exit door, at the end of the hallway
    GO_PLAG_LOAT_DOOR           = 181241,                   // encounter door

    // Military Quarter
    GO_MILI_GOTH_ENTRY_GATE     = 181124,                   // used while encounter is in progress
    GO_MILI_GOTH_EXIT_GATE      = 181125,                   // exit, open at boss dead
    GO_MILI_GOTH_COMBAT_GATE    = 181170,                   // used while encounter is in progress
    GO_MILI_HORSEMEN_DOOR       = 181119,                   // encounter door

    GO_CHEST_HORSEMEN_NORM      = 181366,                   // four horsemen event, DoRespawnGameObject() when event == DONE

    // Construct Quarter
    GO_CONS_PATH_EXIT_DOOR      = 181123,
    GO_CONS_GLUT_EXIT_DOOR      = 181120,
    GO_CONS_THAD_DOOR           = 181121,                   // Thaddius enc door
    GO_CONS_NOX_TESLA_FEUGEN    = 181477,
    GO_CONS_NOX_TESLA_STALAGG   = 181478,

    // Frostwyrm Lair
    GO_KELTHUZAD_WATERFALL_DOOR = 181225,                   // exit, open after sapphiron is dead
    GO_KELTHUZAD_EXIT_DOOR      = 181228,
    GO_KELTHUZAD_WINDOW_1       = 181402,
    GO_KELTHUZAD_WINDOW_2       = 181403,
    GO_KELTHUZAD_WINDOW_3       = 181404,
    GO_KELTHUZAD_WINDOW_4       = 181405,

    // Eyes
    GO_ARAC_EYE_RAMP            = 181212,
    GO_PLAG_EYE_RAMP            = 181211,
    GO_MILI_EYE_RAMP            = 181210,
    GO_CONS_EYE_RAMP            = 181213,

    GO_ARAC_EYE_BOSS            = 181233,
    GO_PLAG_EYE_BOSS            = 181231,
    GO_MILI_EYE_BOSS            = 181230,
    GO_CONS_EYE_BOSS            = 181232,

    // Portals
    GO_ARAC_PORTAL              = 181575,
    GO_PLAG_PORTAL              = 181577,
    GO_MILI_PORTAL              = 181578,
    GO_CONS_PORTAL              = 181576,

    AREATRIGGER_FROSTWYRM       = 4120,                     // not needed here, but AT to be scripted
    AREATRIGGER_KELTHUZAD       = 4112,
    AREATRIGGER_GOTHIK          = 4116,
    AREATRIGGER_THADDIUS_DOOR   = 4113,
    AREATRIGGER_FROSTWYRM_TELE  = 4156,

    //Gothik dummy spells, implemented in InstanceScript actually
    SPELL_A_TO_ANCHOR_1 = 27892,
    SPELL_B_TO_ANCHOR_1 = 27928,
    SPELL_C_TO_ANCHOR_1 = 27935,

    SPELL_A_TO_ANCHOR_2 = 27893,
    SPELL_B_TO_ANCHOR_2 = 27929,
    SPELL_C_TO_ANCHOR_2 = 27936,

    SPELL_A_TO_SKULL = 27915,
    SPELL_B_TO_SKULL = 27931,
    SPELL_C_TO_SKULL = 27937,
};

#endif
