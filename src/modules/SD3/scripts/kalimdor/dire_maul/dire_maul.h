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

#ifndef DEF_DM_H
#define DEF_DM_H

enum
{
    MAX_ENCOUNTER               = 16,
    MAX_GENERATORS              = 5,

    // East
    TYPE_ALZZIN                 = 0,                        // Do not change - Handled with Acid
    TYPE_ZEVRIM                 = 1,
    TYPE_IRONBARK               = 2,

    // West
    TYPE_WARPWOOD               = 3,
    TYPE_IMMOLTHAR              = 4,
    TYPE_PRINCE                 = 5,
    TYPE_PYLON_1                = 6,
    TYPE_PYLON_2                = TYPE_PYLON_1 + 1,
    TYPE_PYLON_3                = TYPE_PYLON_1 + 2,
    TYPE_PYLON_4                = TYPE_PYLON_1 + 3,
    TYPE_PYLON_5                = TYPE_PYLON_1 + 4,

    // North
    TYPE_KING_GORDOK            = 11,
    TYPE_MOLDAR                 = 12,
    TYPE_FENGUS                 = 13,
    TYPE_SLIPKIK                = 14,
    TYPE_KROMCRUSH              = 15,

    // East
    GO_CRUMBLE_WALL             = 177220,
    GO_CORRUPT_VINE             = 179502,
    GO_FELVINE_SHARD            = 179559,
    GO_CONSERVATORY_DOOR        = 176907,                   // Opened by Ironbark the redeemed

    NPC_ZEVRIM_THORNHOOF        = 11490,
    NPC_OLD_IRONBARK            = 11491,
    NPC_IRONBARK_REDEEMED       = 14241,

    // West
    NPC_TENDRIS_WARPWOOD        = 11489,
    NPC_PRINCE_TORTHELDRIN      = 11486,
    NPC_IMMOLTHAR               = 11496,
    NPC_ARCANE_ABERRATION       = 11480,
    NPC_MANA_REMNANT            = 11483,
    NPC_HIGHBORNE_SUMMONER      = 11466,

    GO_PRINCES_CHEST            = 179545,
    GO_PRINCES_CHEST_AURA       = 179563,
    GO_CRYSTAL_GENERATOR_1      = 177259,
    GO_CRYSTAL_GENERATOR_2      = 177257,
    GO_CRYSTAL_GENERATOR_3      = 177258,
    GO_CRYSTAL_GENERATOR_4      = 179504,
    GO_CRYSTAL_GENERATOR_5      = 179505,
    GO_FORCEFIELD               = 179503,
    GO_WARPWOOD_DOOR            = 177221,
    GO_WEST_LIBRARY_DOOR        = 179550,

    // North
    NPC_GUARD_MOLDAR            = 14326,
    NPC_STOMPER_KREEG           = 14322,
    NPC_GUARD_FENGUS            = 14321,
    NPC_GUARD_SLIPKIK           = 14323,
    NPC_CAPTAIN_KROMCRUSH       = 14325,
    NPC_CHORUSH                 = 14324,
    NPC_KING_GORDOK             = 11501,
    NPC_MIZZLE_THE_CRAFTY       = 14353,

    GO_KNOTS_CACHE              = 179501,
    GO_KNOTS_BALL_AND_CHAIN     = 179511,
    GO_GORDOK_TRIBUTE           = 179564,
    GO_NORTH_LIBRARY_DOOR       = 179549,

    SAY_FREE_IMMOLTHAR          = -1429000,
    SAY_KILL_IMMOLTHAR          = -1429001,
    SAY_IRONBARK_REDEEM         = -1429002,
    SAY_CHORUSH_KING_DEAD       = -1429003,
    SAY_SLIPKIK_TRAP            = -1429004,

    SPELL_ICE_BLOCK             = 22856,

    FACTION_HOSTILE             = 14,
    FACTION_FRIENDLY        = 35,
    SPELL_KING_OF_GORDOK        = 22799,
};

static const float afMizzleSpawnLoc[4] = { 683.296f, 484.384f, 29.544f, 0.0174f };

class instance_dire_maul : public ScriptedInstance
{
public:
    instance_dire_maul(Map* pMap);
    ~instance_dire_maul() {}

    void Initialize() override;

    void OnPlayerEnter(Player* pPlayer) override;

    void OnCreatureCreate(Creature* pCreature) override;
    void OnObjectCreate(GameObject* pGo) override;

    void SetData(uint32 uiType, uint32 uiData) override;
    uint32 GetData(uint32 uiType) const override;

    void OnCreatureEnterCombat(Creature* pCreature) override;
    void OnCreatureDeath(Creature* pCreature) override;

    const char* Save() const override { return m_strInstData.c_str(); }
    void Load(const char* chrIn) override;

    bool CheckConditionCriteriaMeet(Player const* pPlayer, uint32 uiInstanceConditionId, WorldObject const* pConditionSource, uint32 conditionSourceType) const override;

    void Update(uint32 uiDiff) override;

protected:
    bool CheckAllGeneratorsDestroyed();
    void ProcessForceFieldOpening();
    void SortPylonGuards();
    void PylonGuardJustDied(Creature* pCreature);

    uint32 m_auiEncounter[MAX_ENCOUNTER];
    std::string m_strInstData;

    // East
    bool m_bWallDestroyed;
    GuidList m_lFelvineShardGUIDs;

    // West
    ObjectGuid m_aCrystalGeneratorGuid[MAX_GENERATORS];

    GuidList m_luiHighborneSummonerGUIDs;
    GuidList m_lGeneratorGuardGUIDs;
    std::set<uint32> m_sSortedGeneratorGuards[MAX_GENERATORS];

    // North
    bool m_bDoNorthBeforeWest;
};
#endif
