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

#include "DBCStores.h"
#include "DB2Stores.h"
#include "Policies/Singleton.h"
#include "Log.h"
#include "ProgressBar.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "ObjectGuid.h"

#include "DBCfmt.h"

#include <map>

typedef std::map<uint16, uint32> AreaFlagByAreaID;
typedef std::map<uint32, uint32> AreaFlagByMapID;

struct WMOAreaTableTripple
{
    WMOAreaTableTripple(int32 r, int32 a, int32 g) : groupId(g), rootId(r), adtId(a)
    {
    }

    bool operator <(const WMOAreaTableTripple& b) const
    {
        return memcmp(this, &b, sizeof(WMOAreaTableTripple)) < 0;
    }

    // ordered by entropy; that way memcmp will have a minimal medium runtime
    int32 groupId;
    int32 rootId;
    int32 adtId;
};

typedef std::map<WMOAreaTableTripple, WMOAreaTableEntry const*> WMOAreaInfoByTripple;

DBCStorage <AreaTableEntry> sAreaStore(AreaTableEntryfmt);
DBCStorage <AreaGroupEntry> sAreaGroupStore(AreaGroupEntryfmt);
static AreaFlagByAreaID sAreaFlagByAreaID;
static AreaFlagByMapID  sAreaFlagByMapID;                   // for instances without generated *.map files

static WMOAreaInfoByTripple sWMOAreaInfoByTripple;

DBCStorage <AchievementEntry> sAchievementStore(Achievementfmt);
DBCStorage <AchievementCriteriaEntry> sAchievementCriteriaStore(AchievementCriteriafmt);
DBCStorage <AreaTriggerEntry> sAreaTriggerStore(AreaTriggerEntryfmt);
DBCStorage <ArmorLocationEntry> sArmorLocationStore(ArmorLocationfmt);
DBCStorage <AuctionHouseEntry> sAuctionHouseStore(AuctionHouseEntryfmt);
DBCStorage <BankBagSlotPricesEntry> sBankBagSlotPricesStore(BankBagSlotPricesEntryfmt);
DBCStorage <BattlemasterListEntry> sBattlemasterListStore(BattlemasterListEntryfmt);
DBCStorage <BarberShopStyleEntry> sBarberShopStyleStore(BarberShopStyleEntryfmt);
DBCStorage <CharStartOutfitEntry> sCharStartOutfitStore(CharStartOutfitEntryfmt);
DBCStorage <CharTitlesEntry> sCharTitlesStore(CharTitlesEntryfmt);
DBCStorage <ChatChannelsEntry> sChatChannelsStore(ChatChannelsEntryfmt);
DBCStorage <ChrClassesEntry> sChrClassesStore(ChrClassesEntryfmt);
DBCStorage <ChrPowerTypesEntry> sChrPowerTypesStore(ChrClassesXPowerTypesfmt);
// pair<class,power> => powerIndex
uint32 sChrClassXPowerTypesStore[MAX_CLASSES][MAX_POWERS];
// pair<class,powerIndex> => power
uint32 sChrClassXPowerIndexStore[MAX_CLASSES][MAX_STORED_POWERS];
DBCStorage <ChrRacesEntry> sChrRacesStore(ChrRacesEntryfmt);
DBCStorage <CinematicSequencesEntry> sCinematicSequencesStore(CinematicSequencesEntryfmt);
DBCStorage <CreatureDisplayInfoEntry> sCreatureDisplayInfoStore(CreatureDisplayInfofmt);
DBCStorage <CreatureDisplayInfoExtraEntry> sCreatureDisplayInfoExtraStore(CreatureDisplayInfoExtrafmt);
DBCStorage <CreatureFamilyEntry> sCreatureFamilyStore(CreatureFamilyfmt);
DBCStorage <CreatureModelDataEntry> sCreatureModelDataStore(CreatureModelDatafmt);
DBCStorage <CreatureSpellDataEntry> sCreatureSpellDataStore(CreatureSpellDatafmt); // sCreatureModelDataStore
DBCStorage <CreatureTypeEntry> sCreatureTypeStore(CreatureTypefmt);
DBCStorage <CurrencyTypesEntry> sCurrencyTypesStore(CurrencyTypesfmt);

DBCStorage <DestructibleModelDataEntry> sDestructibleModelDataStore(DestructibleModelDataFmt);
DBCStorage <DungeonEncounterEntry> sDungeonEncounterStore(DungeonEncounterfmt);
DBCStorage <DurabilityQualityEntry> sDurabilityQualityStore(DurabilityQualityfmt);
DBCStorage <DurabilityCostsEntry> sDurabilityCostsStore(DurabilityCostsfmt);

DBCStorage <EmotesEntry> sEmotesStore(EmotesEntryfmt);
DBCStorage <EmotesTextEntry> sEmotesTextStore(EmotesTextEntryfmt);

typedef std::map<uint32, SimpleFactionsList> FactionTeamMap;
static FactionTeamMap sFactionTeamMap;
DBCStorage <FactionEntry> sFactionStore(FactionEntryfmt);
DBCStorage <FactionTemplateEntry> sFactionTemplateStore(FactionTemplateEntryfmt);

DBCStorage <GameObjectDisplayInfoEntry> sGameObjectDisplayInfoStore(GameObjectDisplayInfofmt);
DBCStorage <GemPropertiesEntry> sGemPropertiesStore(GemPropertiesEntryfmt);
DBCStorage <GlyphPropertiesEntry> sGlyphPropertiesStore(GlyphPropertiesfmt);
DBCStorage <GlyphSlotEntry> sGlyphSlotStore(GlyphSlotfmt);

DBCStorage <GtBarberShopCostBaseEntry>    sGtBarberShopCostBaseStore(GtBarberShopCostBasefmt);
DBCStorage <GtCombatRatingsEntry>         sGtCombatRatingsStore(GtCombatRatingsfmt);
DBCStorage <GtChanceToMeleeCritBaseEntry> sGtChanceToMeleeCritBaseStore(GtChanceToMeleeCritBasefmt);
DBCStorage <GtChanceToMeleeCritEntry>     sGtChanceToMeleeCritStore(GtChanceToMeleeCritfmt);
DBCStorage <GtChanceToSpellCritBaseEntry> sGtChanceToSpellCritBaseStore(GtChanceToSpellCritBasefmt);
DBCStorage <GtChanceToSpellCritEntry>     sGtChanceToSpellCritStore(GtChanceToSpellCritfmt);
DBCStorage <GtOCTClassCombatRatingScalarEntry> sGtOCTClassCombatRatingScalarStore(GtOCTClassCombatRatingScalarfmt);
//DBCStorage <GtOCTRegenMPEntry>            sGtOCTRegenMPStore(GtOCTRegenMPfmt);  -- not used currently
DBCStorage <GtOCTHpPerStaminaEntry>       sGtOCTHpPerStaminaStore(GtOCTHpPerStaminafmt);
DBCStorage <GtRegenMPPerSptEntry>         sGtRegenMPPerSptStore(GtRegenMPPerSptfmt);
DBCStorage <GtSpellScalingEntry>          sGtSpellScalingStore(GtSpellScalingfmt);
DBCStorage <GtOCTBaseHPByClassEntry>      sGtOCTBaseHPByClassStore(GtOCTBaseHPByClassfmt);
DBCStorage <GtOCTBaseMPByClassEntry>      sGtOCTBaseMPByClassStore(GtOCTBaseMPByClassfmt);

DBCStorage <HolidaysEntry>                sHolidaysStore(Holidaysfmt);

DBCStorage <ItemArmorQualityEntry>        sItemArmorQualityStore(ItemArmorQualityfmt);
DBCStorage <ItemArmorShieldEntry>         sItemArmorShieldStore(ItemArmorShieldfmt);
DBCStorage <ItemArmorTotalEntry>          sItemArmorTotalStore(ItemArmorTotalfmt);
DBCStorage <ItemBagFamilyEntry>           sItemBagFamilyStore(ItemBagFamilyfmt);
DBCStorage <ItemClassEntry>               sItemClassStore(ItemClassfmt);
DBCStorage <ItemDamageEntry>              sItemDamageAmmoStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageOneHandStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageOneHandCasterStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageRangedStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageThrownStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageTwoHandStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageTwoHandCasterStore(ItemDamagefmt);
DBCStorage <ItemDamageEntry>              sItemDamageWandStore(ItemDamagefmt);
//DBCStorage <ItemDisplayInfoEntry> sItemDisplayInfoStore(ItemDisplayTemplateEntryfmt); -- not used currently
DBCStorage <ItemLimitCategoryEntry>       sItemLimitCategoryStore(ItemLimitCategoryEntryfmt);
DBCStorage <ItemRandomPropertiesEntry>    sItemRandomPropertiesStore(ItemRandomPropertiesfmt);
DBCStorage <ItemRandomSuffixEntry>        sItemRandomSuffixStore(ItemRandomSuffixfmt);
DBCStorage <ItemReforgeEntry>             sItemReforgeStore(ItemReforgefmt);
DBCStorage <ItemSetEntry> sItemSetStore(ItemSetEntryfmt);
DBCStorage <LfgDungeonsEntry> sLfgDungeonsStore(LfgDungeonsEntryfmt);
DBCStorage <LiquidTypeEntry> sLiquidTypeStore(LiquidTypefmt);
DBCStorage <LockEntry> sLockStore(LockEntryfmt);

DBCStorage <MailTemplateEntry> sMailTemplateStore(MailTemplateEntryfmt);
DBCStorage <MapEntry> sMapStore(MapEntryfmt);

DBCStorage <MapDifficultyEntry> sMapDifficultyStore(MapDifficultyEntryfmt); // only for loading
MapDifficultyMap sMapDifficultyMap;

DBCStorage <MovieEntry> sMovieStore(MovieEntryfmt);
DBCStorage <MountCapabilityEntry> sMountCapabilityStore(MountCapabilityfmt);
DBCStorage <MountTypeEntry> sMountTypeStore(MountTypefmt);

DBCStorage <NumTalentsAtLevelEntry> sNumTalentsAtLevelStore(NumTalentsAtLevelfmt);

DBCStorage <OverrideSpellDataEntry> sOverrideSpellDataStore(OverrideSpellDatafmt);
DBCStorage <QuestFactionRewardEntry> sQuestFactionRewardStore(QuestFactionRewardfmt);
DBCStorage <QuestSortEntry> sQuestSortStore(QuestSortEntryfmt);
DBCStorage <QuestXPLevel> sQuestXPLevelStore(QuestXPLevelfmt);

DBCStorage <PhaseEntry> sPhaseStore(Phasefmt);
DBCStorage <PowerDisplayEntry> sPowerDisplayStore(PowerDisplayfmt);
DBCStorage <PvPDifficultyEntry> sPvPDifficultyStore(PvPDifficultyfmt);

DBCStorage <RandomPropertiesPointsEntry> sRandomPropertiesPointsStore(RandomPropertiesPointsfmt);
DBCStorage <ScalingStatDistributionEntry> sScalingStatDistributionStore(ScalingStatDistributionfmt);
DBCStorage <ScalingStatValuesEntry> sScalingStatValuesStore(ScalingStatValuesfmt);

DBCStorage <SkillLineEntry> sSkillLineStore(SkillLinefmt);
DBCStorage <SkillLineAbilityEntry> sSkillLineAbilityStore(SkillLineAbilityfmt);
DBCStorage <SkillRaceClassInfoEntry> sSkillRaceClassInfoStore(SkillRaceClassInfofmt);

DBCStorage <SoundEntriesEntry> sSoundEntriesStore(SoundEntriesfmt);

DBCStorage <SpellItemEnchantmentEntry> sSpellItemEnchantmentStore(SpellItemEnchantmentfmt);
DBCStorage <SpellItemEnchantmentConditionEntry> sSpellItemEnchantmentConditionStore(SpellItemEnchantmentConditionfmt);
DBCStorage <SpellEntry> sSpellStore(SpellEntryfmt);
SpellCategoryStore sSpellCategoryStore;
PetFamilySpellsStore sPetFamilySpellsStore;

DBCStorage <SpellAuraOptionsEntry> sSpellAuraOptionsStore(SpellAuraOptionsEntryfmt);
DBCStorage <SpellAuraRestrictionsEntry> sSpellAuraRestrictionsStore(SpellAuraRestrictionsEntryfmt);
DBCStorage <SpellCastingRequirementsEntry> sSpellCastingRequirementsStore(SpellCastingRequirementsEntryfmt);
DBCStorage <SpellCategoriesEntry> sSpellCategoriesStore(SpellCategoriesEntryfmt);
DBCStorage <SpellClassOptionsEntry> sSpellClassOptionsStore(SpellClassOptionsEntryfmt);
DBCStorage <SpellCooldownsEntry> sSpellCooldownsStore(SpellCooldownsEntryfmt);
DBCStorage <SpellEffectEntry> sSpellEffectStore(SpellEffectEntryfmt);
DBCStorage <SpellEquippedItemsEntry> sSpellEquippedItemsStore(SpellEquippedItemsEntryfmt);
DBCStorage <SpellInterruptsEntry> sSpellInterruptsStore(SpellInterruptsEntryfmt);
DBCStorage <SpellLevelsEntry> sSpellLevelsStore(SpellLevelsEntryfmt);
DBCStorage <SpellPowerEntry> sSpellPowerStore(SpellPowerEntryfmt);
DBCStorage <SpellReagentsEntry> sSpellReagentsStore(SpellReagentsEntryfmt);
DBCStorage <SpellScalingEntry> sSpellScalingStore(SpellScalingEntryfmt);
DBCStorage <SpellShapeshiftEntry> sSpellShapeshiftStore(SpellShapeshiftEntryfmt);
DBCStorage <SpellTargetRestrictionsEntry> sSpellTargetRestrictionsStore(SpellTargetRestrictionsEntryfmt);
DBCStorage <SpellTotemsEntry> sSpellTotemsStore(SpellTotemsEntryfmt);

SpellEffectMap sSpellEffectMap;

DBCStorage <SpellCastTimesEntry> sSpellCastTimesStore(SpellCastTimefmt);
DBCStorage <SpellDifficultyEntry> sSpellDifficultyStore(SpellDifficultyfmt);
DBCStorage <SpellDurationEntry> sSpellDurationStore(SpellDurationfmt);
DBCStorage <SpellFocusObjectEntry> sSpellFocusObjectStore(SpellFocusObjectfmt);
DBCStorage <SpellRadiusEntry> sSpellRadiusStore(SpellRadiusfmt);
DBCStorage <SpellRangeEntry> sSpellRangeStore(SpellRangefmt);
DBCStorage <SpellRuneCostEntry> sSpellRuneCostStore(SpellRuneCostfmt);
DBCStorage <SpellShapeshiftFormEntry> sSpellShapeshiftFormStore(SpellShapeshiftFormfmt);
//DBCStorage <StableSlotPricesEntry> sStableSlotPricesStore(StableSlotPricesfmt);
DBCStorage <SummonPropertiesEntry> sSummonPropertiesStore(SummonPropertiesfmt);
DBCStorage <TalentEntry> sTalentStore(TalentEntryfmt);
TalentSpellPosMap sTalentSpellPosMap;
DBCStorage <TalentTabEntry> sTalentTabStore(TalentTabEntryfmt);
DBCStorage <TalentTreePrimarySpellsEntry> sTalentTreePrimarySpellsStore(TalentTreePrimarySpellsfmt);
typedef std::map<uint32, std::vector<uint32> > TalentTreeSpellsMap;
TalentTreeSpellsMap sTalentTreeMasterySpellsMap;
TalentTreeSpellsMap sTalentTreePrimarySpellsMap;
typedef std::map<uint32, uint32> TalentTreeRolesMap;
TalentTreeRolesMap sTalentTreeRolesMap;

// store absolute bit position for first rank for talent inspect
static uint32 sTalentTabPages[MAX_CLASSES][3];

DBCStorage <TaxiNodesEntry> sTaxiNodesStore(TaxiNodesEntryfmt);
TaxiMask sTaxiNodesMask;
TaxiMask sOldContinentsNodesMask;
TaxiMask sHordeTaxiNodesMask;
TaxiMask sAllianceTaxiNodesMask;
TaxiMask sDeathKnightTaxiNodesMask;

// DBC used only for initialization sTaxiPathSetBySource at startup.
TaxiPathSetBySource sTaxiPathSetBySource;
DBCStorage <TaxiPathEntry> sTaxiPathStore(TaxiPathEntryfmt);

// DBC store data but sTaxiPathNodesByPath used for fast access to entries (it's not owner pointed data).
TaxiPathNodesByPath sTaxiPathNodesByPath;
static DBCStorage <TaxiPathNodeEntry> sTaxiPathNodeStore(TaxiPathNodeEntryfmt);

TransportAnimationsByEntry sTransportAnimationsByEntry;
DBCStorage <TransportAnimationEntry> sTransportAnimationStore(TransportAnimationEntryfmt);
DBCStorage <TotemCategoryEntry> sTotemCategoryStore(TotemCategoryEntryfmt);
DBCStorage <VehicleEntry> sVehicleStore(VehicleEntryfmt);
DBCStorage <VehicleSeatEntry> sVehicleSeatStore(VehicleSeatEntryfmt);
DBCStorage <WMOAreaTableEntry>  sWMOAreaTableStore(WMOAreaTableEntryfmt);
DBCStorage <WorldMapAreaEntry>  sWorldMapAreaStore(WorldMapAreaEntryfmt);
DBCStorage <WorldMapOverlayEntry> sWorldMapOverlayStore(WorldMapOverlayEntryfmt);
DBCStorage <WorldSafeLocsEntry> sWorldSafeLocsStore(WorldSafeLocsEntryfmt);
DBCStorage <WorldPvPAreaEntry>  sWorldPvPAreaStore(WorldPvPAreaEnrtyfmt);

typedef std::list<std::string> StoreProblemList;

bool IsAcceptableClientBuild(uint32 build)
{
    int accepted_versions[] = EXPECTED_MANGOSD_CLIENT_BUILD;
    for (int i = 0; accepted_versions[i]; ++i)
        if (int(build) == accepted_versions[i])
        {
            return true;
        }

    return false;
}

std::string AcceptableClientBuildsListStr()
{
    std::ostringstream data;
    int accepted_versions[] = EXPECTED_MANGOSD_CLIENT_BUILD;
    for (int i = 0; accepted_versions[i]; ++i)
    {
        data << accepted_versions[i] << " ";
    }
    return data.str();
}

static bool ReadDBCBuildFileText(const std::string& dbc_path, char const* localeName, std::string& text)
{
    std::string filename  = dbc_path + "component.wow-" + localeName + ".txt";

    if (FILE* file = fopen(filename.c_str(), "rb"))
    {
        char buf[100];
        fread(buf, 1, 100 - 1, file);
        fclose(file);

        text = &buf[0];
        return true;
    }
    else
    {
        return false;
    }
}

int ReadDBCLocale(const std::string sDataPath)
{
    std::string sDBCpath = sDataPath + "dbc/";
    std::string sFilename;

    sLog.outString ("%i Locales defined in core", MAX_LOCALE);
    for (int uLocaleIndex = 0; uLocaleIndex <= MAX_LOCALE; ++uLocaleIndex)
    {
        sFilename  = sDBCpath + "component.wow-" + fullLocaleNameList[uLocaleIndex].name + ".txt";
        if (FILE* file = fopen(sFilename.c_str(), "rb"))
        {
            if (uLocaleIndex==0)
            {
                uLocaleIndex=1;  // Map enus and engb to 0
            }

            return uLocaleIndex-1; // Successfully located the locale
        }
    }

    return -1; // Failed to locate or access the component.wow<locale>.txt file
}

static uint32 ReadDBCBuild(const std::string& dbc_path, LocaleNameStr const*&localeNameStr)
{
    std::string text;

    if (!localeNameStr)
    {
        for (LocaleNameStr const* itr = &fullLocaleNameList[0]; itr->name; ++itr)
        {
            if (ReadDBCBuildFileText(dbc_path, itr->name, text))
            {
                localeNameStr = itr;
                break;
            }
        }
    }
    else
    {
        ReadDBCBuildFileText(dbc_path, localeNameStr->name, text);
    }

    if (text.empty())
    {
        return 0;
    }

    size_t pos = text.find("version=\"");
    size_t pos1 = pos + strlen("version=\"");
    size_t pos2 = text.find("\"", pos1);
    if (pos == text.npos || pos2 == text.npos || pos1 >= pos2)
    {
        return 0;
    }

    std::string build_str = text.substr(pos1, pos2 - pos1);

    int build = atoi(build_str.c_str());
    if (build <= 0)
    {
        return 0;
    }

    return build;
}

static bool LoadDBC_assert_print(uint32 fsize, uint32 rsize, const std::string& filename)
{
    sLog.outError("Size of '%s' setted by format string (%u) not equal size of C++ structure (%u).", filename.c_str(), fsize, rsize);

    // ASSERT must fail after function call
    return false;
}

struct LocalData
{
    LocalData(uint32 build, LocaleConstant loc)
        : main_build(build), defaultLocale(loc), availableDbcLocales(0xFFFFFFFF),checkedDbcLocaleBuilds(0) {}

    uint32 main_build;
    LocaleConstant defaultLocale;

    // bitmasks for index of fullLocaleNameList
    uint32 availableDbcLocales;
    uint32 checkedDbcLocaleBuilds;
};

template<class T>
inline void LoadDBC(LocalData& localeData, BarGoLink& bar, StoreProblemList& errlist, DBCStorage<T>& storage, const std::string& dbc_path, const std::string& filename)
{
    // compatibility format and C++ structure sizes
    MANGOS_ASSERT(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()) == sizeof(T) || LoadDBC_assert_print(DBCFileLoader::GetFormatRecordSize(storage.GetFormat()), sizeof(T), filename));

    std::string dbc_filename = dbc_path + filename;
    if (storage.Load(dbc_filename.c_str(),localeData.defaultLocale))
    {
        bar.step();
        for (uint8 i = 0; fullLocaleNameList[i].name; ++i)
        {
            if (!(localeData.availableDbcLocales & (1 << i)))
            {
                continue;
            }

            LocaleNameStr const* localStr = &fullLocaleNameList[i];

            std::string dbc_dir_loc = dbc_path + localStr->name + "/";

            if (!(localeData.checkedDbcLocaleBuilds & (1 << i)))
            {
                localeData.checkedDbcLocaleBuilds |= (1 << i); // mark as checked for speedup next checks

                uint32 build_loc = ReadDBCBuild(dbc_dir_loc, localStr);
                if (localeData.main_build != build_loc)
                {
                    localeData.availableDbcLocales &= ~(1 << i); // mark as not available for speedup next checks

                    // exist but wrong build
                    if (build_loc)
                    {
                        std::string dbc_filename_loc = dbc_path + localStr->name + "/" + filename;
                        char buf[200];
                        snprintf(buf, 200, " (exist, but DBC locale subdir %s have DBCs for build %u instead expected build %u, it and other DBC from subdir skipped)", localStr->name, build_loc, localeData.main_build);
                        errlist.push_back(dbc_filename_loc + buf);
                    }

                    continue;
                }
            }

            std::string dbc_filename_loc = dbc_path + localStr->name + "/" + filename;
            if (!storage.LoadStringsFrom(dbc_filename_loc.c_str(),localStr->locale))
                localeData.availableDbcLocales &= ~(1 << i);// mark as not available for speedup next checks
        }
    }
    else
    {
        // sort problematic dbc to (1) non compatible and (2) nonexistent
        FILE* f = fopen(dbc_filename.c_str(), "rb");
        if (f)
        {
            char buf[100];
            snprintf(buf, 100, " (exist, but have %u fields instead %zu) Wrong client version DBC file?", storage.GetFieldCount(), strlen(storage.GetFormat()));
            errlist.push_back(dbc_filename + buf);
            fclose(f);
        }
        else
        {
            errlist.push_back(dbc_filename);
        }
    }
}

void LoadDBCStores(const std::string& dataPath)
{
    std::string dbcPath = dataPath + "dbc/";

    LocaleNameStr const* defaultLocaleNameStr = NULL;
    uint32 build = ReadDBCBuild(dbcPath,defaultLocaleNameStr);

    // Check the expected DBC version
    if (!IsAcceptableClientBuild(build))
    {
        if (build)
            sLog.outError("Found DBC files for build %u but mangosd expected DBC for one from builds: %s Please extract correct DBC files.", build, AcceptableClientBuildsListStr().c_str());
        else
        {
            sLog.outError("Incorrect DataDir value in mangosd.conf or not found build info (outdated DBC files). Required one from builds: %s Please extract correct DBC files.", AcceptableClientBuildsListStr().c_str());
        }
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    const uint32 DBCFilesCount = 129;

    BarGoLink bar(DBCFilesCount);

    StoreProblemList bad_dbc_files;

    LocalData availableDbcLocales(build,defaultLocaleNameStr->locale);

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAreaStore,                dbcPath, "AreaTable.dbc");

    // must be after sAreaStore loading
    for (uint32 i = 0; i < sAreaStore.GetNumRows(); ++i)    // areaflag numbered from 0
    {
        if (AreaTableEntry const* area = sAreaStore.LookupEntry(i))
        {
            // fill AreaId->DBC records
            sAreaFlagByAreaID.insert(AreaFlagByAreaID::value_type(uint16(area->ID), area->exploreFlag));

            // fill MapId->DBC records ( skip sub zones and continents )
            if (area->zone == 0 && area->mapid != 0 && area->mapid != 1 && area->mapid != 530 && area->mapid != 571)
            {
                sAreaFlagByMapID.insert(AreaFlagByMapID::value_type(area->mapid, area->exploreFlag));
            }
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAchievementStore,         dbcPath, "Achievement.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAchievementCriteriaStore, dbcPath, "Achievement_Criteria.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAreaTriggerStore,         dbcPath, "AreaTrigger.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAreaGroupStore,           dbcPath, "AreaGroup.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sArmorLocationStore,       dbcPath,"ArmorLocation.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sAuctionHouseStore,        dbcPath, "AuctionHouse.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sBankBagSlotPricesStore,   dbcPath, "BankBagSlotPrices.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sBattlemasterListStore,    dbcPath, "BattlemasterList.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sBarberShopStyleStore,     dbcPath, "BarberShopStyle.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCharStartOutfitStore,     dbcPath, "CharStartOutfit.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sCharTitlesStore,          dbcPath, "CharTitles.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sChatChannelsStore,        dbcPath, "ChatChannels.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sChrClassesStore,          dbcPath, "ChrClasses.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sChrPowerTypesStore,       dbcPath,"ChrClassesXPowerTypes.dbc");
    for (uint32 i = 0; i < MAX_CLASSES; ++i)
    {
        for (uint32 j = 0; j < MAX_POWERS; ++j)
        {
            sChrClassXPowerTypesStore[i][j] = INVALID_POWER_INDEX;
        }
        for (uint32 j = 0; j < MAX_STORED_POWERS; ++j)
        {
            sChrClassXPowerIndexStore[i][j] = INVALID_POWER;
        }
    }
    for (uint32 i = 0; i < sChrPowerTypesStore.GetNumRows(); ++i)
    {
        ChrPowerTypesEntry const* entry = sChrPowerTypesStore.LookupEntry(i);
        if (!entry)
        {
            continue;
        }

        MANGOS_ASSERT(entry->classId < MAX_CLASSES && "MAX_CLASSES not updated");
        MANGOS_ASSERT(entry->power < MAX_POWERS && "MAX_POWERS not updated");

        uint32 index = 0;

        for (uint32 j = 0; j < MAX_POWERS; ++j)
        {
            if (sChrClassXPowerTypesStore[entry->classId][j] != INVALID_POWER_INDEX)
            {
                ++index;
            }
        }

        MANGOS_ASSERT(index < MAX_STORED_POWERS && "MAX_STORED_POWERS not updated");

        sChrClassXPowerTypesStore[entry->classId][entry->power] = index;
        sChrClassXPowerIndexStore[entry->classId][index] = entry->power;
    }
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sChrRacesStore,            dbcPath,"ChrRaces.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCinematicSequencesStore,  dbcPath,"CinematicSequences.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCreatureDisplayInfoStore, dbcPath,"CreatureDisplayInfo.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCreatureDisplayInfoExtraStore,dbcPath,"CreatureDisplayInfoExtra.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCreatureFamilyStore,      dbcPath,"CreatureFamily.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCreatureModelDataStore,   dbcPath,"CreatureModelData.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCreatureSpellDataStore,   dbcPath,"CreatureSpellData.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCreatureTypeStore,        dbcPath,"CreatureType.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sCurrencyTypesStore,       dbcPath,"CurrencyTypes.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sDestructibleModelDataStore,dbcPath,"DestructibleModelData.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sDungeonEncounterStore,    dbcPath,"DungeonEncounter.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sDurabilityCostsStore,     dbcPath,"DurabilityCosts.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sDurabilityQualityStore,   dbcPath,"DurabilityQuality.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sEmotesStore,              dbcPath,"Emotes.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sEmotesTextStore,          dbcPath,"EmotesText.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sFactionStore,             dbcPath,"Faction.dbc");
    for (uint32 i = 0; i < sFactionStore.GetNumRows(); ++i)
    {
        FactionEntry const* faction = sFactionStore.LookupEntry(i);
        if (faction && faction->team)
        {
            SimpleFactionsList& flist = sFactionTeamMap[faction->team];
            flist.push_back(i);
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sFactionTemplateStore,     dbcPath, "FactionTemplate.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGameObjectDisplayInfoStore, dbcPath, "GameObjectDisplayInfo.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGemPropertiesStore,       dbcPath, "GemProperties.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGlyphPropertiesStore,     dbcPath, "GlyphProperties.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGlyphSlotStore,           dbcPath, "GlyphSlot.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtBarberShopCostBaseStore, dbcPath, "gtBarberShopCostBase.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtCombatRatingsStore,     dbcPath, "gtCombatRatings.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtChanceToMeleeCritBaseStore, dbcPath, "gtChanceToMeleeCritBase.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtChanceToMeleeCritStore, dbcPath, "gtChanceToMeleeCrit.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtChanceToSpellCritBaseStore, dbcPath, "gtChanceToSpellCritBase.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtChanceToSpellCritStore, dbcPath, "gtChanceToSpellCrit.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtOCTClassCombatRatingScalarStore, dbcPath, "gtOCTClassCombatRatingScalar.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sGtOCTHpPerStaminaStore,   dbcPath,"gtOCTHpPerStamina.dbc");
    // LoadDBC(availableDbcLocales,bar,bad_dbc_files,sGtOCTRegenMPStore,        dbcPath,"gtOCTRegenMP.dbc");       -- not used currently
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sGtRegenMPPerSptStore,     dbcPath, "gtRegenMPPerSpt.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sGtSpellScalingStore,      dbcPath,"gtSpellScaling.dbc");     // 15595
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sGtOCTBaseHPByClassStore,  dbcPath,"gtOCTBaseHPByClass.dbc"); // 15595
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sGtOCTBaseMPByClassStore,  dbcPath,"gtOCTBaseMPByClass.dbc"); // 15595
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sHolidaysStore,            dbcPath, "Holidays.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemArmorQualityStore,    dbcPath,"ItemArmorQuality.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemArmorShieldStore,     dbcPath,"ItemArmorShield.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemArmorTotalStore,      dbcPath,"ItemArmorTotal.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemBagFamilyStore,       dbcPath, "ItemBagFamily.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemReforgeStore,         dbcPath, "ItemReforge.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemClassStore,           dbcPath, "ItemClass.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageAmmoStore,      dbcPath,"ItemDamageAmmo.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageOneHandStore,   dbcPath,"ItemDamageOneHand.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageOneHandCasterStore,dbcPath,"ItemDamageOneHandCaster.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageRangedStore,    dbcPath,"ItemDamageRanged.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageThrownStore,    dbcPath,"ItemDamageThrown.dbc");
    // LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDisplayInfoStore,     dbcPath,"ItemDisplayInfo.dbc");     -- not used currently

    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageTwoHandStore,   dbcPath,"ItemDamageTwoHand.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageTwoHandCasterStore,dbcPath,"ItemDamageTwoHandCaster.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sItemDamageWandStore,      dbcPath,"ItemDamageWand.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemLimitCategoryStore,   dbcPath, "ItemLimitCategory.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemRandomPropertiesStore, dbcPath, "ItemRandomProperties.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemRandomSuffixStore,    dbcPath, "ItemRandomSuffix.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sItemSetStore,             dbcPath, "ItemSet.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sLfgDungeonsStore,         dbcPath, "LFGDungeons.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sLiquidTypeStore,          dbcPath, "LiquidType.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sLockStore,                dbcPath, "Lock.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sMailTemplateStore,        dbcPath, "MailTemplate.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sMapStore,                 dbcPath, "Map.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sMapDifficultyStore,       dbcPath, "MapDifficulty.dbc");
    // fill data
    for (uint32 i = 1; i < sMapDifficultyStore.GetNumRows(); ++i)
        if (MapDifficultyEntry const* entry = sMapDifficultyStore.LookupEntry(i))
        {
            sMapDifficultyMap[MAKE_PAIR32(entry->MapId, entry->Difficulty)] = entry;
        }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sMovieStore,               dbcPath, "Movie.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files, sMountCapabilityStore,     dbcPath,"MountCapability.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files, sMountTypeStore,           dbcPath,"MountType.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sNumTalentsAtLevelStore,   dbcPath,"NumTalentsAtLevel.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sOverrideSpellDataStore,   dbcPath, "OverrideSpellData.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sQuestFactionRewardStore,  dbcPath, "QuestFactionReward.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sQuestSortStore,           dbcPath, "QuestSort.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sQuestXPLevelStore,        dbcPath, "QuestXP.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sPhaseStore,               dbcPath,"Phase.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sPowerDisplayStore,        dbcPath,"PowerDisplay.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sPvPDifficultyStore,       dbcPath, "PvpDifficulty.dbc");
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
            if (entry->bracketId > MAX_BATTLEGROUND_BRACKETS)
            {
                MANGOS_ASSERT(false && "Need update MAX_BATTLEGROUND_BRACKETS by DBC data");
            }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sRandomPropertiesPointsStore, dbcPath, "RandPropPoints.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sScalingStatDistributionStore, dbcPath, "ScalingStatDistribution.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sScalingStatValuesStore,   dbcPath, "ScalingStatValues.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillLineStore,           dbcPath, "SkillLine.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillLineAbilityStore,    dbcPath, "SkillLineAbility.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSkillRaceClassInfoStore,  dbcPath, "SkillRaceClassInfo.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSoundEntriesStore,        dbcPath, "SoundEntries.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellStore,               dbcPath, "Spell.dbc");

    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellAuraOptionsStore,    dbcPath,"SpellAuraOptions.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellAuraRestrictionsStore, dbcPath,"SpellAuraRestrictions.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellCastingRequirementsStore, dbcPath,"SpellCastingRequirements.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellCategoriesStore,     dbcPath,"SpellCategories.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellClassOptionsStore,   dbcPath,"SpellClassOptions.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellCooldownsStore,      dbcPath,"SpellCooldowns.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellEffectStore,         dbcPath,"SpellEffect.dbc");

    for (uint32 i = 1; i < sSpellStore.GetNumRows(); ++i)
    {
        if (SpellEntry const * spell = sSpellStore.LookupEntry(i))
        {
            if (SpellCategoriesEntry const* category = spell->GetSpellCategories())
                if (uint32 cat = category->Category)
                {
                    sSpellCategoryStore[cat].insert(i);
                }

            // DBC not support uint64 fields but SpellEntry have SpellFamilyFlags mapped at 2 uint32 fields
            // uint32 field already converted to bigendian if need, but must be swapped for correct uint64 bigendian view
            #if MANGOS_ENDIAN == MANGOS_BIGENDIAN
            std::swap(*((uint32*)(&spell->SpellFamilyFlags)),*(((uint32*)(&spell->SpellFamilyFlags))+1));
            #endif
        }
    }

    for(uint32 i = 1; i < sSpellEffectStore.GetNumRows(); ++i)
    {
        if (SpellEffectEntry const *spellEffect = sSpellEffectStore.LookupEntry(i))
        {
            switch (spellEffect->EffectApplyAuraName)
            {
                case SPELL_AURA_MOD_INCREASE_ENERGY:
                case SPELL_AURA_MOD_INCREASE_ENERGY_PERCENT:
                case SPELL_AURA_PERIODIC_MANA_LEECH:
                case SPELL_AURA_PERIODIC_ENERGIZE:
                case SPELL_AURA_POWER_BURN_MANA:
                    MANGOS_ASSERT(spellEffect->EffectMiscValue >= 0 && spellEffect->EffectMiscValue < MAX_POWERS);
                    break;
            }

            sSpellEffectMap[spellEffect->EffectSpellId].effects[spellEffect->EffectIndex] = spellEffect;
        }
    }

    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellEquippedItemsStore,  dbcPath,"SpellEquippedItems.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellInterruptsStore,     dbcPath,"SpellInterrupts.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellLevelsStore,         dbcPath,"SpellLevels.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellPowerStore,          dbcPath,"SpellPower.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellReagentsStore,       dbcPath,"SpellReagents.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellScalingStore,        dbcPath,"SpellScaling.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellShapeshiftStore,     dbcPath,"SpellShapeshift.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellTargetRestrictionsStore, dbcPath,"SpellTargetRestrictions.dbc");
    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sSpellTotemsStore,         dbcPath,"SpellTotems.dbc");

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);

        if (!skillLine)
        {
            continue;
        }

        SpellEntry const* spellInfo = sSpellStore.LookupEntry(skillLine->spellId);
        if (spellInfo && (spellInfo->Attributes & (SPELL_ATTR_ABILITY | SPELL_ATTR_PASSIVE | SPELL_ATTR_UNK7 | SPELL_ATTR_UNK8)) == (SPELL_ATTR_ABILITY | SPELL_ATTR_PASSIVE | SPELL_ATTR_UNK7 | SPELL_ATTR_UNK8))
        {
            for (unsigned int i = 1; i < sCreatureFamilyStore.GetNumRows(); ++i)
            {
                CreatureFamilyEntry const* cFamily = sCreatureFamilyStore.LookupEntry(i);
                if (!cFamily)
                {
                    continue;
                }

                if (skillLine->skillId != cFamily->skillLine[0] && skillLine->skillId != cFamily->skillLine[1])
                {
                    continue;
                }

                sPetFamilySpellsStore[i].insert(spellInfo->Id);
            }
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellCastTimesStore,      dbcPath, "SpellCastTimes.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellDurationStore,       dbcPath, "SpellDuration.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellDifficultyStore,     dbcPath, "SpellDifficulty.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellFocusObjectStore,    dbcPath, "SpellFocusObject.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellItemEnchantmentStore, dbcPath, "SpellItemEnchantment.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellItemEnchantmentConditionStore, dbcPath, "SpellItemEnchantmentCondition.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellRadiusStore,         dbcPath, "SpellRadius.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellRangeStore,          dbcPath, "SpellRange.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellRuneCostStore,       dbcPath, "SpellRuneCost.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSpellShapeshiftFormStore, dbcPath, "SpellShapeshiftForm.dbc");
    //LoadDBC(availableDbcLocales,bar,bad_dbc_files,sStableSlotPricesStore,    dbcPath,"StableSlotPrices.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sSummonPropertiesStore,    dbcPath, "SummonProperties.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTalentStore,              dbcPath, "Talent.dbc");

    // create talent spells set
    for (unsigned int i = 0; i < sTalentStore.GetNumRows(); ++i)
    {
        TalentEntry const* talentInfo = sTalentStore.LookupEntry(i);
        if (!talentInfo) continue;
        for (int j = 0; j < MAX_TALENT_RANK; j++)
            if (talentInfo->RankID[j])
            {
                sTalentSpellPosMap[talentInfo->RankID[j]] = TalentSpellPos(i, j);
            }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTalentTabStore,           dbcPath, "TalentTab.dbc");

    // prepare fast data access to bit pos of talent ranks for use at inspecting
    {
        // now have all max ranks (and then bit amount used for store talent ranks in inspect)
        for (uint32 talentTabId = 1; talentTabId < sTalentTabStore.GetNumRows(); ++talentTabId)
        {
            TalentTabEntry const* talentTabInfo = sTalentTabStore.LookupEntry(talentTabId);
            if (!talentTabInfo)
            {
                continue;
            }

            for (uint32 i = 0; i < MAX_MASTERY_SPELLS; ++i)
                if (uint32 spellid = talentTabInfo->masterySpells[i])
                    if (sSpellStore.LookupEntry(spellid))
                    {
                        sTalentTreeMasterySpellsMap[talentTabId].push_back(spellid);
                    }

            // prevent memory corruption; otherwise cls will become 12 below
            if ((talentTabInfo->ClassMask & CLASSMASK_ALL_PLAYABLE) == 0)
            {
                continue;
            }

            // store class talent tab pages
            for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
                if (talentTabInfo->ClassMask & (1 << (cls - 1)))
                {
                    sTalentTabPages[cls][talentTabInfo->tabpage] = talentTabId;
                }

            sTalentTreeRolesMap[talentTabId] = talentTabInfo->rolesMask;
        }
    }

    LoadDBC(availableDbcLocales,bar,bad_dbc_files, sTalentTreePrimarySpellsStore, dbcPath, "TalentTreePrimarySpells.dbc");
    for (uint32 i = 0; i < sTalentTreePrimarySpellsStore.GetNumRows(); ++i)
        if (TalentTreePrimarySpellsEntry const* talentSpell = sTalentTreePrimarySpellsStore.LookupEntry(i))
            if (sSpellStore.LookupEntry(talentSpell->SpellId))
            {
                sTalentTreePrimarySpellsMap[talentSpell->TalentTree].push_back(talentSpell->SpellId);
            }
    sTalentTreePrimarySpellsStore.Clear();

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTaxiNodesStore,           dbcPath, "TaxiNodes.dbc");

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTaxiPathStore,            dbcPath, "TaxiPath.dbc");
    for (uint32 i = 1; i < sTaxiPathStore.GetNumRows(); ++i)
        if (TaxiPathEntry const* entry = sTaxiPathStore.LookupEntry(i))
        {
            sTaxiPathSetBySource[entry->from][entry->to] = TaxiPathBySourceAndDestination(entry->ID, entry->price);
        }
    uint32 pathCount = sTaxiPathStore.GetNumRows();

    //## TaxiPathNode.dbc ## Loaded only for initialization different structures
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTaxiPathNodeStore,        dbcPath, "TaxiPathNode.dbc");
    // Calculate path nodes count
    std::vector<uint32> pathLength;
    pathLength.resize(pathCount);                           // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
        {
            if (pathLength[entry->path] < entry->index + 1)
            {
                pathLength[entry->path] = entry->index + 1;
            }
        }
    // Set path length
    sTaxiPathNodesByPath.resize(pathCount);                 // 0 and some other indexes not used
    for (uint32 i = 1; i < sTaxiPathNodesByPath.size(); ++i)
    {
        sTaxiPathNodesByPath[i].resize(pathLength[i]);
    }
    // fill data (pointers to sTaxiPathNodeStore elements
    for (uint32 i = 1; i < sTaxiPathNodeStore.GetNumRows(); ++i)
        if (TaxiPathNodeEntry const* entry = sTaxiPathNodeStore.LookupEntry(i))
        {
            sTaxiPathNodesByPath[entry->path].set(entry->index, entry);
        }

    // Initialize global taxinodes mask
    // include existing nodes that have at least single not spell base (scripted) path
    {
        std::set<uint32> spellPaths;
        for (uint32 i = 1; i < sSpellStore.GetNumRows(); ++i)
            if (SpellEntry const* sInfo = sSpellStore.LookupEntry(i))
                for (int j = 0; j < MAX_EFFECT_INDEX; ++j)
                    if (SpellEffectEntry const* effect = sInfo->GetSpellEffect(SpellEffectIndex(j)))
                        if (effect->Effect==123 /*SPELL_EFFECT_SEND_TAXI*/)
                        {
                            spellPaths.insert(effect->EffectMiscValue);
                        }

        memset(sTaxiNodesMask, 0, sizeof(sTaxiNodesMask));
        memset(sOldContinentsNodesMask, 0, sizeof(sTaxiNodesMask));
        memset(sHordeTaxiNodesMask, 0, sizeof(sHordeTaxiNodesMask));
        memset(sAllianceTaxiNodesMask, 0, sizeof(sAllianceTaxiNodesMask));
        memset(sDeathKnightTaxiNodesMask, 0, sizeof(sDeathKnightTaxiNodesMask));
        for (uint32 i = 1; i < sTaxiNodesStore.GetNumRows(); ++i)
        {
            TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(i);
            if (!node)
            {
                continue;
            }

            TaxiPathSetBySource::const_iterator src_i = sTaxiPathSetBySource.find(i);
            if (src_i != sTaxiPathSetBySource.end() && !src_i->second.empty())
            {
                bool ok = false;
                for (TaxiPathSetForSource::const_iterator dest_i = src_i->second.begin(); dest_i != src_i->second.end(); ++dest_i)
                {
                    // not spell path
                    if (spellPaths.find(dest_i->second.ID) == spellPaths.end())
                    {
                        ok = true;
                        break;
                    }
                }

                if (!ok)
                {
                    continue;
                }
            }

            // valid taxi network node
            uint8  field   = (uint8)((i - 1) / 8);
            uint32 submask = 1 << ((i-1) % 8);
            sTaxiNodesMask[field] |= submask;

            if (node->MountCreatureID[0] && node->MountCreatureID[0] != 32981)
            {
                sHordeTaxiNodesMask[field] |= submask;
            }
            if (node->MountCreatureID[1] && node->MountCreatureID[1] != 32981)
            {
                sAllianceTaxiNodesMask[field] |= submask;
            }
            if (node->MountCreatureID[0] == 32981 || node->MountCreatureID[1] == 32981)
            {
                sDeathKnightTaxiNodesMask[field] |= submask;
            }

            // old continent node (+ nodes virtually at old continents, check explicitly to avoid loading map files for zone info)
            if (node->map_id < 2 || i == 82 || i == 83 || i == 93 || i == 94)
            {
                sOldContinentsNodesMask[field] |= submask;
            }

            // fix DK node at Ebon Hold
            if (i == 315)
            {
                (const_cast<TaxiNodesEntry*>(node))->MountCreatureID[1] = node->MountCreatureID[0];
            }
        }
    }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sTotemCategoryStore,       dbcPath, "TotemCategory.dbc");

    LoadDBC(availableDbcLocales,bar,bad_dbc_files,sTransportAnimationStore,  dbcPath,"TransportAnimation.dbc");
    for (uint32 i = 0; i < sTransportAnimationStore.GetNumRows(); ++i)
        if (TransportAnimationEntry const* entry = sTransportAnimationStore.LookupEntry(i))
        {
            sTransportAnimationsByEntry[entry->transportEntry][entry->timeFrame] = entry;
        }

    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sVehicleStore,             dbcPath, "Vehicle.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sVehicleSeatStore,         dbcPath, "VehicleSeat.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWorldMapAreaStore,        dbcPath, "WorldMapArea.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWMOAreaTableStore,        dbcPath, "WMOAreaTable.dbc");
    for (uint32 i = 0; i < sWMOAreaTableStore.GetNumRows(); ++i)
    {
        if (WMOAreaTableEntry const* entry = sWMOAreaTableStore.LookupEntry(i))
        {
            sWMOAreaInfoByTripple.insert(WMOAreaInfoByTripple::value_type(WMOAreaTableTripple(entry->rootId, entry->adtId, entry->groupId), entry));
        }
    }
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWorldMapOverlayStore,     dbcPath, "WorldMapOverlay.dbc");
    LoadDBC(availableDbcLocales, bar, bad_dbc_files, sWorldSafeLocsStore,       dbcPath, "WorldSafeLocs.dbc");

    // error checks
    if (bad_dbc_files.size() >= DBCFilesCount)
    {
        sLog.outError("\nIncorrect DataDir value in mangosd.conf or ALL required *.dbc files (%d) not found by path: %sdbc", DBCFilesCount, dataPath.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }
    else if (!bad_dbc_files.empty())
    {
        std::string str;
        for (std::list<std::string>::iterator i = bad_dbc_files.begin(); i != bad_dbc_files.end(); ++i)
        {
            str += *i + "\n";
        }

        sLog.outError("\nSome required *.dbc files (%u from %d) not found or not compatible:\n%s", (uint32)bad_dbc_files.size(), DBCFilesCount, str.c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    // Check loaded DBC files proper version
    if (!sAreaStore.LookupEntry(4713)              ||       // last area (areaflag) added in 4.3.4
        !sCharTitlesStore.LookupEntry(287)         ||       // last char title added in 4.3.4
        !sGemPropertiesStore.LookupEntry(2250)     ||       // last gem property added in 4.3.4
        !sMapStore.LookupEntry(980)                ||       // last map added in 4.3.4
        !sSpellStore.LookupEntry(121820)           )        // last added spell in 4.3.4
    {
        sLog.outError("\nYou have mixed version DBC files. Please re-extract DBC files for one from client build: %s", AcceptableClientBuildsListStr().c_str());
        Log::WaitBeforeContinueIfNeed();
        exit(1);
    }

    sLog.outString();
    sLog.outString(">> Initialized %d data stores", DBCFilesCount);
}

SimpleFactionsList const* GetFactionTeamList(uint32 faction)
{
    FactionTeamMap::const_iterator itr = sFactionTeamMap.find(faction);
    if (itr == sFactionTeamMap.end())
    {
        return NULL;
    }
    return &itr->second;
}

char const* GetPetName(uint32 petfamily, uint32 dbclang)
{
    if (!petfamily)
    {
        return NULL;
    }
    CreatureFamilyEntry const* pet_family = sCreatureFamilyStore.LookupEntry(petfamily);
    if (!pet_family)
    {
        return NULL;
    }
    return pet_family->Name[dbclang] ? pet_family->Name[dbclang] : NULL;
}

TalentSpellPos const* GetTalentSpellPos(uint32 spellId)
{
    TalentSpellPosMap::const_iterator itr = sTalentSpellPosMap.find(spellId);
    if (itr == sTalentSpellPosMap.end())
    {
        return NULL;
    }

    return &itr->second;
}

SpellEffectEntry const* GetSpellEffectEntry(uint32 spellId, SpellEffectIndex effect)
{
    SpellEffectMap::const_iterator itr = sSpellEffectMap.find(spellId);
    if (itr == sSpellEffectMap.end())
    {
        return NULL;
    }

    return itr->second.effects[effect];
}

uint32 GetTalentSpellCost(TalentSpellPos const* pos)
{
    if (pos)
    {
        return pos->rank + 1;
    }

    return 0;
}

uint32 GetTalentSpellCost(uint32 spellId)
{
    return GetTalentSpellCost(GetTalentSpellPos(spellId));
}

int32 GetAreaFlagByAreaID(uint32 area_id)
{
    AreaFlagByAreaID::iterator i = sAreaFlagByAreaID.find(area_id);
    if (i == sAreaFlagByAreaID.end())
    {
        return -1;
    }

    return i->second;
}

WMOAreaTableEntry const* GetWMOAreaTableEntryByTripple(int32 rootid, int32 adtid, int32 groupid)
{
    WMOAreaInfoByTripple::iterator i = sWMOAreaInfoByTripple.find(WMOAreaTableTripple(rootid, adtid, groupid));
    if (i == sWMOAreaInfoByTripple.end())
    {
        return NULL;
    }
    return i->second;
}

AreaTableEntry const* GetAreaEntryByAreaID(uint32 area_id)
{
    int32 areaflag = GetAreaFlagByAreaID(area_id);
    if (areaflag < 0)
    {
        return NULL;
    }

    return sAreaStore.LookupEntry(areaflag);
}

AreaTableEntry const* GetAreaEntryByAreaFlagAndMap(uint32 area_flag, uint32 map_id)
{
    if (area_flag)
    {
        return sAreaStore.LookupEntry(area_flag);
    }

    if (MapEntry const* mapEntry = sMapStore.LookupEntry(map_id))
    {
        return GetAreaEntryByAreaID(mapEntry->linked_zone);
    }

    return NULL;
}

uint32 GetAreaFlagByMapId(uint32 mapid)
{
    AreaFlagByMapID::iterator i = sAreaFlagByMapID.find(mapid);
    if (i == sAreaFlagByMapID.end())
    {
        return 0;
    }
    else
    {
        return i->second;
    }
}

uint32 GetVirtualMapForMapAndZone(uint32 mapid, uint32 zoneId)
{
    if (mapid != 530 && mapid != 571 && mapid != 732)           // speed for most cases
    {
        return mapid;
    }

    if (WorldMapAreaEntry const* wma = sWorldMapAreaStore.LookupEntry(zoneId))
    {
        return wma->virtual_map_id >= 0 ? wma->virtual_map_id : wma->map_id;
    }

    return mapid;
}

ContentLevels GetContentLevelsForMap(uint32 mapid)
{
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapid);
    if (!mapEntry)
    {
        return CONTENT_1_60;
    }

    // exceptions for 648 - Goblin Starter area and 654 - Worgen Starter area
    if (mapid == 648 || mapid == 654)
    {
        return CONTENT_1_60;
    }

    switch (mapEntry->Expansion())
    {
        default: return CONTENT_1_60;
        case 1:  return CONTENT_61_70;
        case 2:  return CONTENT_71_80;
        case 3:  return CONTENT_81_85;
    }
}

ChatChannelsEntry const* GetChannelEntryFor(uint32 channel_id)
{
    // not sorted, numbering index from 0
    for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
    {
        ChatChannelsEntry const* ch = sChatChannelsStore.LookupEntry(i);
        if (ch && ch->ChannelID == channel_id)
        {
            return ch;
        }
    }
    return NULL;
}

bool IsTotemCategoryCompatiableWith(uint32 itemTotemCategoryId, uint32 requiredTotemCategoryId)
{
    if (requiredTotemCategoryId == 0)
    {
        return true;
    }
    if (itemTotemCategoryId == 0)
    {
        return false;
    }

    TotemCategoryEntry const* itemEntry = sTotemCategoryStore.LookupEntry(itemTotemCategoryId);
    if (!itemEntry)
    {
        return false;
    }
    TotemCategoryEntry const* reqEntry = sTotemCategoryStore.LookupEntry(requiredTotemCategoryId);
    if (!reqEntry)
    {
        return false;
    }

    if (itemEntry->categoryType != reqEntry->categoryType)
    {
        return false;
    }

    return (itemEntry->categoryMask & reqEntry->categoryMask) == reqEntry->categoryMask;
}

bool Zone2MapCoordinates(float& x, float& y, uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry || maEntry->x2 == maEntry->x1 || maEntry->y2 == maEntry->y1)
    {
        return false;
    }

    std::swap(x, y);                                        // at client map coords swapped
    x = x * ((maEntry->x2 - maEntry->x1) / 100) + maEntry->x1;
    y = y * ((maEntry->y2 - maEntry->y1) / 100) + maEntry->y1; // client y coord from top to down

    return true;
}

bool Map2ZoneCoordinates(float& x, float& y, uint32 zone)
{
    WorldMapAreaEntry const* maEntry = sWorldMapAreaStore.LookupEntry(zone);

    // if not listed then map coordinates (instance)
    if (!maEntry || maEntry->x2 == maEntry->x1 || maEntry->y2 == maEntry->y1)
    {
        return false;
    }

    x = (x - maEntry->x1) / ((maEntry->x2 - maEntry->x1) / 100);
    y = (y - maEntry->y1) / ((maEntry->y2 - maEntry->y1) / 100); // client y coord from top to down
    std::swap(x, y);                                        // client have map coords swapped

    return true;
}

ContentLevels GetContentLevelsForMapAndZone(uint32 mapId, uint32 zoneId)
{
    MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
    if (!mapEntry)
    {
        return CONTENT_1_60;
    }

    if (mapEntry->rootPhaseMap != -1)
    {
        mapId = mapEntry->rootPhaseMap;
    }

    switch (mapId)
    {
        case 648:   // Lost Islands
        case 654:   // Gilneas
            return CONTENT_1_60;
        default:
            break;
    }

    switch (zoneId)
    {
        case 616:   // Mount Hyjal
        case 4922:  // Twilight Highlands
        case 5034:  // Uldum
        case 5042:  // Deepholm
            return CONTENT_81_85;
        default:
            break;
    }

    switch (mapEntry->Expansion())
    {
        default: return CONTENT_1_60;
        case 1:  return CONTENT_61_70;
        case 2:  return CONTENT_71_80;
        case 3:  return CONTENT_81_85;
    }
}

MapDifficultyEntry const* GetMapDifficultyData(uint32 mapId, Difficulty difficulty)
{
    MapDifficultyMap::const_iterator itr = sMapDifficultyMap.find(MAKE_PAIR32(mapId, difficulty));
    return itr != sMapDifficultyMap.end() ? itr->second : NULL;
}

PvPDifficultyEntry const* GetBattlegroundBracketByLevel(uint32 mapid, uint32 level)
{
    PvPDifficultyEntry const* maxEntry = NULL;              // used for level > max listed level case
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
    {
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
        {
            // skip unrelated and too-high brackets
            if (entry->mapId != mapid || entry->minLevel > level)
            {
                continue;
            }

            // exactly fit
            if (entry->maxLevel >= level)
            {
                return entry;
            }

            // remember for possible out-of-range case (search higher from existed)
            if (!maxEntry || maxEntry->maxLevel < entry->maxLevel)
            {
                maxEntry = entry;
            }
        }
    }

    return maxEntry;
}

PvPDifficultyEntry const* GetBattlegroundBracketById(uint32 mapid, BattleGroundBracketId id)
{
    for (uint32 i = 0; i < sPvPDifficultyStore.GetNumRows(); ++i)
        if (PvPDifficultyEntry const* entry = sPvPDifficultyStore.LookupEntry(i))
            if (entry->mapId == mapid && entry->GetBracketId() == id)
            {
                return entry;
            }

    return NULL;
}

uint32 const* GetTalentTabPages(uint32 cls)
{
    return sTalentTabPages[cls];
}

std::vector<uint32> const* GetTalentTreeMasterySpells(uint32 talentTree)
{
    TalentTreeSpellsMap::const_iterator itr = sTalentTreeMasterySpellsMap.find(talentTree);
    if (itr == sTalentTreeMasterySpellsMap.end())
    {
        return NULL;
    }

    return &itr->second;
}

std::vector<uint32> const* GetTalentTreePrimarySpells(uint32 talentTree)
{
    TalentTreeSpellsMap::const_iterator itr = sTalentTreePrimarySpellsMap.find(talentTree);
    if (itr == sTalentTreePrimarySpellsMap.end())
    {
        return NULL;
    }

    return &itr->second;
}

uint32 GetTalentTreeRolesMask(uint32 talentTree)
{
    TalentTreeRolesMap::const_iterator itr = sTalentTreeRolesMap.find(talentTree);
    if (itr == sTalentTreeRolesMap.end())
    {
        return 0;
    }

    return itr->second;
}

bool IsPointInAreaTriggerZone(AreaTriggerEntry const* atEntry, uint32 mapid, float x, float y, float z, float delta)
{
    if (mapid != atEntry->mapid)
    {
        return false;
    }

    if (atEntry->radius > 0)
    {
        // if we have radius check it
        float dist2 = (x - atEntry->x) * (x - atEntry->x) + (y - atEntry->y) * (y - atEntry->y) + (z - atEntry->z) * (z - atEntry->z);
        if (dist2 > (atEntry->radius + delta) * (atEntry->radius + delta))
        {
            return false;
        }
    }
    else
    {
        // we have only extent

        // rotate the players position instead of rotating the whole cube, that way we can make a simplified
        // is-in-cube check and we have to calculate only one point instead of 4

        // 2PI = 360, keep in mind that ingame orientation is counter-clockwise
        double rotation = 2 * M_PI - atEntry->box_orientation;
        double sinVal = sin(rotation);
        double cosVal = cos(rotation);

        float playerBoxDistX = x - atEntry->x;
        float playerBoxDistY = y - atEntry->y;

        float rotPlayerX = float(atEntry->x + playerBoxDistX * cosVal - playerBoxDistY * sinVal);
        float rotPlayerY = float(atEntry->y + playerBoxDistY * cosVal + playerBoxDistX * sinVal);

        // box edges are parallel to coordiante axis, so we can treat every dimension independently :D
        float dz = z - atEntry->z;
        float dx = rotPlayerX - atEntry->x;
        float dy = rotPlayerY - atEntry->y;
        if ((fabs(dx) > atEntry->box_x / 2 + delta) ||
                (fabs(dy) > atEntry->box_y / 2 + delta) ||
                (fabs(dz) > atEntry->box_z / 2 + delta))
        {
            return false;
        }
    }

    return true;
}

uint32 GetCreatureModelRace(uint32 model_id)
{
    CreatureDisplayInfoEntry const* displayEntry = sCreatureDisplayInfoStore.LookupEntry(model_id);
    if (!displayEntry)
    {
        return 0;
    }
    CreatureDisplayInfoExtraEntry const* extraEntry = sCreatureDisplayInfoExtraStore.LookupEntry(displayEntry->ExtendedDisplayInfoID);
    return extraEntry ? extraEntry->Race : 0;
}

float GetCurrencyPrecision(uint32 currencyId)
{
    CurrencyTypesEntry const * entry = sCurrencyTypesStore.LookupEntry(currencyId);

    return entry ? entry->GetPrecision() : 1.0f;
}

// script support functions
 DBCStorage <SoundEntriesEntry>  const* GetSoundEntriesStore()   { return &sSoundEntriesStore;   }
 DBCStorage <SpellEntry>         const* GetSpellStore()          { return &sSpellStore;          }
 DBCStorage <SpellRangeEntry>    const* GetSpellRangeStore()     { return &sSpellRangeStore;     }
 DBCStorage <FactionEntry>       const* GetFactionStore()        { return &sFactionStore;        }
 DBCStorage <CreatureDisplayInfoEntry> const* GetCreatureDisplayStore() { return &sCreatureDisplayInfoStore; }
 DBCStorage <EmotesEntry>        const* GetEmotesStore()         { return &sEmotesStore;         }
 DBCStorage <EmotesTextEntry>    const* GetEmotesTextStore()     { return &sEmotesTextStore;     }
