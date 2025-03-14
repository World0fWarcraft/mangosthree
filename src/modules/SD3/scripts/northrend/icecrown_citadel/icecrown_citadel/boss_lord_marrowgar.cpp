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
SDName: boss_lord_marrowgar
SD%Complete: 75%
SDComment: Bone Spike Spell requires vehicle support
SDCategory: Icecrown Citadel
EndScriptData */

#include "precompiled.h"
#include "icecrown_citadel.h"

enum
{
    SAY_AGGRO                   = -1631002,
    SAY_BONE_STORM              = -1631003,
    SAY_BONE_SPIKE_1            = -1631004,
    SAY_BONE_SPIKE_2            = -1631005,
    SAY_BONE_SPIKE_3            = -1631006,
    SAY_SLAY_1                  = -1631007,
    SAY_SLAY_2                  = -1631008,
    SAY_DEATH                   = -1631009,
    SAY_BERSERK                 = -1631010,

    // spells
    SPELL_BERSERK               = 47008,
    SPELL_BONE_SLICE            = 69055,
    SPELL_BONE_STORM            = 69076,
    SPELL_COLDFLAME             = 69140,
    SPELL_COLDFLAME_STORM       = 72705,
    SPELL_BONE_SPIKE            = 69057,
    SPELL_BONE_SPIKE_STORM      = 73142,

    // summoned spells
    SPELL_COLDFLAME_AURA        = 69145,
    SPELL_IMPALED               = 69065,

    // npcs
    NPC_BONE_SPIKE              = 38711,
    NPC_COLDFLAME               = 36672,

    // phases and max cold flame charges
    PHASE_NORMAL                = 1,
    PHASE_BONE_STORM_CHARGE     = 2,
    PHASE_BONE_STORM_CHARGING   = 3,
    PHASE_BONE_STORM_COLDFLAME  = 4,

    MAX_CHARGES_NORMAL          = 4,
    MAX_CHARGES_HEROIC          = 5,
};

struct boss_lord_marrowgar : public CreatureScript
{
    boss_lord_marrowgar() : CreatureScript("boss_lord_marrowgar") {}

    struct boss_lord_marrowgarAI : public ScriptedAI
    {
        boss_lord_marrowgarAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
            // on heroic, there is 1 more Bone Storm charge
            m_uiMaxCharges = (m_pInstance && m_pInstance->GetData(TYPE_DATA_IS_HEROIC)) ? MAX_CHARGES_HEROIC : MAX_CHARGES_NORMAL;
        }

        ScriptedInstance* m_pInstance;

        uint8 m_uiPhase;
        uint8 m_uiChargesCount;
        uint8 m_uiMaxCharges;
        uint32 m_uiBerserkTimer;
        uint32 m_uiBoneSliceTimer;
        uint32 m_uiColdflameTimer;
        uint32 m_uiBoneSpikeTimer;
        uint32 m_uiBoneStormTimer;
        uint32 m_uiBoneStormChargeTimer;
        uint32 m_uiBoneStormColdflameTimer;

        void Reset() override
        {
            SetCombatMovement(true);

            m_uiPhase = PHASE_NORMAL;
            m_uiChargesCount = 0;
            m_uiBerserkTimer = 10 * MINUTE * IN_MILLISECONDS;
            m_uiBoneSliceTimer = 1000;
            m_uiColdflameTimer = 5000;
            m_uiBoneSpikeTimer = 15000;
            m_uiBoneStormTimer = 45000;
            m_uiBoneStormChargeTimer = 3000;
            m_uiBoneStormColdflameTimer = 1000;
        }

        void Aggro(Unit* /*pWho*/) override
        {
            DoScriptText(SAY_AGGRO, m_creature);

            if (m_pInstance)
            {
                m_pInstance->SetData(TYPE_MARROWGAR, IN_PROGRESS);
            }
        }

        void KilledUnit(Unit* pVictim) override
        {
            if (pVictim->GetTypeId() != TYPEID_PLAYER)
            {
                return;
            }

            if (urand(0, 1))
            {
                DoScriptText(urand(0, 1) ? SAY_SLAY_1 : SAY_SLAY_2, m_creature);
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            DoScriptText(SAY_DEATH, m_creature);

            if (m_pInstance)
            {
                m_pInstance->SetData(TYPE_MARROWGAR, DONE);
            }
        }

        void JustReachedHome() override
        {
            if (m_pInstance)
            {
                m_pInstance->SetData(TYPE_MARROWGAR, FAIL);
            }
        }

        void MovementInform(uint32 uiType, uint32 uiPointId) override
        {
            if (uiType != POINT_MOTION_TYPE)
            {
                return;
            }

            if (uiPointId)
            {
                m_uiPhase = PHASE_BONE_STORM_COLDFLAME;
                ++m_uiChargesCount;
            }
        }

        void JustSummoned(Creature* pSummoned) override
        {
            if (pSummoned->GetEntry() == NPC_COLDFLAME)
            {
                pSummoned->CastSpell(pSummoned, SPELL_COLDFLAME_AURA, true);

                float fX, fY;
                float fZ = pSummoned->GetPositionZ();
                // Note: the NearPoint2D function may not be correct here, because we may use a wrong Z value
                m_creature->GetNearPoint2D(fX, fY, 80.0f, m_creature->GetAngle(pSummoned));
                pSummoned->GetMotionMaster()->MovePoint(0, fX, fY, fZ, false);
            }
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            {
                return;
            }

            switch (m_uiPhase)
            {
            case PHASE_NORMAL:

                // Coldflame
                if (m_uiColdflameTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_COLDFLAME) == CAST_OK)
                    {
                        m_uiColdflameTimer = 5000;
                    }
                }
                else
                {
                    m_uiColdflameTimer -= uiDiff;
                }

                // Bone Storm
                if (m_uiBoneStormTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_BONE_STORM) == CAST_OK)
                    {
                        // ToDo: research if we need to increase the speed here
                        DoScriptText(SAY_BONE_STORM, m_creature);
                        m_uiPhase = PHASE_BONE_STORM_CHARGE;
                        SetCombatMovement(false);
                        m_creature->GetMotionMaster()->MoveIdle();
                        m_uiBoneStormTimer = 90000;
                    }
                }
                else
                {
                    m_uiBoneStormTimer -= uiDiff;
                }

                // Bone Slice
                if (m_uiBoneSliceTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature->getVictim(), SPELL_BONE_SLICE) == CAST_OK)
                    {
                        m_uiBoneSliceTimer = 1000;
                    }
                }
                else
                {
                    m_uiBoneSliceTimer -= uiDiff;
                }

                DoMeleeAttackIfReady();

                break;
            case PHASE_BONE_STORM_CHARGE:

                // next charge to random enemy
                if (m_uiBoneStormChargeTimer < uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0, uint32(0), SELECT_FLAG_PLAYER))
                    {
                        float fX, fY, fZ;
                        pTarget->GetPosition(fX, fY, fZ);
                        m_creature->GetMotionMaster()->Clear();
                        m_creature->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                        m_uiBoneStormChargeTimer = 3000;
                        m_uiPhase = PHASE_BONE_STORM_CHARGING;
                    }
                }
                else
                {
                    m_uiBoneStormChargeTimer -= uiDiff;
                }

                break;
            case PHASE_BONE_STORM_CHARGING:
                // waiting to arrive at target position
                break;
            case PHASE_BONE_STORM_COLDFLAME:

                if (m_uiBoneStormColdflameTimer < uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_COLDFLAME_STORM) == CAST_OK)
                    {
                        // When the max cold flame charges are reached, remove Bone storm aura
                        if (m_uiChargesCount == m_uiMaxCharges)
                        {
                            m_creature->RemoveAurasDueToSpell(SPELL_BONE_STORM);
                            m_uiBoneStormTimer = 60000;
                            m_uiBoneSliceTimer = 10000;
                            SetCombatMovement(true);
                            m_creature->GetMotionMaster()->MoveChase(m_creature->getVictim());
                            m_uiChargesCount = 0;
                            m_uiPhase = PHASE_NORMAL;
                        }
                        else
                        {
                            m_uiPhase = PHASE_BONE_STORM_CHARGE;
                        }

                        m_uiBoneStormColdflameTimer = 1000;
                    }
                }
                else
                {
                    m_uiBoneStormColdflameTimer -= uiDiff;
                }

                break;
            }

            // Bone spike - different spells for the normal phase or storm phase
            // ToDo: uncommnet this when vehicles and the Bone spike spells are properly supported by core
            /*if (m_pInstance && (m_pInstance->IsHeroicDifficulty() || m_uiPhase == PHASE_NORMAL))
            {
            if (m_uiBoneSpikeTimer < uiDiff)
            {
            if (DoCastSpellIfCan(m_creature, m_uiPhase == PHASE_NORMAL ? SPELL_BONE_SPIKE : SPELL_BONE_SPIKE_STORM) == CAST_OK)
            {
            switch (urand(0, 2))
            {
            case 0: DoScriptText(SAY_BONE_SPIKE_1, m_creature); break;
            case 1: DoScriptText(SAY_BONE_SPIKE_2, m_creature); break;
            case 2: DoScriptText(SAY_BONE_SPIKE_3, m_creature); break;
            }
            m_uiBoneSpikeTimer = 18000;
            }
            }
            else
            {
                m_uiBoneSpikeTimer -= uiDiff;
            }
            }*/

            // Berserk
            if (m_uiBerserkTimer)
            {
                if (m_uiBerserkTimer <= uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_BERSERK))
                    {
                        DoScriptText(SAY_BERSERK, m_creature);
                        m_uiBerserkTimer = 0;
                    }
                }
                else
                {
                    m_uiBerserkTimer -= uiDiff;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* pCreature) override
    {
        return new boss_lord_marrowgarAI(pCreature);
    }
};

void AddSC_boss_lord_marrowgar()
{
    Script* s;

    s = new boss_lord_marrowgar();
    s->RegisterSelf();

    //pNewScript = new Script;
    //pNewScript->Name = "boss_lord_marrowgar";
    //pNewScript->GetAI = &GetAI_boss_lord_marrowgar;
    //pNewScript->RegisterSelf();
}
