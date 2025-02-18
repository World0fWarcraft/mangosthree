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
SDName: Boss_Taldaram
SD%Complete: 90%
SDComment: Timers;
SDCategory: Ahn'kahet
EndScriptData */

#include "precompiled.h"
#include "ahnkahet.h"

enum
{
    SAY_AGGRO                       = -1619008,
    SAY_VANISH_1                    = -1619009,
    SAY_VANISH_2                    = -1619010,
    SAY_FEED_1                      = -1619011,
    SAY_FEED_2                      = -1619012,
    SAY_SLAY_1                      = -1619013,
    SAY_SLAY_2                      = -1619014,
    SAY_SLAY_3                      = -1619015,
    SAY_DEATH                       = -1619016,

    SPELL_CONJURE_FLAME_SPHERE      = 55931,
    SPELL_FLAME_SPHERE_SUMMON_1     = 55895,        // summons 30106
    SPELL_FLAME_SPHERE_SUMMON_2     = 59511,        // summons 31686
    SPELL_FLAME_SPHERE_SUMMON_3     = 59512,        // summons 31687
    SPELL_BLOODTHIRST               = 55968,
    SPELL_VANISH                    = 55964,
    SPELL_EMBRACE_OF_THE_VAMPYR     = 55959,
    SPELL_EMBRACE_OF_THE_VAMPYR_H   = 59513,

    // Spells used by the Flame Sphere
    SPELL_FLAME_SPHERE_PERIODIC     = 55926,
    SPELL_FLAME_SPHERE_PERIODIC_H   = 59508,
    SPELL_FLAME_SPHERE_SPAWN_EFFECT = 55891,
    SPELL_FLAME_SPHERE_VISUAL       = 55928,
    SPELL_FLAME_SPHERE_DEATH_EFFECT = 55947,
};

/*######
## boss_taldaram
######*/

struct boss_taldaram : public CreatureScript
{
    boss_taldaram() : CreatureScript("boss_taldaram") {}

    struct boss_taldaramAI : public ScriptedAI
    {
        boss_taldaramAI(Creature* pCreature) : ScriptedAI(pCreature)
        {
            m_pInstance = (ScriptedInstance*)pCreature->GetInstanceData();
            m_bIsRegularMode = pCreature->GetMap()->IsRegularDifficulty();
            // Don't set the visual timers if the devices are already activated (reload case)
            m_uiVisualTimer = m_pInstance->GetData(TYPE_TALDARAM) == SPECIAL ? 0 : 1000;
        }

        ScriptedInstance* m_pInstance;
        bool m_bIsRegularMode;

        bool m_bIsFirstAggro;
        uint32 m_uiVisualTimer;
        uint32 m_uiBloodthirstTimer;
        uint32 m_uiFlameOrbTimer;
        uint32 m_uiVanishTimer;
        uint32 m_uiEmbraceTimer;

        GuidList m_lFlameOrbsGuidList;

        void Reset() override
        {
            // Timers seem to be very random...
            m_uiBloodthirstTimer = urand(20000, 25000);
            m_uiFlameOrbTimer = urand(15000, 20000);
            m_uiVanishTimer = 0;
            m_uiEmbraceTimer = 0;
            m_bIsFirstAggro = false;
        }

        void Aggro(Unit* /*pWho*/) override
        {
            // Aggro is called after the boss vanish expires. There is no need to call this multiple times
            if (m_bIsFirstAggro)
            {
                return;
            }

            DoScriptText(SAY_AGGRO, m_creature);
            m_bIsFirstAggro = true;

            if (m_pInstance)
            {
                m_pInstance->SetData(TYPE_TALDARAM, IN_PROGRESS);
            }
        }

        void KilledUnit(Unit* /*pVictim*/) override
        {
            switch (urand(0, 2))
            {
            case 0: DoScriptText(SAY_SLAY_1, m_creature); break;
            case 1: DoScriptText(SAY_SLAY_2, m_creature); break;
            case 2: DoScriptText(SAY_SLAY_3, m_creature); break;
            }
        }

        void JustDied(Unit* /*pKiller*/) override
        {
            DoScriptText(SAY_DEATH, m_creature);

            if (m_pInstance)
            {
                m_pInstance->SetData(TYPE_TALDARAM, DONE);
            }
        }

        void JustReachedHome() override
        {
            if (m_pInstance)
            {
                m_pInstance->SetData(TYPE_TALDARAM, FAIL);
            }
        }

        void EnterEvadeMode() override
        {
            // Don't allow him to evade during vanish
            if (m_uiEmbraceTimer)
            {
                return;
            }

            m_creature->RemoveAllAurasOnEvade();
            m_creature->DeleteThreatList();
            m_creature->CombatStop(true);
            m_creature->LoadCreatureAddon(true);

            // should evade on the ground
            if (m_creature->IsAlive())
            {
                m_creature->GetMotionMaster()->MovePoint(1, aTaldaramLandingLoc[0], aTaldaramLandingLoc[1], aTaldaramLandingLoc[2]);
            }

            m_creature->SetLootRecipient(nullptr);

            Reset();
        }

        void MovementInform(uint32 uiMoveType, uint32 uiPointId) override
        {
            if (uiMoveType != POINT_MOTION_TYPE)
            {
                return;
            }

            // Adjust orientation
            if (uiPointId)
            {
                m_creature->SetLevitate(false);
                m_creature->SetFacingTo(aTaldaramLandingLoc[3]);
            }
        }

        void JustSummoned(Creature* pSummoned) override
        {
            pSummoned->CastSpell(pSummoned, SPELL_FLAME_SPHERE_SPAWN_EFFECT, true);
            pSummoned->CastSpell(pSummoned, SPELL_FLAME_SPHERE_VISUAL, true);

            m_lFlameOrbsGuidList.push_back(pSummoned->GetObjectGuid());
        }

        void SummonedCreatureDespawn(Creature* pSummoned) override
        {
            pSummoned->CastSpell(pSummoned, SPELL_FLAME_SPHERE_DEATH_EFFECT, true);
        }

        // Wrapper which sends each sphere in a different direction
        void ReceiveAIEvent(AIEventType eventType, Creature* /*sender*/, Unit* invoker, uint32 /**/) override
        {
            if (eventType != AI_EVENT_CUSTOM_A || invoker != m_creature)
            {
                return;
            }

            float fX, fY;
            uint8 uiIndex = m_bIsRegularMode ? urand(0, 2) : 0;
            for (GuidList::const_iterator itr = m_lFlameOrbsGuidList.begin(); itr != m_lFlameOrbsGuidList.end(); ++itr)
            {
                if (Creature* pOrb = m_creature->GetMap()->GetCreature(*itr))
                {
                    pOrb->CastSpell(pOrb, m_bIsRegularMode ? SPELL_FLAME_SPHERE_PERIODIC : SPELL_FLAME_SPHERE_PERIODIC_H, true);

                    pOrb->GetNearPoint2D(fX, fY, 70.0f, (2 * M_PI_F / 3)*uiIndex);
                    pOrb->GetMotionMaster()->MovePoint(0, fX, fY, pOrb->GetPositionZ());
                }
                ++uiIndex;
            }
        }

        void UpdateAI(const uint32 uiDiff) override
        {
            if (m_uiVisualTimer)
            {
                if (m_uiVisualTimer <= uiDiff)
                {
                    if (m_pInstance)
                    {
                        m_pInstance->SetData(TYPE_DO_TALDARAM, 0);
                    }
                    m_uiVisualTimer = 0;
                }
                else
                {
                    m_uiVisualTimer -= uiDiff;
                }
            }

            if (!m_creature->SelectHostileTarget() || !m_creature->getVictim())
            {
                return;
            }

            // Cast Embrace of the Vampyr after Vanish expires - note: because of the invisibility effect, the timers won't decrease during vanish
            if (m_uiEmbraceTimer)
            {
                if (m_uiEmbraceTimer <= uiDiff)
                {
                    if (Unit* pTarget = m_creature->SelectAttackingTarget(ATTACKING_TARGET_RANDOM, 0))
                    {
                        if (DoCastSpellIfCan(pTarget, m_bIsRegularMode ? SPELL_EMBRACE_OF_THE_VAMPYR : SPELL_EMBRACE_OF_THE_VAMPYR_H) == CAST_OK)
                        {
                            DoScriptText(urand(0, 1) ? SAY_FEED_1 : SAY_FEED_2, m_creature);
                            m_uiEmbraceTimer = 0;
                        }
                    }
                }
                else
                {
                    m_uiEmbraceTimer -= uiDiff;
                }

                // do not use other abilities during vanish
                return;
            }

            if (m_uiVanishTimer)
            {
                if (m_uiVanishTimer <= uiDiff)
                {
                    if (DoCastSpellIfCan(m_creature, SPELL_VANISH) == CAST_OK)
                    {
                        DoScriptText(urand(0, 1) ? SAY_VANISH_1 : SAY_VANISH_2, m_creature);
                        m_uiVanishTimer = 0;
                        m_uiEmbraceTimer = 2000;
                    }
                }
                else
                {
                    m_uiVanishTimer -= uiDiff;
                }
            }

            if (m_uiBloodthirstTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_BLOODTHIRST) == CAST_OK)
                {
                    m_uiBloodthirstTimer = urand(20000, 25000);
                }
            }
            else
            {
                m_uiBloodthirstTimer -= uiDiff;
            }

            if (m_uiFlameOrbTimer < uiDiff)
            {
                if (DoCastSpellIfCan(m_creature, SPELL_CONJURE_FLAME_SPHERE) == CAST_OK)
                {
                    m_lFlameOrbsGuidList.clear();

                    // Flame speres are summoned above the boss
                    m_creature->CastSpell(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ() + 5.0f, SPELL_FLAME_SPHERE_SUMMON_1, true);

                    // 2 more spheres on heroic
                    if (!m_bIsRegularMode)
                    {
                        m_creature->CastSpell(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ() + 5.0f, SPELL_FLAME_SPHERE_SUMMON_2, true);
                        m_creature->CastSpell(m_creature->GetPositionX(), m_creature->GetPositionY(), m_creature->GetPositionZ() + 5.0f, SPELL_FLAME_SPHERE_SUMMON_3, true);
                    }

                    m_uiFlameOrbTimer = urand(50000, 60000);
                    m_uiVanishTimer = 12000;
                }
            }
            else
            {
                m_uiFlameOrbTimer -= uiDiff;
            }

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* pCreature) override
    {
        return new boss_taldaramAI(pCreature);
    }
};

struct spell_conjure_flame_sphere : public SpellScript
{
    spell_conjure_flame_sphere() : SpellScript("spell_conjure_flame_sphere") {}

    bool EffectDummy(Unit* /*pCaster*/, uint32 uiSpellId, SpellEffectIndex uiEffIndex, Object* pTarget, ObjectGuid /*originalCasterGuid*/) override
    {
        // always check spellid and effectindex
        if (uiSpellId == SPELL_CONJURE_FLAME_SPHERE && uiEffIndex == EFFECT_INDEX_0)
        {
            if (CreatureAI* pBossAI = pTarget->ToCreature()->AI())
            {
                pBossAI->SendAIEvent(AI_EVENT_CUSTOM_A, pTarget->ToCreature(), pTarget->ToCreature());
            }

            // always return true when we are handling this spell and effect
            return true;
        }

        return false;
    }
};

/*######
## go_nerubian_device
######*/

struct go_nerubian_device : public GameObjectScript
{
    go_nerubian_device() : GameObjectScript("go_nerubian_device") {}

    bool OnUse(Player* /*pPlayer*/, GameObject* pGo) override
    {
        ScriptedInstance* pInstance = (ScriptedInstance*)pGo->GetInstanceData();

        if (!pInstance)
        {
            return false;
        }

        // Don't allow players to use the devices if encounter is already finished or in progress (reload case)
        if (pInstance->GetData(TYPE_TALDARAM) == SPECIAL || pInstance->GetData(TYPE_TALDARAM) == DONE)
        {
            return false;
        }

        pInstance->SetData(TYPE_TALDARAM, SPECIAL);
        return false;
    }
};

void AddSC_boss_taldaram()
{
    Script* s;

    s = new boss_taldaram();
    s->RegisterSelf();

    s = new go_nerubian_device();
    s->RegisterSelf();

    s = new spell_conjure_flame_sphere();
    s->RegisterSelf();

    //pNewScript = new Script;
    //pNewScript->Name = "boss_taldaram";
    //pNewScript->GetAI = &GetAI_boss_taldaram;
    //pNewScript->pEffectDummyNPC = &EffectDummyCreature_spell_conjure_flame_orbs;
    //pNewScript->RegisterSelf();

    //pNewScript = new Script;
    //pNewScript->Name = "go_nerubian_device";
    //pNewScript->pGOUse = &GOUse_go_nerubian_device;
    //pNewScript->RegisterSelf();
}
