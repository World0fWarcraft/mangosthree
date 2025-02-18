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

/* ScriptData
SDName: Instance_Naxxramas
SD%Complete: 90%
SDComment:
SDCategory: Naxxramas
EndScriptData */

#include "precompiled.h"
#include "naxxramas.h"
#include <unordered_map>

static const DialogueEntry aNaxxDialogue[] =
{
    {NPC_KELTHUZAD,         0,                  10000},
    {SAY_SAPP_DIALOG1,      NPC_KELTHUZAD,      8000},
    {SAY_SAPP_DIALOG2_LICH, NPC_THE_LICHKING,   14000},
    {SAY_SAPP_DIALOG3,      NPC_KELTHUZAD,      10000},
    {SAY_SAPP_DIALOG4_LICH, NPC_THE_LICHKING,   12000},
    {SAY_SAPP_DIALOG5,      NPC_KELTHUZAD,      0},
    {NPC_THANE,             0,                  10000},
    {SAY_KORT_TAUNT1,       NPC_THANE,          5000},
    {SAY_ZELI_TAUNT1,       NPC_ZELIEK,         6000},
    {SAY_BLAU_TAUNT1,       NPC_BLAUMEUX,       6000},
    {SAY_RIVE_TAUNT1,       NPC_RIVENDARE,      6000},
    {SAY_BLAU_TAUNT2,       NPC_BLAUMEUX,       6000},
    {SAY_ZELI_TAUNT2,       NPC_ZELIEK,         5000},
    {SAY_KORT_TAUNT2,       NPC_THANE,          7000},
    {SAY_RIVE_TAUNT2,       NPC_RIVENDARE,      0},
    {0, 0, 0}
};

struct GothTrigger
{
    bool bIsRightSide;
    bool bIsAnchorHigh;
};

static const float aSapphPositions[4] = { 3521.48f, -5234.87f, 137.626f, 4.53329f };

struct is_naxxramas : public InstanceScript
{
    is_naxxramas() : InstanceScript("instance_naxxramas") {}

    class  instance_naxxramas : public ScriptedInstance
    {
    public:
        instance_naxxramas(Map* pMap) : ScriptedInstance(pMap),
            m_fChamberCenterX(0.0f),
            m_fChamberCenterY(0.0f),
            m_fChamberCenterZ(0.0f),
            m_uiSapphSpawnTimer(0),
            m_uiTauntTimer(0),
            m_uiHorsemenAchievTimer(0),
            m_uiHorseMenKilled(0),
            m_dialogueHelper(aNaxxDialogue),
            m_uiLivingPoisonTimer(5000)
        {
            Initialize();
        }

        ~instance_naxxramas()
        {
            if (!m_mGothTriggerMap.empty()) //see SetGothTriggers
            {
                for (std::unordered_map<ObjectGuid, GothTrigger>::iterator it = m_mGothTriggerMap.begin(); it != m_mGothTriggerMap.end(); ++it)
                {
                    delete &it->second;
                }
            }
        }

        void Initialize() override
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

            for (uint8 i = 0; i < MAX_SPECIAL_ACHIEV_CRITS; ++i)
            {
                m_abAchievCriteria[i] = false;
            }

            m_dialogueHelper.InitializeDialogueHelper(this, true);
        }

        bool IsEncounterInProgress() const override
        {
            for (uint8 i = 0; i <= TYPE_KELTHUZAD; ++i)
            {
                if (m_auiEncounter[i] == IN_PROGRESS)
                {
                    return true;
                }
            }

            // Some Encounters use SPECIAL while in progress
            if (m_auiEncounter[TYPE_GOTHIK] == SPECIAL)
            {
                return true;
            }

            return false;
        }

        void OnPlayerEnter(Player* pPlayer) override
        {
            // Function only used to summon Sapphiron in case of server reload
            if (GetData(TYPE_SAPPHIRON) != SPECIAL)
            {
                return;
            }

            // Check if already summoned
            if (GetSingleCreatureFromStorage(NPC_SAPPHIRON, true))
            {
                return;
            }

            pPlayer->SummonCreature(NPC_SAPPHIRON, aSapphPositions[0], aSapphPositions[1], aSapphPositions[2], aSapphPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);
        }

        void OnCreatureCreate(Creature* pCreature) override
        {
            switch (pCreature->GetEntry())
            {
            case NPC_HEIGAN_THE_UNCLEAN:
            case NPC_ANUB_REKHAN:
            case NPC_FAERLINA:
            case NPC_THADDIUS:
            case NPC_STALAGG:
            case NPC_FEUGEN:
            case NPC_ZELIEK:
            case NPC_THANE:
            case NPC_BLAUMEUX:
            case NPC_RIVENDARE:
            case NPC_GOTHIK:
            case NPC_SAPPHIRON:
            case NPC_KELTHUZAD:
            case NPC_THE_LICHKING:
                m_mNpcEntryGuidStore[pCreature->GetEntry()] = pCreature->GetObjectGuid();
                break;

            case NPC_SUB_BOSS_TRIGGER:  m_lGothTriggerList.push_back(pCreature->GetObjectGuid()); break;
            case NPC_TESLA_COIL:        m_lThadTeslaCoilList.push_back(pCreature->GetObjectGuid()); break;
            }
        }

        void OnObjectCreate(GameObject* pGo) override
        {
            switch (pGo->GetEntry())
            {
                // Arachnid Quarter
            case GO_ARAC_ANUB_DOOR:
                break;
            case GO_ARAC_ANUB_GATE:
                if (m_auiEncounter[TYPE_ANUB_REKHAN] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_ARAC_FAER_WEB:
                break;
            case GO_ARAC_FAER_DOOR:
                if (m_auiEncounter[TYPE_FAERLINA] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_ARAC_MAEX_INNER_DOOR:
                break;
            case GO_ARAC_MAEX_OUTER_DOOR:
                if (m_auiEncounter[TYPE_FAERLINA] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;

                // Plague Quarter
            case GO_PLAG_NOTH_ENTRY_DOOR:
                break;
            case GO_PLAG_NOTH_EXIT_DOOR:
                if (m_auiEncounter[TYPE_NOTH] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_PLAG_HEIG_ENTRY_DOOR:
                if (m_auiEncounter[TYPE_NOTH] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_PLAG_HEIG_EXIT_DOOR:
                if (m_auiEncounter[TYPE_HEIGAN] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_PLAG_LOAT_DOOR:
                break;

                // Military Quarter
            case GO_MILI_GOTH_ENTRY_GATE:
                break;
            case GO_MILI_GOTH_EXIT_GATE:
                if (m_auiEncounter[TYPE_GOTHIK] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_MILI_GOTH_COMBAT_GATE:
                break;
            case GO_MILI_HORSEMEN_DOOR:
                if (m_auiEncounter[TYPE_GOTHIK] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_CHEST_HORSEMEN_NORM:
            case GO_CHEST_HORSEMEN_HERO:
                break;

                // Construct Quarter
            case GO_CONS_PATH_EXIT_DOOR:
                if (m_auiEncounter[TYPE_PATCHWERK] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_CONS_GLUT_EXIT_DOOR:
                if (m_auiEncounter[TYPE_GLUTH] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_CONS_THAD_DOOR:
                if (m_auiEncounter[TYPE_GLUTH] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_CONS_NOX_TESLA_FEUGEN:
                if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                {
                    pGo->SetGoState(GO_STATE_READY);
                }
                break;
            case GO_CONS_NOX_TESLA_STALAGG:
                if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                {
                    pGo->SetGoState(GO_STATE_READY);
                }
                break;

                // Frostwyrm Lair
            case GO_KELTHUZAD_WATERFALL_DOOR:
                if (m_auiEncounter[TYPE_SAPPHIRON] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_KELTHUZAD_EXIT_DOOR:
                break;

                // Eyes
            case GO_ARAC_EYE_RAMP:
            case GO_ARAC_EYE_BOSS:
                if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_PLAG_EYE_RAMP:
            case GO_PLAG_EYE_BOSS:
                if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_MILI_EYE_RAMP:
            case GO_MILI_EYE_BOSS:
                if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;
            case GO_CONS_EYE_RAMP:
            case GO_CONS_EYE_BOSS:
                if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                {
                    pGo->SetGoState(GO_STATE_ACTIVE);
                }
                break;

                // Portals
            case GO_ARAC_PORTAL:
                if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                {
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                }
                break;
            case GO_PLAG_PORTAL:
                if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                {
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                }
                break;
            case GO_MILI_PORTAL:
                if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                {
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                }
                break;
            case GO_CONS_PORTAL:
                if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                {
                    pGo->RemoveFlag(GAMEOBJECT_FLAGS, GO_FLAG_NO_INTERACT);
                }
                break;

            default:
                // Heigan Traps - many different entries which are only required for sorting
                if (pGo->GetGoType() == GAMEOBJECT_TYPE_TRAP)
                {
                    uint32 uiGoEntry = pGo->GetEntry();

                    if ((uiGoEntry >= 181517 && uiGoEntry <= 181524) || uiGoEntry == 181678)
                    {
                        m_alHeiganTrapGuids[0].push_back(pGo->GetObjectGuid());
                    }
                    else if ((uiGoEntry >= 181510 && uiGoEntry <= 181516) || (uiGoEntry >= 181525 && uiGoEntry <= 181531) || uiGoEntry == 181533 || uiGoEntry == 181676)
                    {
                        m_alHeiganTrapGuids[1].push_back(pGo->GetObjectGuid());
                    }
                    else if ((uiGoEntry >= 181534 && uiGoEntry <= 181544) || uiGoEntry == 181532 || uiGoEntry == 181677)
                    {
                        m_alHeiganTrapGuids[2].push_back(pGo->GetObjectGuid());
                    }
                    else if ((uiGoEntry >= 181545 && uiGoEntry <= 181552) || uiGoEntry == 181695)
                    {
                        m_alHeiganTrapGuids[3].push_back(pGo->GetObjectGuid());
                    }
                }

                return;
            }
            m_mGoEntryGuidStore[pGo->GetEntry()] = pGo->GetObjectGuid();
        }

        void OnPlayerDeath(Player* pPlayer) override
        {
            if (IsEncounterInProgress())
            {
                SetData(TYPE_UNDYING_FAILED, DONE);
            }

            if (GetData(TYPE_HEIGAN) == IN_PROGRESS)
            {
                SetSpecialAchievementCriteria(TYPE_ACHIEV_SAFETY_DANCE, false);
            }
        }

        void OnCreatureDeath(Creature* pCreature) override
        {
            if (pCreature->GetEntry() == NPC_MR_BIGGLESWORTH && m_auiEncounter[TYPE_KELTHUZAD] != DONE)
            {
                DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_CAT_DIED, NPC_KELTHUZAD);
            }
        }

        void SetData(uint32 uiType, uint32 uiData) override
        {
            switch (uiType)
            {
            case TYPE_ANUB_REKHAN:
                m_auiEncounter[uiType] = uiData;
                DoUseDoorOrButton(GO_ARAC_ANUB_DOOR);
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_ARAC_ANUB_GATE);
                    DoStartTimedAchievement(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, ACHIEV_START_MAEXXNA_ID);
                }
                break;
            case TYPE_FAERLINA:
                DoUseDoorOrButton(GO_ARAC_FAER_WEB);
                if (uiData == IN_PROGRESS)
                {
                    SetSpecialAchievementCriteria(TYPE_ACHIEV_KNOCK_YOU_OUT, true);
                }
                else if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_ARAC_FAER_DOOR);
                    DoUseDoorOrButton(GO_ARAC_MAEX_OUTER_DOOR);
                }
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_MAEXXNA:
                m_auiEncounter[uiType] = uiData;
                DoUseDoorOrButton(GO_ARAC_MAEX_INNER_DOOR, uiData);
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_ARAC_EYE_RAMP);
                    DoUseDoorOrButton(GO_ARAC_EYE_BOSS);
                    DoRespawnGameObject(GO_ARAC_PORTAL, 30 * MINUTE);
                    DoToggleGameObjectFlags(GO_ARAC_PORTAL, GO_FLAG_NO_INTERACT, false);
                    m_uiTauntTimer = 5000;
                }
                break;
            case TYPE_NOTH:
                m_auiEncounter[uiType] = uiData;
                DoUseDoorOrButton(GO_PLAG_NOTH_ENTRY_DOOR);
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_PLAG_NOTH_EXIT_DOOR);
                    DoUseDoorOrButton(GO_PLAG_HEIG_ENTRY_DOOR);
                }
                break;
            case TYPE_HEIGAN:
                m_auiEncounter[uiType] = uiData;
                DoUseDoorOrButton(GO_PLAG_HEIG_ENTRY_DOOR);
                if (uiData == IN_PROGRESS)
                {
                    SetSpecialAchievementCriteria(TYPE_ACHIEV_SAFETY_DANCE, true);
                }
                else if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_PLAG_HEIG_EXIT_DOOR);
                }
                break;
            case TYPE_LOATHEB:
                m_auiEncounter[uiType] = uiData;
                DoUseDoorOrButton(GO_PLAG_LOAT_DOOR);
                if (uiData == IN_PROGRESS)
                {
                    SetSpecialAchievementCriteria(TYPE_ACHIEV_SPORE_LOSER, true);
                }
                else if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_PLAG_EYE_RAMP);
                    DoUseDoorOrButton(GO_PLAG_EYE_BOSS);
                    DoRespawnGameObject(GO_PLAG_PORTAL, 30 * MINUTE);
                    DoToggleGameObjectFlags(GO_PLAG_PORTAL, GO_FLAG_NO_INTERACT, false);
                    m_uiTauntTimer = 5000;
                }
                break;
            case TYPE_RAZUVIOUS:
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_GOTHIK:
                switch (uiData)
                {
                case IN_PROGRESS:
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    SetGothTriggers();
                    break;
                case SPECIAL:
                    DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    break;
                case FAIL:
                    if (m_auiEncounter[uiType] == IN_PROGRESS)
                    {
                        DoUseDoorOrButton(GO_MILI_GOTH_COMBAT_GATE);
                    }

                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    break;
                case DONE:
                    DoUseDoorOrButton(GO_MILI_GOTH_ENTRY_GATE);
                    DoUseDoorOrButton(GO_MILI_GOTH_EXIT_GATE);
                    DoUseDoorOrButton(GO_MILI_HORSEMEN_DOOR);

                    m_dialogueHelper.StartNextDialogueText(NPC_THANE);
                    break;
                }
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_FOUR_HORSEMEN:
                // Skip if already set
                if (m_auiEncounter[uiType] == uiData)
                {
                    return;
                }

                if (uiData == SPECIAL)
                {
                    // Start the achiev countdown
                    if (!m_uiHorseMenKilled)
                    {
                        m_uiHorsemenAchievTimer = 15000;
                    }

                    ++m_uiHorseMenKilled;

                    if (m_uiHorseMenKilled == 4)
                    {
                        SetData(TYPE_FOUR_HORSEMEN, DONE);
                    }

                    // Don't store special data
                    return;
                }
                else if (uiData == FAIL)
                {
                    m_uiHorseMenKilled = 0;
                }
                else if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_MILI_EYE_RAMP);
                    DoUseDoorOrButton(GO_MILI_EYE_BOSS);
                    DoRespawnGameObject(GO_MILI_PORTAL, 30 * MINUTE);
                    DoToggleGameObjectFlags(GO_MILI_PORTAL, GO_FLAG_NO_INTERACT, false);
                    DoRespawnGameObject(instance->IsRegularDifficulty() ? GO_CHEST_HORSEMEN_NORM : GO_CHEST_HORSEMEN_HERO, 30 * MINUTE);
                    m_uiTauntTimer = 5000;
                }
                DoUseDoorOrButton(GO_MILI_HORSEMEN_DOOR);
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_PATCHWERK:
                m_auiEncounter[uiType] = uiData;
                if (uiData == IN_PROGRESS)
                {
                    DoStartTimedAchievement(ACHIEVEMENT_CRITERIA_TYPE_KILL_CREATURE, ACHIEV_START_PATCHWERK_ID);
                }
                else if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_CONS_PATH_EXIT_DOOR);
                }
                break;
            case TYPE_GROBBULUS:
                m_auiEncounter[uiType] = uiData;
                break;
            case TYPE_GLUTH:
                m_auiEncounter[uiType] = uiData;
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_CONS_GLUT_EXIT_DOOR);
                    DoUseDoorOrButton(GO_CONS_THAD_DOOR);
                }
                break;
            case TYPE_THADDIUS:
                // Only process real changes here
                if (m_auiEncounter[uiType] == uiData)
                {
                    return;
                }

                m_auiEncounter[uiType] = uiData;
                if (uiData != SPECIAL)
                {
                    DoUseDoorOrButton(GO_CONS_THAD_DOOR, uiData);
                }
                // Uncomment when this achievement is implemented
                // if (uiData == IN_PROGRESS)
                //    SetSpecialAchievementCriteria(TYPE_ACHIEV_SHOCKING, true);
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_CONS_EYE_RAMP);
                    DoUseDoorOrButton(GO_CONS_EYE_BOSS);
                    DoRespawnGameObject(GO_CONS_PORTAL, 30 * MINUTE);
                    DoToggleGameObjectFlags(GO_CONS_PORTAL, GO_FLAG_NO_INTERACT, false);
                    m_uiTauntTimer = 5000;
                }
                break;
            case TYPE_SAPPHIRON:
                m_auiEncounter[uiType] = uiData;
                // Uncomment when achiev check implemented
                // if (uiData == IN_PROGRESS)
                //    SetSpecialAchievementCriteria(TYPE_ACHIEV_HUNDRED_CLUB, true);
                if (uiData == DONE)
                {
                    DoUseDoorOrButton(GO_KELTHUZAD_WATERFALL_DOOR);
                    m_dialogueHelper.StartNextDialogueText(NPC_KELTHUZAD);
                }
                // Start Sapph summoning process
                if (uiData == SPECIAL)
                {
                    m_uiSapphSpawnTimer = 22000;
                }
                break;
            case TYPE_KELTHUZAD:
                m_auiEncounter[uiType] = uiData;
                DoUseDoorOrButton(GO_KELTHUZAD_EXIT_DOOR);
                if (uiData == IN_PROGRESS)
                {
                    SetSpecialAchievementCriteria(TYPE_ACHIEV_GET_ENOUGH, false);
                }
                break;
            case TYPE_UNDYING_FAILED:
                m_auiEncounter[uiType] = uiData;
                break;
            //case TYPE_DO_SET_CHAMBER_CENTER:
            //    if (AreaTriggerEntry const* at = sAreaTriggerStore.LookupEntry(uiData))
            //        SetChamberCenterCoords(at->x, at->y, at->z);
            //    return;
            case TYPE_ACHIEV_GET_ENOUGH:
            case TYPE_ACHIEV_SPORE_LOSER:
            case TYPE_ACHIEV_SHOCKING:
            case TYPE_ACHIEV_HUNDRED_CLUB:
            case TYPE_ACHIEV_KNOCK_YOU_OUT:
            case TYPE_ACHIEV_SAFETY_DANCE:
                SetSpecialAchievementCriteria(uiType - TYPE_ACHIEV_SAFETY_DANCE, bool(uiData));
                return;
            case TYPE_DO_GOTH_SUMMON:
                SummonAdds(uiData);
                return;
            case TYPE_DO_HEIGAN_TRAPS:
                DoTriggerHeiganTraps(uiData);
                return;
            case TYPE_DO_THAD_OVERLOAD:
                for (GuidList::const_iterator itr = m_lThadTeslaCoilList.begin(); itr != m_lThadTeslaCoilList.end(); ++itr)
                {
                    if (Creature* pTesla = instance->GetCreature(*itr))
                    {
                        if (CreatureAI* pTeslaAI = pTesla->AI())
                        {
                            pTeslaAI->ReceiveAIEvent(AI_EVENT_CUSTOM_A, (Creature*)nullptr, (Unit*)nullptr, 0);
                        }
                    }
                }
                return;
            case TYPE_DO_THAD_CHAIN:
                for (GuidList::const_iterator itr = m_lThadTeslaCoilList.begin(); itr != m_lThadTeslaCoilList.end(); ++itr)
                {
                    if (Creature* pTesla = instance->GetCreature(*itr))
                    {
                        if (CreatureAI* pTeslaAI = pTesla->AI())
                        {
                            pTeslaAI->ReceiveAIEvent(AI_EVENT_CUSTOM_B, (Creature*)nullptr, (Unit*)nullptr, uiData);
                        }
                    }
                }
                return;
            }

            if (uiData == DONE || (uiData == SPECIAL && uiType == TYPE_SAPPHIRON))
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << m_auiEncounter[0] << " " << m_auiEncounter[1] << " " << m_auiEncounter[2] << " "
                    << m_auiEncounter[3] << " " << m_auiEncounter[4] << " " << m_auiEncounter[5] << " "
                    << m_auiEncounter[6] << " " << m_auiEncounter[7] << " " << m_auiEncounter[8] << " "
                    << m_auiEncounter[9] << " " << m_auiEncounter[10] << " " << m_auiEncounter[11] << " "
                    << m_auiEncounter[12] << " " << m_auiEncounter[13] << " " << m_auiEncounter[14] << " " << m_auiEncounter[15];

                m_strInstData = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        uint32 GetData(uint32 uiType) const override
        {
            if (uiType < MAX_ENCOUNTER)
            {
                return m_auiEncounter[uiType];
            }

            return 0;
        }

        void SetData64(uint32 uiType, uint64 uiData) override
        {
            if (uiType == DATA64_GOTH_LEFT_ANCHOR || uiType == DATA64_GOTH_RIGHT_ANCHOR)
            {
                m_creatureGUID = ObjectGuid(uiData);
            }
        }

        uint64 GetData64(uint32 uiType) const override
        {
            switch (uiType)
            {
            case DATA64_GOTH_LEFT_ANCHOR:
            case DATA64_GOTH_RIGHT_ANCHOR:
                return GetClosestAnchorForGoth(uiType == DATA64_GOTH_RIGHT_ANCHOR);
            case DATA64_GOTH_RANDOM_LEFT:
            case DATA64_GOTH_RANDOM_RIGHT:
                return GetGothSummonPoint(uiType == DATA64_GOTH_RANDOM_RIGHT);
            default:
                break;
            }
            return 0;
        }

        bool CheckAchievementCriteriaMeet(uint32 uiCriteriaId, Player const* pSource, Unit const* pTarget, uint32 uiMiscValue1 /* = 0*/) const override
        {
            switch (uiCriteriaId)
            {
            case ACHIEV_CRIT_SAFETY_DANCE_N:
            case ACHIEV_CRIT_SAFETY_DANCE_H:
                return m_abAchievCriteria[TYPE_ACHIEV_SAFETY_DANCE];
            case ACHIEV_CRIT_KNOCK_YOU_OUT_N:
            case ACHIEV_CRIT_KNOCK_YOU_OUT_H:
                return m_abAchievCriteria[TYPE_ACHIEV_KNOCK_YOU_OUT];
            case ACHIEV_CRIT_HUNDRED_CLUB_N:
            case ACHIEV_CRIT_HUNDRED_CLUB_H:
                return m_abAchievCriteria[TYPE_ACHIEV_HUNDRED_CLUB];
            case ACHIEV_CRIT_SHOCKING_N:
            case ACHIEV_CRIT_SHOCKING_H:
                return m_abAchievCriteria[TYPE_ACHIEV_SHOCKING];
            case ACHIEV_CRIT_SPORE_LOSER_N:
            case ACHIEV_CRIT_SPORE_LOSER_H:
                return m_abAchievCriteria[TYPE_ACHIEV_SPORE_LOSER];
            case ACHIEV_CRIT_GET_ENOUGH_N:
            case ACHIEV_CRIT_GET_ENOUGH_H:
                return m_abAchievCriteria[TYPE_ACHIEV_GET_ENOUGH];
            case ACHIEV_CRIT_TOGETHER_N:
            case ACHIEV_CRIT_TOGETHER_H:
                return m_uiHorsemenAchievTimer > 0;
                // 'The Immortal'(25m) or 'Undying'(10m) - (achievs 2186, 2187)
            case ACHIEV_CRIT_IMMORTAL_KEL:
            case ACHIEV_CRIT_IMMOORTAL_LOA:
            case ACHIEV_CRIT_IMMOORTAL_THAD:
            case ACHIEV_CRIT_IMMOORTAL_MAEX:
            case ACHIEV_CRIT_IMMOORTAL_HORSE:
            case ACHIEV_CRIT_UNDYING_KEL:
            case ACHIEV_CRIT_UNDYING_HORSE:
            case ACHIEV_CRIT_UNDYING_MAEX:
            case ACHIEV_CRIT_UNDYING_LOA:
            case ACHIEV_CRIT_UNDYING_THAD:
            {
                                             // First, check if all bosses are killed (except the last encounter)
                                             uint8 uiEncounterDone = 0;
                                             for (uint8 i = 0; i < TYPE_KELTHUZAD; ++i)
                                             if (m_auiEncounter[i] == DONE)
                                             {
                                                 ++uiEncounterDone;
                                             }

                                             return uiEncounterDone >= 14 && GetData(TYPE_UNDYING_FAILED) != DONE;
            }
            default:
                return false;
            }
        }

        const char* Save() const override { return m_strInstData.c_str(); }
        void Load(const char* chrIn) override
        {
            if (!chrIn)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(chrIn);

            std::istringstream loadStream(chrIn);
            loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3]
                >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7]
                >> m_auiEncounter[8] >> m_auiEncounter[9] >> m_auiEncounter[10] >> m_auiEncounter[11]
                >> m_auiEncounter[12] >> m_auiEncounter[13] >> m_auiEncounter[14] >> m_auiEncounter[15];

            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
            {
                if (m_auiEncounter[i] == IN_PROGRESS)
                {
                    m_auiEncounter[i] = NOT_STARTED;
                }
            }

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void Update(uint32 uiDiff) override
        {
            // Handle the continuous spawning of Living Poison blobs in Patchwerk corridor
            if (m_uiLivingPoisonTimer)
            {
                if (m_uiLivingPoisonTimer <= uiDiff)
                {
                    if (Player* pPlayer = GetPlayerInMap())
                    {
                        // Spawn 3 living poisons every 5 secs and make them cross the corridor and then despawn, for ever and ever
                        for (uint8 i = 0; i < 3; i++)
                            if (Creature* pPoison = pPlayer->SummonCreature(NPC_LIVING_POISON, aLivingPoisonPositions[i].m_fX, aLivingPoisonPositions[i].m_fY, aLivingPoisonPositions[i].m_fZ, aLivingPoisonPositions[i].m_fO, TEMPSPAWN_DEAD_DESPAWN, 0))
                            {
                                pPoison->GetMotionMaster()->MovePoint(0, aLivingPoisonPositions[i + 3].m_fX, aLivingPoisonPositions[i + 3].m_fY, aLivingPoisonPositions[i + 3].m_fZ);
                                pPoison->ForcedDespawn(15000);
                            }
                    }
                    m_uiLivingPoisonTimer = 5000;
                }
                else
                {
                    m_uiLivingPoisonTimer -= uiDiff;
                }
            }

            if (m_uiTauntTimer)
            {
                if (m_uiTauntTimer <= uiDiff)
                {
                    DoTaunt();
                    m_uiTauntTimer = 0;
                }
                else
                {
                    m_uiTauntTimer -= uiDiff;
                }
            }

            if (m_uiHorsemenAchievTimer)
            {
                if (m_uiHorsemenAchievTimer <= uiDiff)
                {
                    m_uiHorsemenAchievTimer = 0;
                }
                else
                {
                    m_uiHorsemenAchievTimer -= uiDiff;
                }
            }

            if (m_uiSapphSpawnTimer)
            {
                if (m_uiSapphSpawnTimer <= uiDiff)
                {
                    if (Player* pPlayer = GetPlayerInMap())
                    {
                        pPlayer->SummonCreature(NPC_SAPPHIRON, aSapphPositions[0], aSapphPositions[1], aSapphPositions[2], aSapphPositions[3], TEMPSPAWN_DEAD_DESPAWN, 0);
                    }

                    m_uiSapphSpawnTimer = 0;
                }
                else
                {
                    m_uiSapphSpawnTimer -= uiDiff;
                }
            }

            m_dialogueHelper.DialogueUpdate(uiDiff);
        }

        void DoTaunt()
        {
            if (m_auiEncounter[TYPE_KELTHUZAD] != DONE)
            {
                uint8 uiWingsCleared = 0;

                if (m_auiEncounter[TYPE_MAEXXNA] == DONE)
                {
                    ++uiWingsCleared;
                }

                if (m_auiEncounter[TYPE_LOATHEB] == DONE)
                {
                    ++uiWingsCleared;
                }

                if (m_auiEncounter[TYPE_FOUR_HORSEMEN] == DONE)
                {
                    ++uiWingsCleared;
                }

                if (m_auiEncounter[TYPE_THADDIUS] == DONE)
                {
                    ++uiWingsCleared;
                }

                switch (uiWingsCleared)
                {
                case 1: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT1, NPC_KELTHUZAD); break;
                case 2: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT2, NPC_KELTHUZAD); break;
                case 3: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT3, NPC_KELTHUZAD); break;
                case 4: DoOrSimulateScriptTextForThisInstance(SAY_KELTHUZAD_TAUNT4, NPC_KELTHUZAD); break;
                }
            }
        }

    private:
        // Heigan
        void DoTriggerHeiganTraps(uint32 uiAreaIndex)
        {
            Creature* pHeigan = GetSingleCreatureFromStorage(NPC_HEIGAN_THE_UNCLEAN);

            if (!pHeigan || uiAreaIndex >= MAX_HEIGAN_TRAP_AREAS)
            {
                return;
            }

            for (GuidList::const_iterator itr = m_alHeiganTrapGuids[uiAreaIndex].begin(); itr != m_alHeiganTrapGuids[uiAreaIndex].end(); ++itr)
            {
                if (GameObject* pTrap = instance->GetGameObject(*itr))
                {
                    pTrap->Use(pHeigan);
                }
            }
        }

        // goth
        uint64 GetClosestAnchorForGoth(bool bRightSide) const
        {
            std::list<Creature* > lList;

            for (std::unordered_map<ObjectGuid, GothTrigger>::const_iterator itr = m_mGothTriggerMap.begin(); itr != m_mGothTriggerMap.end(); ++itr)
            {
                if (!itr->second.bIsAnchorHigh)
                {
                    continue;
                }

                if (itr->second.bIsRightSide != bRightSide)
                {
                    continue;
                }

                if (Creature* pCreature = instance->GetCreature(itr->first))
                {
                    lList.push_back(pCreature);
                }
            }

            if (!lList.empty())
            {
                lList.sort(ObjectDistanceOrder(instance->GetCreature(m_creatureGUID)));
                return lList.front()->GetObjectGuid().GetRawValue();
            }

            return 0;
        }

        uint64 GetGothSummonPoint(bool bRightSide) const
        {
            std::list<Creature*> lList;
            GetGothSummonPointCreatures(bRightSide, lList);

            if (!lList.empty())
            {
                std::list<Creature*>::iterator itr = lList.begin();
                uint32 uiPosition = urand(0, lList.size() - 1);
                advance(itr, uiPosition);
                return (*itr)->GetObjectGuid().GetRawValue();
            }

            return 0;
        }

        void GetGothSummonPointCreatures(bool bRightSide, std::list<Creature*> &lList) const
        {
            for (std::unordered_map<ObjectGuid, GothTrigger>::const_iterator itr = m_mGothTriggerMap.begin(); itr != m_mGothTriggerMap.end(); ++itr)
            {
                if (itr->second.bIsAnchorHigh)
                {
                    continue;
                }

                if (itr->second.bIsRightSide != bRightSide)
                {
                    continue;
                }

                if (Creature* pCreature = instance->GetCreature(itr->first))
                {
                    lList.push_back(pCreature);
                }
            }
        }

        void SetGothTriggers()
        {
            Creature* pGoth = GetSingleCreatureFromStorage(NPC_GOTHIK);
            GameObject* pCombatGate = GetSingleGameObjectFromStorage(GO_MILI_GOTH_COMBAT_GATE);

            if (!pGoth || !pCombatGate)
            {
                return;
            }

            if (!m_mGothTriggerMap.empty())
            {
                for (GuidList::const_iterator itr = m_lGothTriggerList.begin(); itr != m_lGothTriggerList.end(); ++itr)
                {
                    if (Creature* pTrigger = instance->GetCreature(*itr))
                    {
                        GothTrigger pGt;
                        pGt.bIsAnchorHigh = (pTrigger->GetPositionZ() >= (pGoth->GetPositionZ() - 5.0f));
                        pGt.bIsRightSide = pCombatGate->GetPositionY() >= pTrigger->GetPositionY();

                        m_mGothTriggerMap[pTrigger->GetObjectGuid()] = pGt;
                    }
                }
            }

            //PrepareSummonPlaces()
            std::list<Creature*> lSummonList;
            GetGothSummonPointCreatures(true, lSummonList);

            if (lSummonList.empty())
            {
                return;
            }

            // Trainees and Rider
            uint8 index = 0;
            uint8 uiTraineeCount = instance->IsRegularDifficulty() ? 2 : 3;
            lSummonList.sort(ObjectDistanceOrder(pGoth));
            for (std::list<Creature*>::iterator itr = lSummonList.begin(); itr != lSummonList.end(); ++itr)
            {
                if (*itr)
                {
                    if (uiTraineeCount == 0)
                    {
                        break;
                    }
                    if (index == 1)
                    {
                        m_lRiderSummonPosGuids.push_back((*itr)->GetObjectGuid());
                    }
                    else
                    {
                        m_lTraineeSummonPosGuids.push_back((*itr)->GetObjectGuid());
                        --uiTraineeCount;
                    }
                    index++;
                }
            }

            // DeathKnights
            uint8 uiDeathKnightCount = instance->IsRegularDifficulty() ? 1 : 2;
            lSummonList.sort(ObjectDistanceOrderReversed(pGoth));
            for (std::list<Creature*>::iterator itr = lSummonList.begin(); itr != lSummonList.end(); ++itr)
            {
                if (*itr)
                {
                    if (uiDeathKnightCount == 0)
                    {
                        break;
                    }
                    m_lDeathKnightSummonPosGuids.push_back((*itr)->GetObjectGuid());
                    --uiDeathKnightCount;
                }
            }
        }

        void SummonAdds(uint32 uiSummonEntry)
        {
            Creature* pGoth = GetSingleCreatureFromStorage(NPC_GOTHIK);
            if (!pGoth || pGoth->IsDead())
            {
                return;
            }

            GuidList* plSummonPosGuids;
            switch (uiSummonEntry)
            {
            case NPC_UNREL_TRAINEE:      plSummonPosGuids = &m_lTraineeSummonPosGuids;     break;
            case NPC_UNREL_DEATH_KNIGHT: plSummonPosGuids = &m_lDeathKnightSummonPosGuids; break;
            case NPC_UNREL_RIDER:        plSummonPosGuids = &m_lRiderSummonPosGuids;       break;
            default:
                return;
            }
            if (plSummonPosGuids->empty())
            {
                return;
            }

            for (GuidList::iterator itr = plSummonPosGuids->begin(); itr != plSummonPosGuids->end(); ++itr)
            {
                if (Creature* pPos = instance->GetCreature(*itr))
                {
                    pGoth->SummonCreature(uiSummonEntry, pPos->GetPositionX(), pPos->GetPositionY(), pPos->GetPositionZ(), pPos->GetOrientation(), TEMPSPAWN_DEAD_DESPAWN, 0);
                }
            }
        }

        void SetSpecialAchievementCriteria(uint32 uiType, bool bIsMet)
        {
            if (uiType < MAX_SPECIAL_ACHIEV_CRITS)
            {
                m_abAchievCriteria[uiType] = bIsMet;
            }
        }

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        bool m_abAchievCriteria[MAX_SPECIAL_ACHIEV_CRITS];
        std::string m_strInstData;

        GuidList m_lThadTeslaCoilList;
        GuidList m_lGothTriggerList;
        GuidList m_lTraineeSummonPosGuids;
        GuidList m_lDeathKnightSummonPosGuids;
        GuidList m_lRiderSummonPosGuids;

        std::unordered_map<ObjectGuid, GothTrigger> m_mGothTriggerMap;
        GuidList m_alHeiganTrapGuids[MAX_HEIGAN_TRAP_AREAS];
        ObjectGuid m_creatureGUID;

        float m_fChamberCenterX;
        float m_fChamberCenterY;
        float m_fChamberCenterZ;

        uint32 m_uiSapphSpawnTimer;
        uint32 m_uiTauntTimer;
        uint32 m_uiHorsemenAchievTimer;
        uint8 m_uiHorseMenKilled;
        uint32 m_uiLivingPoisonTimer;

        DialogueHelper m_dialogueHelper;
    };

    InstanceData* GetInstanceData(Map* pMap) override
    {
        return new instance_naxxramas(pMap);
    }
};

struct at_naxxramas : public AreaTriggerScript
{
    at_naxxramas() : AreaTriggerScript("at_naxxramas") {}

    bool OnTrigger(Player* pPlayer, AreaTriggerEntry const* pAt) override
    {
        if (pAt->id == AREATRIGGER_KELTHUZAD)
        {
            if (pPlayer->isGameMaster() || !pPlayer->IsAlive())
            {
                return false;
            }

            ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData();

            if (!pInstance)
            {
                return false;
            }

            if (pInstance->GetData(TYPE_KELTHUZAD) == NOT_STARTED)
            {
                if (Creature* pKelthuzad = pInstance->GetSingleCreatureFromStorage(NPC_KELTHUZAD))
                {
                    if (pKelthuzad->IsAlive())
                    {
                        pInstance->SetData(TYPE_KELTHUZAD, IN_PROGRESS);
                        pKelthuzad->SetInCombatWithZone();
                    }
                }
            }
        }

        if (pAt->id == AREATRIGGER_THADDIUS_DOOR)
        {
            if (ScriptedInstance* pInstance = (ScriptedInstance*)pPlayer->GetInstanceData())
            {
                if (pInstance->GetData(TYPE_THADDIUS) == NOT_STARTED)
                {
                    if (Creature* pThaddius = pInstance->GetSingleCreatureFromStorage(NPC_THADDIUS))
                    {
                        pInstance->SetData(TYPE_THADDIUS, SPECIAL);
                        DoScriptText(SAY_THADDIUS_GREET, pThaddius);
                    }
                }
            }
        }

        return false;
    }
};

void AddSC_instance_naxxramas()
{
    Script* s;

    s = new is_naxxramas();
    s->RegisterSelf();

    s = new at_naxxramas();
    s->RegisterSelf();

    //pNewScript = new Script;
    //pNewScript->Name = "instance_naxxramas";
    //pNewScript->GetInstanceData = &GetInstanceData_instance_naxxramas;
    //pNewScript->RegisterSelf();

    //pNewScript = new Script;
    //pNewScript->Name = "at_naxxramas";
    //pNewScript->pAreaTrigger = &AreaTrigger_at_naxxramas;
    //pNewScript->RegisterSelf();
}
