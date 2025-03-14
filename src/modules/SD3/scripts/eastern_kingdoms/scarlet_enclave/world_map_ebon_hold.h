﻿/**
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

#ifndef DEF_EBON_HOLD_H
#define DEF_EBON_HOLD_H

enum
{
    TYPE_BATTLE                         = 0,
    TYPE_GOTHIK_YELL                    = 1,
    TYPE_DO_ACTION                      = 2,
    // also 3 worldstates, above 3500

    DATA_ACTION_ENABLE_TRAPS            = 0,
    DATA_ACTION_MOVE                    = 1,
    DATA_ACTION_DESPAWN                 = 2,

    // npcs
    // death knights
    NPC_HIGHLORD_DARION_MOGRAINE        = 29173,
    NPC_KOLTIRA_DEATHWEAVER             = 29199,
    NPC_ORBAZ_BLOODBANE                 = 29204,
    NPC_THASSARIAN                      = 29200,

    // scourge warriors - summond during the event
    NPC_FLESH_BEHEMOTH                  = 29190,
    NPC_RAMPAGING_ABOMINATION           = 29186,
    NPC_VOLATILE_GHOUL                  = 29219,
    NPC_WARRIOR_OF_THE_FROZEN_WASTES    = 29206,

    // argent dawn commanders
    NPC_HIGHLORD_TIRION_FORDRING        = 29175,
    NPC_KORFAX_CHAMPION_OF_THE_LIGHT    = 29176,
    NPC_LORD_MAXWELL_TYROSUS            = 29178,
    NPC_COMMANDER_ELIGOR_DAWNBRINGER    = 29177,
    NPC_LEONID_BARTHALOMEW_THE_REVERED  = 29179,
    NPC_DUKE_NICHOLAS_ZVERENHOFF        = 29180,
    NPC_RIMBLAT_EARTHSHATTER            = 29182,
    NPC_RAYNE                           = 29181,

    // argent warriors
    NPC_DEFENDER_OF_THE_LIGHT           = 29174,

    // cinematic
    NPC_THE_LICH_KING                   = 29183,
    NPC_HIGHLORD_ALEXANDROS_MOGRAINE    = 29227,
    NPC_DARION_MOGRAINE                 = 29228,

    // object
    GO_LIGHT_OF_DAWN                    = 191330,
    GO_HOLY_LIGHTNING_1                 = 191301,
    GO_HOLY_LIGHTNING_2                 = 191302,

    // spells
    SPELL_CHAPTER_IV                    = 53405,            // phase aura

    // variables
    MAX_LIGHT_CHAMPIONS                 = 7,                // the number of the light champions
    MAX_WARRIORS_SUMMONED_PER_TURN      = 5,                // summoned warriors (light and death) per turn
    MAX_LIGHT_GUARDS                    = 4,                // guards summond for the outro

    // event variables
    MAX_BATTLE_INTRO_TIMER              = 5,
    MAX_FORCES_LIGHT                    = 300,
    MAX_FORCES_SCOURGE                  = 10000,

    // world states
    // basically world states should be shown to all players with phase mask = 128 as stated in DBC
    // because we don't have the possibility to do that we'll just iterate through the players and set the phase mask manually based on the battle status
    WORLD_STATE_FORCES_SHOW             = 3592,             // show the remaining units
    WORLD_STATE_FORCES_SCOURGE          = 3591,
    WORLD_STATE_FORCES_LIGHT            = 3590,
    WORLD_STATE_BATTLE_TIMER_SHOW       = 3603,             // countdown timer
    WORLD_STATE_BATTLE_TIMER_TIME       = 3604,
    WORLD_STATE_BATTLE_BEGIN            = 3605,             // battle has begun
};

struct sSpawnLocation
{
    float m_fX, m_fY, m_fZ, m_fO;
    uint32 m_uiEntry;
};

// light champions
static sSpawnLocation aLightArmySpawnLoc[MAX_LIGHT_CHAMPIONS] =
{
    {2285.80f, -5308.82f, 87.04f, 1.67f, NPC_KORFAX_CHAMPION_OF_THE_LIGHT},
    {2276.96f, -5309.36f, 86.66f, 1.61f, NPC_LORD_MAXWELL_TYROSUS},
    {2279.82f, -5322.61f, 88.95f, 1.54f, NPC_LEONID_BARTHALOMEW_THE_REVERED},
    {2287.96f, -5313.96f, 88.27f, 1.63f, NPC_DUKE_NICHOLAS_ZVERENHOFF},
    {2276.84f, -5313.78f, 87.62f, 1.61f, NPC_COMMANDER_ELIGOR_DAWNBRINGER},
    {2275.80f, -5322.51f, 88.62f, 1.68f, NPC_RAYNE},
    {2282.47f, -5319.84f, 88.83f, 1.74f, NPC_RIMBLAT_EARTHSHATTER}
};

// Tirion is spawned at the edge of the battle and runs toward the chapel
// When he reach the chapel he cast some powerfull light spell and the battle ends
static sSpawnLocation aEventLocations[] =
{
    {2165.711f, -5266.124f, 95.50f, 0.13f, 0},       // 0 Tirion spawn location
    {2281.390f, -5299.98f,  85.07f, 1.61f, 0},       // 1 Tirion move location
    {2289.259f, -5280.350f, 82.11f, 0.0f,  0},       // 2 Koltira chapel loc
    {2273.289f, -5273.675f, 81.70f, 0.0f,  0},       // 3 Thassarian chapel loc
    {2280.159f, -5263.561f, 81.15f, 4.70f, 0},       // 4 Alexandros summon location
    {2279.927f, -5265.84f,  81.39f, 0.0f,  0},       // 5 Alexandros move loc
    {2280.538f, -5280.103f, 82.41f, 1.60f, 0},       // 6 Young Darion spawn
    {2279.895f, -5269.334f, 81.73f, 0.0f,  0},       // 7 Young Darion move
    {2280.304f, -5257.205f, 80.09f, 4.62f, 0},       // 8 Lich King spawn
    {2281.523f, -5261.058f, 80.87f, 0.0f,  0},       // 9 Lich King move
    {2273.071f, -5293.428f, 83.06f, 0.0f,  0},       // 10 Tirion final point
};

#endif
