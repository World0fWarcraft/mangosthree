/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2025 MaNGOS <https://www.getmangos.eu>
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

#ifndef MANGOS_H_SPELLMGR
#define MANGOS_H_SPELLMGR

// For static or at-server-startup loaded spell data
// For more high level function for sSpellStore data

#include "Common.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "DBCStructure.h"

#include <map>

class Player;
class Spell;
class Unit;
struct CreatureInfo;

// only used in code
enum SpellCategories
{
    SPELLCATEGORY_HEALTH_MANA_POTIONS = 4,
    SPELLCATEGORY_DEVOUR_MAGIC        = 12,
    SPELLCATEGORY_JUDGEMENT           = 1210,               // Judgement (seal trigger)
};

/**
 * Spell clasification (Taken from comments)
 * \todo Properly document this
 */
enum SpellSpecific
{
    SPELL_NORMAL            = 0,
    SPELL_SEAL              = 1,
    SPELL_BLESSING          = 2,
    SPELL_AURA              = 3,
    SPELL_STING             = 4,
    SPELL_CURSE             = 5,
    SPELL_ASPECT            = 6,
    SPELL_TRACKER           = 7,
    SPELL_WARLOCK_ARMOR     = 8,
    SPELL_MAGE_ARMOR        = 9,
    SPELL_ELEMENTAL_SHIELD  = 10,
    SPELL_MAGE_POLYMORPH    = 11,
    SPELL_POSITIVE_SHOUT    = 12,
    SPELL_JUDGEMENT         = 13,
    SPELL_BATTLE_ELIXIR     = 14,
    SPELL_GUARDIAN_ELIXIR   = 15,
    SPELL_FLASK_ELIXIR      = 16,
    SPELL_PRESENCE          = 17,
    SPELL_HAND              = 18,
    SPELL_WELL_FED          = 19,
    SPELL_FOOD              = 20,
    SPELL_DRINK             = 21,
    SPELL_FOOD_AND_DRINK    = 22,
    SPELL_UA_IMMOLATE       = 23,                           // Unstable Affliction and Immolate
};

SpellSpecific GetSpellSpecific(uint32 spellId);

// Different spell properties
inline float GetSpellRadius(SpellRadiusEntry const* radius) { return (radius ? radius->Radius : 0); }
uint32 GetSpellCastTime(SpellEntry const* spellInfo, Spell const* spell = NULL);
uint32 GetSpellCastTimeForBonus(SpellEntry const* spellProto, DamageEffectType damagetype);
float CalculateDefaultCoefficient(SpellEntry const* spellProto, DamageEffectType const damagetype);
inline float GetSpellMinRange(SpellRangeEntry const* range, bool friendly = false)
{
    if (!range)
    {
        return 0;
    }
    return (friendly ? range->minRangeFriendly : range->minRange);
}
inline float GetSpellMaxRange(SpellRangeEntry const* range, bool friendly = false)
{
    if (!range)
    {
        return 0;
    }
    return (friendly ? range->maxRangeFriendly : range->maxRange);
}
inline uint32 GetSpellRecoveryTime(SpellEntry const *spellInfo)
{
    if (SpellCooldownsEntry const* cooldowns = spellInfo->GetSpellCooldowns())
    {
        return cooldowns->RecoveryTime > cooldowns->CategoryRecoveryTime ? cooldowns->RecoveryTime : cooldowns->CategoryRecoveryTime;
    }
    return 0;
}
int32 GetSpellDuration(SpellEntry const* spellInfo);
int32 GetSpellMaxDuration(SpellEntry const* spellInfo);
int32 CalculateSpellDuration(SpellEntry const* spellInfo, Unit const* caster = NULL);
uint16 GetSpellAuraMaxTicks(SpellEntry const* spellInfo);
uint16 GetSpellAuraMaxTicks(uint32 spellId);
WeaponAttackType GetWeaponAttackType(SpellEntry const* spellInfo);

inline bool IsSpellHaveEffect(SpellEntry const* spellInfo, SpellEffects effect)
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i)))
            if (SpellEffects(effectEntry->Effect) == effect)
            {
                return true;
            }
    }
    return false;
}

inline bool IsAuraApplyEffect(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    SpellEffectEntry const* spellEffect = spellInfo->GetSpellEffect(SpellEffectIndex(effecIdx));
    if (!spellEffect)
    {
        return false;
    }

    switch (spellEffect->Effect)
    {
        case SPELL_EFFECT_APPLY_AURA:
        case SPELL_EFFECT_APPLY_AREA_AURA_PARTY:
        case SPELL_EFFECT_APPLY_AREA_AURA_RAID:
        case SPELL_EFFECT_APPLY_AREA_AURA_PET:
        case SPELL_EFFECT_APPLY_AREA_AURA_FRIEND:
        case SPELL_EFFECT_APPLY_AREA_AURA_ENEMY:
        case SPELL_EFFECT_APPLY_AREA_AURA_OWNER:
            return true;
    }
    return false;
}

inline bool IsSpellAppliesAura(SpellEntry const* spellInfo, uint32 effectMask = ((1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2)))
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (effectMask & (1 << i))
            if (IsAuraApplyEffect(spellInfo, SpellEffectIndex(i)))
            {
                return true;
            }

    return false;
}

inline bool IsEffectHandledOnDelayedSpellLaunch(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(effecIdx);
    if (!effectEntry)
    {
        return false;
    }
    switch (effectEntry->Effect)
    {
        case SPELL_EFFECT_SCHOOL_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE_NOSCHOOL:
        case SPELL_EFFECT_WEAPON_PERCENT_DAMAGE:
        case SPELL_EFFECT_WEAPON_DAMAGE:
        case SPELL_EFFECT_NORMALIZED_WEAPON_DMG:
            return true;
        default:
            return false;
    }
}

inline bool IsPeriodicRegenerateEffect(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(effecIdx);
    if (!effectEntry)
    {
        return false;
    }

    switch (AuraType(effectEntry->EffectApplyAuraName))
    {
        case SPELL_AURA_PERIODIC_ENERGIZE:
        case SPELL_AURA_PERIODIC_HEAL:
        case SPELL_AURA_PERIODIC_HEALTH_FUNNEL:
            return true;
        default:
            return false;
    }
}

bool IsCastEndProcModifierAura(SpellEntry const* spellInfo, SpellEffectIndex effecIdx, SpellEntry const* procSpell);

inline bool IsSpellHaveAura(SpellEntry const* spellInfo, AuraType aura, uint32 effectMask = (1 << EFFECT_INDEX_0) | (1 << EFFECT_INDEX_1) | (1 << EFFECT_INDEX_2))
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
        if (effectMask & (1 << i))
            if (SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i)))
                if (AuraType(effectEntry->EffectApplyAuraName) == aura)
                {
                    return true;
                }
    return false;
}

inline bool IsSpellLastAuraEffect(SpellEntry const* spellInfo, SpellEffectIndex effecIdx)
{
    for (int i = effecIdx + 1; i < MAX_EFFECT_INDEX; ++i)
    {
        if (SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i)))
            if (effectEntry->EffectApplyAuraName)
            {
                return false;
            }
    }

    return true;
}

bool IsNoStackAuraDueToAura(uint32 spellId_1, uint32 spellId_2);

inline bool IsSealSpell(SpellEntry const* spellInfo)
{
    SpellEffectEntry const* spellEffect = spellInfo->GetSpellEffect(EFFECT_INDEX_0);
    //Collection of all the seal family flags. No other paladin spell has any of those.
    return spellInfo->IsFitToFamily(SPELLFAMILY_PALADIN, UI64LIT(0x26000C000A000000)) &&
        // avoid counting target triggered effect as seal for avoid remove it or seal by it.
        spellEffect && spellEffect->EffectImplicitTargetA == TARGET_SELF;
}

inline bool IsElementalShield(SpellEntry const* spellInfo)
{
    SpellClassOptionsEntry const* classOptions = spellInfo->GetSpellClassOptions();
    // family flags 10 (Lightning), 42 (Earth), 37 (Water), proc shield from T2 8 pieces bonus
    return (classOptions && classOptions->SpellFamilyFlags & UI64LIT(0x42000000400)) || spellInfo->Id == 23552;
}

inline bool IsExplicitDiscoverySpell(SpellEntry const* spellInfo)
{
    SpellEffectEntry const* spellEffect0 = spellInfo->GetSpellEffect(EFFECT_INDEX_0);
    SpellEffectEntry const* spellEffect1 = spellInfo->GetSpellEffect(EFFECT_INDEX_1);
    return ((spellEffect0 && (spellEffect0->Effect == SPELL_EFFECT_CREATE_RANDOM_ITEM ||
        spellEffect0->Effect == SPELL_EFFECT_CREATE_ITEM_2) &&
        (spellEffect1 && spellEffect1->Effect == SPELL_EFFECT_SCRIPT_EFFECT)) ||
        spellInfo->Id == 64323);                         // Book of Glyph Mastery (Effect0==SPELL_EFFECT_SCRIPT_EFFECT without any other data)
}

inline bool IsLootCraftingSpell(SpellEntry const* spellInfo)
{
    SpellEffectEntry const* spellEffect0 = spellInfo->GetSpellEffect(EFFECT_INDEX_0);
    if (!spellEffect0)
    {
        return false;
    }

    SpellTotemsEntry const* totems = spellInfo->GetSpellTotems();

    return ((spellEffect0->Effect == SPELL_EFFECT_CREATE_RANDOM_ITEM) ||
        // different random cards from Inscription (121==Virtuoso Inking Set category) or without explicit item or explicit spells
        ((spellEffect0->Effect == SPELL_EFFECT_CREATE_ITEM_2) && ((totems && totems->TotemCategory[0] != 0) || (spellEffect0->EffectItemType == 0))) ||
        (spellInfo->Id == 62941));
}

int32 CompareAuraRanks(uint32 spellId_1, uint32 spellId_2);

// order from less to more strict
bool IsSingleFromSpellSpecificPerTargetPerCaster(SpellSpecific spellSpec1, SpellSpecific spellSpec2);
bool IsSingleFromSpellSpecificSpellRanksPerTarget(SpellSpecific spellSpec1, SpellSpecific spellSpec2);
bool IsSingleFromSpellSpecificPerTarget(SpellSpecific spellSpec1, SpellSpecific spellSpec2);

bool IsPassiveSpell(uint32 spellId);
bool IsPassiveSpell(SpellEntry const* spellProto);

inline bool IsPassiveSpellStackableWithRanks(SpellEntry const* spellProto)
{
    MANGOS_ASSERT(IsPassiveSpell(spellProto));
    return !IsSpellHaveEffect(spellProto, SPELL_EFFECT_APPLY_AURA);
}

inline bool IsSpellRemoveAllMovementAndControlLossEffects(SpellEntry const* spellProto)
{
    SpellEffectEntry const* spellEffect0 = spellProto->GetSpellEffect(EFFECT_INDEX_0);
    if (!spellEffect0)
    {
        return false;
    }

    return spellEffect0->EffectApplyAuraName == SPELL_AURA_MECHANIC_IMMUNITY &&
        spellEffect0->EffectMiscValue == 1 &&
        spellProto->GetEffectApplyAuraNameByIndex(EFFECT_INDEX_1) == SPELL_AURA_NONE &&
        spellProto->GetEffectApplyAuraNameByIndex(EFFECT_INDEX_2) == SPELL_AURA_NONE &&
        spellProto->HasAttribute(SPELL_ATTR_EX_DISPEL_AURAS_ON_IMMUNITY)/* && -- all above selected spells have SPELL_ATTR_EX5_* mask
        ((spellProto->AttributesEx5 &
            (SPELL_ATTR_EX5_USABLE_WHILE_CONFUSED|SPELL_ATTR_EX5_USABLE_WHILE_FEARED|SPELL_ATTR_EX5_USABLE_WHILE_STUNNED)) ==
            (SPELL_ATTR_EX5_USABLE_WHILE_CONFUSED|SPELL_ATTR_EX5_USABLE_WHILE_FEARED|SPELL_ATTR_EX5_USABLE_WHILE_STUNNED))*/;
}

inline bool IsDeathOnlySpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX3_CAST_ON_DEAD) || spellInfo->Id == 2584;
}

inline bool IsDeathPersistentSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX3_DEATH_PERSISTENT);
}

inline bool IsNonCombatSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_CANT_USED_IN_COMBAT);
}

bool IsPositiveSpell(uint32 spellId);
bool IsPositiveSpell(SpellEntry const* spellproto);
bool IsPositiveEffect(SpellEntry const* spellInfo, SpellEffectIndex effIndex);
bool IsPositiveTarget(uint32 targetA, uint32 targetB);

bool IsExplicitPositiveTarget(uint32 targetA);
bool IsExplicitNegativeTarget(uint32 targetA);

bool IsSingleTargetSpell(SpellEntry const* spellInfo);
bool IsSingleTargetSpells(SpellEntry const* spellInfo1, SpellEntry const* spellInfo2);

inline bool IsCasterSourceTarget(uint32 target)
{
    switch (target)
    {
        case TARGET_SELF:
        case TARGET_PET:
        case TARGET_ALL_PARTY_AROUND_CASTER:
        case TARGET_IN_FRONT_OF_CASTER:
        case TARGET_MASTER:
        case TARGET_MINION:
        case TARGET_ALL_PARTY:
        case TARGET_ALL_PARTY_AROUND_CASTER_2:
        case TARGET_SELF_FISHING:
        case TARGET_TOTEM_EARTH:
        case TARGET_TOTEM_WATER:
        case TARGET_TOTEM_AIR:
        case TARGET_TOTEM_FIRE:
        case TARGET_AREAEFFECT_GO_AROUND_DEST:
        case TARGET_ALL_RAID_AROUND_CASTER:
        case TARGET_SELF2:
        case TARGET_DIRECTLY_FORWARD:
        case TARGET_NONCOMBAT_PET:
        case TARGET_IN_FRONT_OF_CASTER_30:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsSpellWithCasterSourceTargetsOnly(SpellEntry const* spellInfo)
{
    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i));
        if (!effectEntry)
        {
            continue;
        }

        uint32 targetA = effectEntry->EffectImplicitTargetA;
        if (targetA && !IsCasterSourceTarget(targetA))
        {
            return false;
        }

        uint32 targetB = effectEntry->EffectImplicitTargetB;
        if (targetB && !IsCasterSourceTarget(targetB))
        {
            return false;
        }

        if (!targetA && !targetB)
        {
            return false;
        }
    }
    return true;
}

inline bool IsPointEffectTarget(Targets target)
{
    switch (target)
    {
        case TARGET_INNKEEPER_COORDINATES:
        case TARGET_TABLE_X_Y_Z_COORDINATES:
        case TARGET_CASTER_COORDINATES:
        case TARGET_SCRIPT_COORDINATES:
        case TARGET_CURRENT_ENEMY_COORDINATES:
        case TARGET_DUELVSPLAYER_COORDINATES:
        case TARGET_DYNAMIC_OBJECT_COORDINATES:
        case TARGET_POINT_AT_NORTH:
        case TARGET_POINT_AT_SOUTH:
        case TARGET_POINT_AT_EAST:
        case TARGET_POINT_AT_WEST:
        case TARGET_POINT_AT_NE:
        case TARGET_POINT_AT_NW:
        case TARGET_POINT_AT_SE:
        case TARGET_POINT_AT_SW:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsAreaEffectPossitiveTarget(Targets target)
{
    switch (target)
    {
        case TARGET_ALL_PARTY_AROUND_CASTER:
        case TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER:
        case TARGET_ALL_FRIENDLY_UNITS_IN_AREA:
        case TARGET_ALL_PARTY:
        case TARGET_ALL_PARTY_AROUND_CASTER_2:
        case TARGET_AREAEFFECT_PARTY:
        case TARGET_ALL_RAID_AROUND_CASTER:
        case TARGET_AREAEFFECT_PARTY_AND_CLASS:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsAreaEffectTarget(Targets target)
{
    switch (target)
    {
        case TARGET_AREAEFFECT_INSTANT:
        case TARGET_AREAEFFECT_CUSTOM:
        case TARGET_ALL_ENEMY_IN_AREA:
        case TARGET_ALL_ENEMY_IN_AREA_INSTANT:
        case TARGET_ALL_PARTY_AROUND_CASTER:
        case TARGET_IN_FRONT_OF_CASTER:
        case TARGET_ALL_ENEMY_IN_AREA_CHANNELED:
        case TARGET_ALL_FRIENDLY_UNITS_AROUND_CASTER:
        case TARGET_ALL_FRIENDLY_UNITS_IN_AREA:
        case TARGET_ALL_PARTY:
        case TARGET_ALL_PARTY_AROUND_CASTER_2:
        case TARGET_AREAEFFECT_PARTY:
        case TARGET_AREAEFFECT_GO_AROUND_DEST:
        case TARGET_ALL_RAID_AROUND_CASTER:
        case TARGET_AREAEFFECT_PARTY_AND_CLASS:
        case TARGET_IN_FRONT_OF_CASTER_30:
            return true;
        default:
            break;
    }
    return false;
}

inline bool IsAreaOfEffectSpell(SpellEntry const* spellInfo)
{
    SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(EFFECT_INDEX_0);
    if (effectEntry && (IsAreaEffectTarget(Targets(effectEntry->EffectImplicitTargetA)) || IsAreaEffectTarget(Targets(effectEntry->EffectImplicitTargetB))))
    {
        return true;
    }
    effectEntry = spellInfo->GetSpellEffect(EFFECT_INDEX_1);
    if (effectEntry && (IsAreaEffectTarget(Targets(effectEntry->EffectImplicitTargetA)) || IsAreaEffectTarget(Targets(effectEntry->EffectImplicitTargetB))))
    {
        return true;
    }
    effectEntry = spellInfo->GetSpellEffect(EFFECT_INDEX_2);
    if (effectEntry && (IsAreaEffectTarget(Targets(effectEntry->EffectImplicitTargetA)) || IsAreaEffectTarget(Targets(effectEntry->EffectImplicitTargetB))))
    {
        return true;
    }
    return false;
}

inline bool IsAreaAuraEffect(uint32 effect)
{
    if (effect == SPELL_EFFECT_APPLY_AREA_AURA_PARTY    ||
            effect == SPELL_EFFECT_APPLY_AREA_AURA_RAID     ||
            effect == SPELL_EFFECT_APPLY_AREA_AURA_FRIEND   ||
            effect == SPELL_EFFECT_APPLY_AREA_AURA_ENEMY    ||
            effect == SPELL_EFFECT_APPLY_AREA_AURA_PET      ||
            effect == SPELL_EFFECT_APPLY_AREA_AURA_OWNER)
            {
                return true;
            }
    return false;
}

inline bool HasAreaAuraEffect(SpellEntry const* spellInfo)
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i));
        if (!effectEntry)
        {
            continue;
        }
        if (IsAreaAuraEffect(effectEntry->Effect))
        {
            return true;
        }
    }
    return false;
}

inline bool HasAuraWithTriggerEffect(SpellEntry const* spellInfo)
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i));
        if (!effectEntry)
        {
            continue;
        }

        switch(effectEntry->Effect)
        {
            case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
            case SPELL_AURA_PROC_TRIGGER_SPELL:
            case SPELL_AURA_PROC_TRIGGER_DAMAGE:
            case SPELL_AURA_PROC_TRIGGER_SPELL_WITH_VALUE:
                return true;
        }
    }
    return false;
}

inline bool IsOnlySelfTargeting(SpellEntry const* spellInfo)
{
    for (int32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i));
        if (!effectEntry)
        {
            continue;
        }

        switch (effectEntry->EffectImplicitTargetA)
        {
            case TARGET_SELF:
            case TARGET_SELF2:
                break;
            default:
                return false;
        }
        switch (effectEntry->EffectImplicitTargetB)
        {
            case TARGET_SELF:
            case TARGET_SELF2:
            case TARGET_NONE:
                break;
            default:
                return false;
        }
    }
    return true;
}

inline bool IsDispelSpell(SpellEntry const* spellInfo)
{
    return IsSpellHaveEffect(spellInfo, SPELL_EFFECT_DISPEL);
}

inline bool isSpellBreakStealth(SpellEntry const* spellInfo)
{
    return !spellInfo->HasAttribute(SPELL_ATTR_EX_NOT_BREAK_STEALTH);
}

inline bool IsAutoRepeatRangedSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_RANGED) && spellInfo->HasAttribute(SPELL_ATTR_EX2_AUTOREPEAT_FLAG);
}

inline bool IsSpellRequiresRangedAP(SpellEntry const* spellInfo)
{
    return (spellInfo->GetSpellFamilyName() == SPELLFAMILY_HUNTER && spellInfo->GetDmgClass() != SPELL_DAMAGE_CLASS_MELEE);
}

SpellCastResult GetErrorAtShapeshiftedCast(SpellEntry const* spellInfo, uint32 form);

inline bool IsChanneledSpell(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_1) || spellInfo->HasAttribute(SPELL_ATTR_EX_CHANNELED_2);
}

inline bool IsNeedCastSpellAtFormApply(SpellEntry const* spellInfo, ShapeshiftForm form)
{
    if ((!spellInfo->HasAttribute(SPELL_ATTR_PASSIVE) && !spellInfo->HasAttribute(SPELL_ATTR_UNK7)) || !form)
    {
        return false;
    }

    SpellShapeshiftEntry const* shapeShift = spellInfo->GetSpellShapeshift();
    if (!shapeShift)
    {
        return false;
    }

    // passive spells with SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT are already active without shapeshift, do no recast!
    return (shapeShift->Stances & (1<<(form-1)) && !(spellInfo->AttributesEx2 & SPELL_ATTR_EX2_NOT_NEED_SHAPESHIFT));
}

inline bool IsNeedCastSpellAtOutdoor(SpellEntry const* spellInfo)
{
    return (spellInfo->HasAttribute(SPELL_ATTR_OUTDOORS_ONLY) && spellInfo->HasAttribute(SPELL_ATTR_PASSIVE));
}

inline bool NeedsComboPoints(SpellEntry const* spellInfo)
{
    return spellInfo->HasAttribute(SPELL_ATTR_EX_REQ_TARGET_COMBO_POINTS) || spellInfo->HasAttribute(SPELL_ATTR_EX_REQ_COMBO_POINTS);
}

inline SpellSchoolMask GetSpellSchoolMask(SpellEntry const* spellInfo)
{
    return SpellSchoolMask(spellInfo->SchoolMask);
}

inline uint32 GetSpellMechanicMask(SpellEntry const* spellInfo, uint32 effectMask)
{
    uint32 mask = 0;
    if (uint32 mech = spellInfo->GetMechanic())
    {
        mask |= 1 << (mech - 1);
    }

    for (uint32 i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        if (!(effectMask & (1 << i)))
        {
            continue;
        }

        SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i));
        if (!effectEntry)
        {
            continue;
        }

        if (effectEntry->EffectMechanic)
        {
            mask |= 1 << (effectEntry->EffectMechanic - 1);
        }
    }

    return mask;
}

inline uint32 GetAllSpellMechanicMask(SpellEntry const* spellInfo)
{
    SpellCategoriesEntry const* spellCategory = spellInfo->GetSpellCategories();
    uint32 mask = 0;
    if (spellCategory && spellCategory->Mechanic)
    {
        mask |= 1 << (spellCategory->Mechanic - 1);
    }

    for (int i = 0; i < MAX_EFFECT_INDEX; ++i)
    {
        SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(i));
        if (effectEntry && effectEntry->EffectMechanic)
        {
            mask |= 1 << (effectEntry->EffectMechanic-1);
        }
    }

    return mask;
}

inline Mechanics GetEffectMechanic(SpellEntry const* spellInfo, SpellEffectIndex effect)
{
    SpellEffectEntry const* effectEntry = spellInfo->GetSpellEffect(SpellEffectIndex(effect));
    if (effectEntry && effectEntry->EffectMechanic)
    {
        return Mechanics(effectEntry->EffectMechanic);
    }
    SpellCategoriesEntry const* spellCategory = spellInfo->GetSpellCategories();
    if (spellCategory && spellCategory->Mechanic)
    {
        return Mechanics(spellCategory->Mechanic);
    }
    return MECHANIC_NONE;
}

inline uint32 GetDispellMask(DispelType dispel)
{
    // If dispell all
    if (dispel == DISPEL_ALL)
    {
        return DISPEL_ALL_MASK;
    }
    else
    {
        return (1 << dispel);
    }
}

// Diminishing Returns interaction with spells
DiminishingGroup GetDiminishingReturnsGroupForSpell(SpellEntry const* spellproto, bool triggered);
bool IsDiminishingReturnsGroupDurationLimited(DiminishingGroup group);
DiminishingReturnsType GetDiminishingReturnsGroupType(DiminishingGroup group);
int32 GetDiminishingReturnsLimitDuration(DiminishingGroup group, SpellEntry const* spellproto);

SpellEntry const* GetSpellEntryByDifficulty(uint32 id, Difficulty difficulty, bool isRaid);

int32 GetMasteryCoefficient(SpellEntry const * spellProto);
/**
 * Spell proc event related declarations (accessed using SpellMgr functions) (Taken from comments)
 */
enum ProcFlags
{
    PROC_FLAG_NONE                          = 0x00000000,

    PROC_FLAG_KILLED                        = 0x00000001,   // 00 Killed by aggressor
    PROC_FLAG_KILL                          = 0x00000002,   // 01 Kill target (in most cases need XP/Honor reward, see Unit::IsTriggeredAtSpellProcEvent for additinoal check)

    PROC_FLAG_SUCCESSFUL_MELEE_HIT          = 0x00000004,   // 02 Successful melee auto attack
    PROC_FLAG_TAKEN_MELEE_HIT               = 0x00000008,   // 03 Taken damage from melee auto attack hit

    PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT    = 0x00000010,   // 04 Successful attack by Spell that use melee weapon
    PROC_FLAG_TAKEN_MELEE_SPELL_HIT         = 0x00000020,   // 05 Taken damage by Spell that use melee weapon

    PROC_FLAG_SUCCESSFUL_RANGED_HIT         = 0x00000040,   // 06 Successful Ranged auto attack
    PROC_FLAG_TAKEN_RANGED_HIT              = 0x00000080,   // 07 Taken damage from ranged auto attack

    PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT   = 0x00000100,   // 08 Successful Ranged attack by Spell that use ranged weapon
    PROC_FLAG_TAKEN_RANGED_SPELL_HIT        = 0x00000200,   // 09 Taken damage by Spell that use ranged weapon

    PROC_FLAG_SUCCESSFUL_POSITIVE_AOE_HIT   = 0x00000400,   // 10 Successful AoE (not 100% shure unused)
    PROC_FLAG_TAKEN_POSITIVE_AOE            = 0x00000800,   // 11 Taken AoE      (not 100% shure unused)

    PROC_FLAG_SUCCESSFUL_AOE_SPELL_HIT      = 0x00001000,   // 12 Successful AoE damage spell hit (not 100% shure unused)
    PROC_FLAG_TAKEN_AOE_SPELL_HIT           = 0x00002000,   // 13 Taken AoE damage spell hit      (not 100% shure unused)

    PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL     = 0x00004000,   // 14 Successful cast positive spell (by default only on healing)
    PROC_FLAG_TAKEN_POSITIVE_SPELL          = 0x00008000,   // 15 Taken positive spell hit (by default only on healing)

    PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT = 0x00010000,   // 16 Successful negative spell cast (by default only on damage)
    PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT      = 0x00020000,   // 17 Taken negative spell (by default only on damage)

    PROC_FLAG_ON_DO_PERIODIC                = 0x00040000,   // 18 Successful do periodic (damage / healing, determined by PROC_EX_PERIODIC_POSITIVE or negative if no procEx)
    PROC_FLAG_ON_TAKE_PERIODIC              = 0x00080000,   // 19 Taken spell periodic (damage / healing, determined by PROC_EX_PERIODIC_POSITIVE or negative if no procEx)

    PROC_FLAG_TAKEN_ANY_DAMAGE              = 0x00100000,   // 20 Taken any damage
    PROC_FLAG_ON_TRAP_ACTIVATION            = 0x00200000,   // 21 On trap activation

    PROC_FLAG_TAKEN_OFFHAND_HIT             = 0x00400000,   // 22 Taken off-hand melee attacks(not used)
    PROC_FLAG_SUCCESSFUL_OFFHAND_HIT        = 0x00800000,   // 23 Successful off-hand melee attacks

    PROC_FLAG_ON_DEATH                      = 0x01000000,   // 24 On death
    PROC_FLAG_ON_JUMP                       = 0x02000000,   // 25 On jump

    PROC_FLAG_UNK26                         = 0x04000000,   // 26 not used
    PROC_FLAG_UNK27                         = 0x08000000,   // 27 not used

    PROC_FLAG_UNK28                         = 0x10000000,   // 28 not used
    PROC_FLAG_UNK29                         = 0x20000000,   // 29 not used

    PROC_FLAG_UNK30                         = 0x40000000,   // 30 not used
    PROC_FLAG_UNK31                         = 0x80000000,   // 31 not used
};

/// Proc flags for a melee based trigger
#define MELEE_BASED_TRIGGER_MASK (PROC_FLAG_SUCCESSFUL_MELEE_HIT        | \
                                  PROC_FLAG_TAKEN_MELEE_HIT             | \
                                  PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT  | \
                                  PROC_FLAG_TAKEN_MELEE_SPELL_HIT       | \
                                  PROC_FLAG_SUCCESSFUL_RANGED_HIT       | \
                                  PROC_FLAG_TAKEN_RANGED_HIT            | \
                                  PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT | \
                                  PROC_FLAG_TAKEN_RANGED_SPELL_HIT)

/**
 * Proc flags mask for a negative trigger
 * \todo What is negative in this case?
 */
#define NEGATIVE_TRIGGER_MASK (MELEE_BASED_TRIGGER_MASK                | \
                               PROC_FLAG_SUCCESSFUL_AOE_SPELL_HIT      | \
                               PROC_FLAG_TAKEN_AOE_SPELL_HIT           | \
                               PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT | \
                               PROC_FLAG_TAKEN_NEGATIVE_SPELL_HIT)

#define SPELL_CAST_TRIGGER_MASK (PROC_FLAG_SUCCESSFUL_MELEE_SPELL_HIT    | \
                                 PROC_FLAG_SUCCESSFUL_RANGED_HIT         | \
                                 PROC_FLAG_SUCCESSFUL_RANGED_SPELL_HIT   | \
                                 PROC_FLAG_SUCCESSFUL_POSITIVE_AOE_HIT   | \
                                 PROC_FLAG_SUCCESSFUL_AOE_SPELL_HIT      | \
                                 PROC_FLAG_SUCCESSFUL_POSITIVE_SPELL     | \
                                 PROC_FLAG_SUCCESSFUL_NEGATIVE_SPELL_HIT)
enum ProcFlagsEx
{
    /// If none can trigger on Hit/Crit only (passive spells MUST defined by SpellFamily flag)
    PROC_EX_NONE                = 0x0000000,
    /// If set only from normal hit (only damage spells)
    PROC_EX_NORMAL_HIT          = 0x0000001,
    PROC_EX_CRITICAL_HIT        = 0x0000002,
    PROC_EX_MISS                = 0x0000004,
    PROC_EX_RESIST              = 0x0000008,
    PROC_EX_DODGE               = 0x0000010,
    PROC_EX_PARRY               = 0x0000020,
    PROC_EX_BLOCK               = 0x0000040,
    PROC_EX_EVADE               = 0x0000080,
    PROC_EX_IMMUNE              = 0x0000100,
    PROC_EX_DEFLECT             = 0x0000200,
    PROC_EX_ABSORB              = 0x0000400,
    PROC_EX_REFLECT             = 0x0000800,
    /// Melee hit result can be Interrupt (not used)
    PROC_EX_INTERRUPT           = 0x0001000,
    PROC_EX_FULL_BLOCK          = 0x0002000,                // block al attack damage
    PROC_EX_RESERVED2           = 0x0004000,
    PROC_EX_RESERVED3           = 0x0008000,
    /// If set trigger always ( no matter another flags) used for drop charges
    PROC_EX_EX_TRIGGER_ALWAYS   = 0x0010000,
    /// If set trigger always but only one time (not used)
    PROC_EX_EX_ONE_TIME_TRIGGER = 0x0020000,
    /// For periodic heal
    PROC_EX_PERIODIC_POSITIVE   = 0x0040000,
    PROC_EX_CAST_END            = 0x0080000                 // procs on end of cast
};

struct SpellProcEventEntry
{
    uint32      schoolMask;                                 // if nonzero - bit mask for matching proc condition based on spell candidate's school: Fire=2, Mask=1<<(2-1)=2
    uint32      spellFamilyName;                            // if nonzero - for matching proc condition based on candidate spell's SpellFamilyNamer value
    ClassFamilyMask spellFamilyMask[MAX_EFFECT_INDEX];      // if nonzero - for matching proc condition based on candidate spell's SpellFamilyFlags  (like auras 107 and 108 do)
    uint32      procFlags;                                  // bitmask for matching proc event
    uint32      procEx;                                     // proc Extend info (see ProcFlagsEx)
    float       ppmRate;                                    // for melee (ranged?) damage spells - proc rate per minute. if zero, falls back to flat chance from Spell.dbc
    float       customChance;                               // Owerride chance (in most cases for debug only)
    uint32      cooldown;                                   // hidden cooldown used for some spell proc events, applied to _triggered_spell_
};

struct SpellBonusEntry
{
    float  direct_damage;
    float  dot_damage;
    float  ap_bonus;
    float  ap_dot_bonus;
};

typedef std::unordered_map<uint32, SpellProcEventEntry> SpellProcEventMap;
typedef std::unordered_map<uint32, SpellBonusEntry>     SpellBonusMap;

#define ELIXIR_BATTLE_MASK    0x01
#define ELIXIR_GUARDIAN_MASK  0x02
#define ELIXIR_FLASK_MASK     (ELIXIR_BATTLE_MASK|ELIXIR_GUARDIAN_MASK)
#define ELIXIR_UNSTABLE_MASK  0x04
#define ELIXIR_SHATTRATH_MASK 0x08
#define ELIXIR_WELL_FED       0x10                          // Some foods have SPELLFAMILY_POTION

struct SpellThreatEntry
{
    uint16 threat;
    float multiplier;
    float ap_bonus;
};

typedef std::map<uint32, uint8> SpellElixirMap;
typedef std::map<uint32, float> SpellProcItemEnchantMap;
typedef std::map<uint32, SpellThreatEntry> SpellThreatMap;

// Spell script target related declarations (accessed using SpellMgr functions)
enum SpellTargetType
{
    SPELL_TARGET_TYPE_GAMEOBJECT = 0,
    SPELL_TARGET_TYPE_CREATURE   = 1,
    SPELL_TARGET_TYPE_DEAD       = 2
};

#define MAX_SPELL_TARGET_TYPE 3

// pre-defined targeting for spells
struct SpellTargetEntry
{
    uint32 spellId;
    uint32 type;
    uint32 targetEntry;
    uint32 inverseEffectMask;

    bool CanNotHitWithSpellEffect(SpellEffectIndex effect) const { return inverseEffectMask & (1 << effect); }
};

// coordinates for spells (accessed using SpellMgr functions)
struct SpellTargetPosition
{
    uint32 target_mapId;
    float  target_X;
    float  target_Y;
    float  target_Z;
    float  target_Orientation;
};

typedef std::unordered_map<uint32, SpellTargetPosition> SpellTargetPositionMap;

// Spell pet auras
class PetAura
{
    public:
        PetAura() :
            removeOnChangePet(false),
            damage(0)
        {
        }

        PetAura(uint32 petEntry, uint32 aura, bool _removeOnChangePet, int _damage) :
            removeOnChangePet(_removeOnChangePet), damage(_damage)
        {
            auras[petEntry] = aura;
        }

        uint32 GetAura(uint32 petEntry) const
        {
            std::map<uint32, uint32>::const_iterator itr = auras.find(petEntry);
            if (itr != auras.end())
            {
                return itr->second;
            }
            else
            {
                std::map<uint32, uint32>::const_iterator itr2 = auras.find(0);
                if (itr2 != auras.end())
                {
                    return itr2->second;
                }
                else
                {
                    return 0;
                }
            }
        }

        void AddAura(uint32 petEntry, uint32 aura)
        {
            auras[petEntry] = aura;
        }

        bool IsRemovedOnChangePet() const
        {
            return removeOnChangePet;
        }

        int32 GetDamage() const
        {
            return damage;
        }

    private:
        std::map<uint32, uint32> auras;
        bool removeOnChangePet;
        int32 damage;
};
typedef std::map<uint32, PetAura> SpellPetAuraMap;

struct SpellArea
{
    uint32 spellId;
    uint32 areaId;                                          // zone/subzone/or 0 is not limited to zone
    uint32 questStart;                                      // quest start (quest must be active or rewarded for spell apply)
    uint32 questEnd;                                        // quest end (quest don't must be rewarded for spell apply)
    uint16 conditionId;                                     // conditionId - will replace questStart, questEnd, raceMask, gender and questStartCanActive
    int32  auraSpell;                                       // spell aura must be applied for spell apply )if possitive) and it don't must be applied in other case
    uint32 raceMask;                                        // can be applied only to races
    Gender gender;                                          // can be applied only to gender
    bool questStartCanActive;                               // if true then quest start can be active (not only rewarded)
    bool autocast;                                          // if true then auto applied at area enter, in other case just allowed to cast

    // helpers
    bool IsFitToRequirements(Player const* player, uint32 newZone, uint32 newArea) const;
    void ApplyOrRemoveSpellIfCan(Player* player, uint32 newZone, uint32 newArea, bool onlyApply) const;
};

typedef std::multimap < uint32 /*applySpellId*/, SpellArea > SpellAreaMap;
typedef std::multimap < uint32 /*auraSpellId*/, SpellArea const* > SpellAreaForAuraMap;
typedef std::multimap < uint32 /*areaOrZoneId*/, SpellArea const* > SpellAreaForAreaMap;
typedef std::pair<SpellAreaMap::const_iterator, SpellAreaMap::const_iterator> SpellAreaMapBounds;
typedef std::pair<SpellAreaForAuraMap::const_iterator, SpellAreaForAuraMap::const_iterator>  SpellAreaForAuraMapBounds;
typedef std::pair<SpellAreaForAreaMap::const_iterator, SpellAreaForAreaMap::const_iterator>  SpellAreaForAreaMapBounds;


// Spell rank chain  (accessed using SpellMgr functions)
struct SpellChainNode
{
    uint32 prev;
    uint32 first;
    uint32 req;
    uint8  rank;
};

typedef std::unordered_map<uint32, SpellChainNode> SpellChainMap;
typedef std::multimap<uint32, uint32> SpellChainMapNext;

// Spell learning properties (accessed using SpellMgr functions)
struct SpellLearnSkillNode
{
    uint16 skill;
    uint16 step;
    uint16 value;                                           // 0  - max skill value for player level
    uint16 maxvalue;                                        // 0  - max skill value for player level
};

typedef std::map<uint32, SpellLearnSkillNode> SpellLearnSkillMap;

struct SpellLearnSpellNode
{
    uint32 spell;
    bool active;                                            // show in spellbook or not
    bool autoLearned;
};

typedef std::multimap<uint32, SpellLearnSpellNode> SpellLearnSpellMap;
typedef std::pair<SpellLearnSpellMap::const_iterator, SpellLearnSpellMap::const_iterator> SpellLearnSpellMapBounds;

typedef std::multimap<uint32, SkillLineAbilityEntry const*> SkillLineAbilityMap;
typedef std::pair<SkillLineAbilityMap::const_iterator, SkillLineAbilityMap::const_iterator> SkillLineAbilityMapBounds;

typedef std::multimap<uint32, SkillRaceClassInfoEntry const*> SkillRaceClassInfoMap;
typedef std::pair<SkillRaceClassInfoMap::const_iterator, SkillRaceClassInfoMap::const_iterator> SkillRaceClassInfoMapBounds;

typedef std::multimap<uint32, uint32> PetLevelupSpellSet;
typedef std::map<uint32, PetLevelupSpellSet> PetLevelupSpellMap;

struct PetDefaultSpellsEntry
{
    PetDefaultSpellsEntry()
    {
        memset(&spellid, 0, sizeof(spellid));
    }

    uint32 spellid[MAX_CREATURE_SPELL_DATA_SLOT];
};

// < 0 for petspelldata id, > 0 for creature_id
typedef std::map<int32, PetDefaultSpellsEntry> PetDefaultSpellsMap;

bool IsPrimaryProfessionSkill(uint32 skill);

inline bool IsProfessionSkill(uint32 skill)
{
    return  IsPrimaryProfessionSkill(skill) || skill == SKILL_FISHING || skill == SKILL_COOKING || skill == SKILL_FIRST_AID;
}

inline bool IsProfessionOrRidingSkill(uint32 skill)
{
    return  IsProfessionSkill(skill) || skill == SKILL_RIDING;
}

class SpellMgr
{
        friend struct DoSpellBonuses;
        friend struct DoSpellProcEvent;
        friend struct DoSpellProcItemEnchant;

        // Constructors
    public:
        SpellMgr();
        ~SpellMgr();

        // Accessors (const or static functions)
    public:

        SpellElixirMap const& GetSpellElixirMap() const { return mSpellElixirs; }

        uint32 GetSpellElixirMask(uint32 spellid) const
        {
            SpellElixirMap::const_iterator itr = mSpellElixirs.find(spellid);
            if (itr == mSpellElixirs.end())
            {
                return 0x0;
            }

            return itr->second;
        }

        SpellSpecific GetSpellElixirSpecific(uint32 spellid) const
        {
            uint32 mask = GetSpellElixirMask(spellid);

            // flasks must have all bits set from ELIXIR_FLASK_MASK
            if ((mask & ELIXIR_FLASK_MASK) == ELIXIR_FLASK_MASK)
            {
                return SPELL_FLASK_ELIXIR;
            }
            else if (mask & ELIXIR_BATTLE_MASK)
            {
                return SPELL_BATTLE_ELIXIR;
            }
            else if (mask & ELIXIR_GUARDIAN_MASK)
            {
                return SPELL_GUARDIAN_ELIXIR;
            }
            else if (mask & ELIXIR_WELL_FED)
            {
                return SPELL_WELL_FED;
            }
            else
            {
                return SPELL_NORMAL;
            }
        }

        SpellThreatEntry const* GetSpellThreatEntry(uint32 spellid) const
        {
            SpellThreatMap::const_iterator itr = mSpellThreatMap.find(spellid);
            if (itr != mSpellThreatMap.end())
            {
                return &itr->second;
            }

            return NULL;
        }

        float GetSpellThreatMultiplier(SpellEntry const* spellInfo) const
        {
            if (!spellInfo)
            {
                return 1.0f;
            }

            if (SpellThreatEntry const* entry = GetSpellThreatEntry(spellInfo->Id))
            {
                return entry->multiplier;
            }

            return 1.0f;
        }

        // Spell proc events
        SpellProcEventEntry const* GetSpellProcEvent(uint32 spellId) const
        {
            SpellProcEventMap::const_iterator itr = mSpellProcEventMap.find(spellId);
            if (itr != mSpellProcEventMap.end())
            {
                return &itr->second;
            }
            return NULL;
        }

        // Spell procs from item enchants
        float GetItemEnchantProcChance(uint32 spellid) const
        {
            SpellProcItemEnchantMap::const_iterator itr = mSpellProcItemEnchantMap.find(spellid);
            if (itr == mSpellProcItemEnchantMap.end())
            {
                return 0.0f;
            }

            return itr->second;
        }

        static bool IsSpellProcEventCanTriggeredBy(SpellProcEventEntry const* spellProcEvent, uint32 EventProcFlag, SpellEntry const* procSpell, uint32 procFlags, uint32 procExtra);

        // Spell bonus data
        SpellBonusEntry const* GetSpellBonusData(uint32 spellId) const
        {
            // Lookup data
            SpellBonusMap::const_iterator itr = mSpellBonusMap.find(spellId);
            if (itr != mSpellBonusMap.end())
            {
                return &itr->second;
            }

            return NULL;
        }

        // Spell target coordinates
        SpellTargetPosition const* GetSpellTargetPosition(uint32 spell_id) const
        {
            SpellTargetPositionMap::const_iterator itr = mSpellTargetPositions.find(spell_id);
            if (itr != mSpellTargetPositions.end())
            {
                return &itr->second;
            }
            return NULL;
        }

        // Spell ranks chains
        SpellChainNode const* GetSpellChainNode(uint32 spell_id) const
        {
            SpellChainMap::const_iterator itr = mSpellChains.find(spell_id);
            if (itr == mSpellChains.end())
            {
                return NULL;
            }

            return &itr->second;
        }

        uint32 GetFirstSpellInChain(uint32 spell_id) const
        {
            if (SpellChainNode const* node = GetSpellChainNode(spell_id))
            {
                return node->first;
            }

            return spell_id;
        }

        uint32 GetPrevSpellInChain(uint32 spell_id) const
        {
            if (SpellChainNode const* node = GetSpellChainNode(spell_id))
            {
                return node->prev;
            }

            return 0;
        }

        SpellChainMapNext const& GetSpellChainNext() const { return mSpellChainsNext; }

        template<typename Worker>
        void doForHighRanks(uint32 spellid, Worker& worker)
        {
            SpellChainMapNext const& nextMap = GetSpellChainNext();
            for (SpellChainMapNext::const_iterator itr = nextMap.lower_bound(spellid); itr != nextMap.upper_bound(spellid); ++itr)
            {
                worker(itr->second);
                doForHighRanks(itr->second, worker);
            }
        }

        // Note: not use rank for compare to spell ranks: spell chains isn't linear order
        // Use IsHighRankOfSpell instead
        uint8 GetSpellRank(uint32 spell_id) const
        {
            if (SpellChainNode const* node = GetSpellChainNode(spell_id))
            {
                return node->rank;
            }

            return 0;
        }

        bool IsHighRankOfSpell(uint32 spell1, uint32 spell2) const
        {
            SpellChainMap::const_iterator itr = mSpellChains.find(spell1);

            uint32 rank2 = GetSpellRank(spell2);

            // not ordered correctly by rank value
            if (itr == mSpellChains.end() || !rank2 || itr->second.rank <= rank2)
            {
                return false;
            }

            // check present in same rank chain
            for (; itr != mSpellChains.end(); itr = mSpellChains.find(itr->second.prev))
                if (itr->second.prev == spell2)
                {
                    return true;
                }

            return false;
        }

        bool IsRankSpellDueToSpell(SpellEntry const* spellInfo_1, uint32 spellId_2) const;
        bool IsNoStackSpellDueToSpell(uint32 spellId_1, uint32 spellId_2) const;
        bool canStackSpellRanksInSpellBook(SpellEntry const* spellInfo) const;
        bool IsRankedSpellNonStackableInSpellBook(SpellEntry const* spellInfo) const
        {
            return !canStackSpellRanksInSpellBook(spellInfo) && GetSpellRank(spellInfo->Id) != 0;
        }

        // return true if spell1 can affect spell2
        bool IsSpellCanAffectSpell(SpellEntry const* spellInfo_1, SpellEntry const* spellInfo_2) const;

        SpellEntry const* SelectAuraRankForLevel(SpellEntry const* spellInfo, uint32 Level) const;

        // Spell learning
        SpellLearnSkillNode const* GetSpellLearnSkill(uint32 spell_id) const
        {
            SpellLearnSkillMap::const_iterator itr = mSpellLearnSkills.find(spell_id);
            if (itr != mSpellLearnSkills.end())
            {
                return &itr->second;
            }
            else
            {
                return NULL;
            }
        }

        bool IsSpellLearnSpell(uint32 spell_id) const
        {
            return mSpellLearnSpells.find(spell_id) != mSpellLearnSpells.end();
        }

        SpellLearnSpellMapBounds GetSpellLearnSpellMapBounds(uint32 spell_id) const
        {
            return mSpellLearnSpells.equal_range(spell_id);
        }

        bool IsSpellLearnToSpell(uint32 spell_id1, uint32 spell_id2) const
        {
            SpellLearnSpellMapBounds bounds = GetSpellLearnSpellMapBounds(spell_id1);
            for (SpellLearnSpellMap::const_iterator i = bounds.first; i != bounds.second; ++i)
                if (i->second.spell == spell_id2)
                {
                    return true;
                }
            return false;
        }

        static bool IsProfessionOrRidingSpell(uint32 spellId);
        static bool IsProfessionSpell(uint32 spellId);
        static bool IsPrimaryProfessionSpell(uint32 spellId);
        bool IsPrimaryProfessionFirstRankSpell(uint32 spellId) const;
        uint32 GetProfessionSpellMinLevel(uint32 spellId);

        bool IsSkillBonusSpell(uint32 spellId) const;

        // Spell correctness for client using
        static bool IsSpellValid(SpellEntry const* spellInfo, Player* pl = NULL, bool msg = true);

        SkillLineAbilityMapBounds GetSkillLineAbilityMapBounds(uint32 spell_id) const
        {
            return mSkillLineAbilityMap.equal_range(spell_id);
        }

        SkillRaceClassInfoMapBounds GetSkillRaceClassInfoMapBounds(uint32 skill_id) const
        {
            return mSkillRaceClassInfoMap.equal_range(skill_id);
        }

        PetAura const* GetPetAura(uint32 spell_id, SpellEffectIndex eff)
        {
            SpellPetAuraMap::const_iterator itr = mSpellPetAuraMap.find((spell_id << 8) + eff);
            if (itr != mSpellPetAuraMap.end())
            {
                return &itr->second;
            }
            else
            {
                return NULL;
            }
        }

        PetLevelupSpellSet const* GetPetLevelupSpellList(uint32 petFamily) const
        {
            PetLevelupSpellMap::const_iterator itr = mPetLevelupSpellMap.find(petFamily);
            if (itr != mPetLevelupSpellMap.end())
            {
                return &itr->second;
            }
            else
            {
                return NULL;
            }
        }

        // < 0 for petspelldata id, > 0 for creature_id
        PetDefaultSpellsEntry const* GetPetDefaultSpellsEntry(int32 id) const
        {
            PetDefaultSpellsMap::const_iterator itr = mPetDefaultSpellsMap.find(id);
            if (itr != mPetDefaultSpellsMap.end())
            {
                return &itr->second;
            }
            return NULL;
        }

        SpellCastResult GetSpellAllowedInLocationError(SpellEntry const* spellInfo, uint32 map_id, uint32 zone_id, uint32 area_id, Player const* player = NULL);

        SpellAreaMapBounds GetSpellAreaMapBounds(uint32 spell_id) const
        {
            return mSpellAreaMap.equal_range(spell_id);
        }

        SpellAreaForAuraMapBounds GetSpellAreaForAuraMapBounds(uint32 spell_id) const
        {
            return mSpellAreaForAuraMap.equal_range(spell_id);
        }

        SpellAreaForAreaMapBounds GetSpellAreaForAreaMapBounds(uint32 area_id) const
        {
            return mSpellAreaForAreaMap.equal_range(area_id);
        }

        // Modifiers
    public:
        static SpellMgr& Instance();

        void CheckUsedSpells(char const* table);

        // Loading data at server startup
        void LoadSpellChains();
        void LoadSpellLearnSkills();
        void LoadSpellLearnSpells();
        void LoadSpellScriptTarget();
        void LoadSpellElixirs();
        void LoadSpellProcEvents();
        void LoadSpellProcItemEnchant();
        void LoadSpellBonuses();
        void LoadSpellTargetPositions();
        void LoadSpellThreats();
        void LoadSkillLineAbilityMap();
        void LoadSkillRaceClassInfoMap();
        void LoadSpellPetAuras();
        void LoadPetLevelupSpellMap();
        void LoadPetDefaultSpells();
        void LoadSpellAreas();

    private:
        bool LoadPetDefaultSpells_helper(CreatureInfo const* cInfo, PetDefaultSpellsEntry& petDefSpells);

        SpellChainMap      mSpellChains;
        SpellChainMapNext  mSpellChainsNext;
        SpellLearnSkillMap mSpellLearnSkills;
        SpellLearnSpellMap mSpellLearnSpells;
        SpellTargetPositionMap mSpellTargetPositions;
        SpellElixirMap     mSpellElixirs;
        SpellThreatMap     mSpellThreatMap;
        SpellProcEventMap  mSpellProcEventMap;
        SpellProcItemEnchantMap mSpellProcItemEnchantMap;
        SpellBonusMap      mSpellBonusMap;
        SkillLineAbilityMap mSkillLineAbilityMap;
        SkillRaceClassInfoMap mSkillRaceClassInfoMap;
        SpellPetAuraMap     mSpellPetAuraMap;
        PetLevelupSpellMap  mPetLevelupSpellMap;
        PetDefaultSpellsMap mPetDefaultSpellsMap;           // only spells not listed in related mPetLevelupSpellMap entry
        SpellAreaMap         mSpellAreaMap;
        SpellAreaForAuraMap  mSpellAreaForAuraMap;
        SpellAreaForAreaMap  mSpellAreaForAreaMap;
};

#define sSpellMgr SpellMgr::Instance()
#endif
