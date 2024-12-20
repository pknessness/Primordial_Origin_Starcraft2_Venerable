#pragma once

#include <sc2api/sc2_api.h>
#include <sc2api/sc2_interfaces.h>

using namespace sc2;

typedef struct {
    unsigned int minerals = 0;
    unsigned int vespene = 0;
    unsigned int energy = 0;
} Cost;

template <typename... Args>
std::string strprintf(const std::string &format, Args... args) {
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1;  // Extra space for '\0'
    if (size_s <= 0) {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1);  // We don't want the '\0' inside
}

namespace Aux {
static bool isPylon(const Unit &unit) {
    UnitTypeID type = unit.unit_type;
    return (type == UNIT_TYPEID::PROTOSS_PYLON);
}

static bool isNexus(const Unit &unit) {
    UnitTypeID type = unit.unit_type;
    return (type == UNIT_TYPEID::PROTOSS_NEXUS);
}

static UnitTypeID buildAbilityToUnit(AbilityID build_ability) {
    switch (uint32_t(build_ability)) {
        case (uint32_t(ABILITY_ID::BUILD_ASSIMILATOR)):
            return UNIT_TYPEID::PROTOSS_ASSIMILATOR;
        case (uint32_t(ABILITY_ID::BUILD_CYBERNETICSCORE)):
            return UNIT_TYPEID::PROTOSS_CYBERNETICSCORE;
        case (uint32_t(ABILITY_ID::BUILD_DARKSHRINE)):
            return UNIT_TYPEID::PROTOSS_DARKSHRINE;
        case (uint32_t(ABILITY_ID::BUILD_FLEETBEACON)):
            return UNIT_TYPEID::PROTOSS_FLEETBEACON;
        case (uint32_t(ABILITY_ID::BUILD_FORGE)):
            return UNIT_TYPEID::PROTOSS_FORGE;
        case (uint32_t(ABILITY_ID::BUILD_GATEWAY)):
            return UNIT_TYPEID::PROTOSS_GATEWAY;
        case (uint32_t(ABILITY_ID::BUILD_NEXUS)):
            return UNIT_TYPEID::PROTOSS_NEXUS;
        case (uint32_t(ABILITY_ID::BUILD_PHOTONCANNON)):
            return UNIT_TYPEID::PROTOSS_PHOTONCANNON;
        case (uint32_t(ABILITY_ID::BUILD_PYLON)):
            return UNIT_TYPEID::PROTOSS_PYLON;
        case (uint32_t(ABILITY_ID::BUILD_ROBOTICSBAY)):
            return UNIT_TYPEID::PROTOSS_ROBOTICSBAY;
        case (uint32_t(ABILITY_ID::BUILD_ROBOTICSFACILITY)):
            return UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY;
        case (uint32_t(ABILITY_ID::BUILD_SHIELDBATTERY)):
            return UNIT_TYPEID::PROTOSS_SHIELDBATTERY;
        case (uint32_t(ABILITY_ID::BUILD_STARGATE)):
            return UNIT_TYPEID::PROTOSS_STARGATE;
        case (uint32_t(ABILITY_ID::BUILD_TEMPLARARCHIVE)):
            return UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE;
        case (uint32_t(ABILITY_ID::BUILD_TWILIGHTCOUNCIL)):
            return UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL;
        case(uint32_t(ABILITY_ID::TRAINWARP_ADEPT)):
	        return UNIT_TYPEID::PROTOSS_ADEPT;
        case(uint32_t(ABILITY_ID::TRAINWARP_DARKTEMPLAR)):
	        return UNIT_TYPEID::PROTOSS_DARKTEMPLAR;
        case(uint32_t(ABILITY_ID::TRAINWARP_HIGHTEMPLAR)):
            return UNIT_TYPEID::PROTOSS_HIGHTEMPLAR;
        case(uint32_t(ABILITY_ID::TRAINWARP_SENTRY)):
            return UNIT_TYPEID::PROTOSS_SENTRY;
        case(uint32_t(ABILITY_ID::TRAINWARP_STALKER)):
            return UNIT_TYPEID::PROTOSS_STALKER;
        case(uint32_t(ABILITY_ID::TRAINWARP_ZEALOT)):
            return UNIT_TYPEID::PROTOSS_ZEALOT;
        case(uint32_t(ABILITY_ID::TRAIN_ADEPT)):
            return UNIT_TYPEID::PROTOSS_ADEPT;
        case(uint32_t(ABILITY_ID::TRAIN_ARCHON)):
            return UNIT_TYPEID::PROTOSS_ARCHON;
        case(uint32_t(ABILITY_ID::TRAIN_CARRIER)):
            return UNIT_TYPEID::PROTOSS_CARRIER;
        case(uint32_t(ABILITY_ID::TRAIN_COLOSSUS)):
            return UNIT_TYPEID::PROTOSS_COLOSSUS;
        case(uint32_t(ABILITY_ID::TRAIN_DARKTEMPLAR)):
            return UNIT_TYPEID::PROTOSS_DARKTEMPLAR;
        case(uint32_t(ABILITY_ID::TRAIN_DISRUPTOR)):
            return UNIT_TYPEID::PROTOSS_DISRUPTOR;
        case(uint32_t(ABILITY_ID::TRAIN_HIGHTEMPLAR)):
            return UNIT_TYPEID::PROTOSS_HIGHTEMPLAR;
        case(uint32_t(ABILITY_ID::TRAIN_IMMORTAL)):
            return UNIT_TYPEID::PROTOSS_IMMORTAL;
        case(uint32_t(ABILITY_ID::TRAIN_MOTHERSHIP)):
            return UNIT_TYPEID::PROTOSS_MOTHERSHIP;
        case(uint32_t(ABILITY_ID::TRAIN_MOTHERSHIPCORE)):
            return UNIT_TYPEID::PROTOSS_MOTHERSHIPCORE;
        case(uint32_t(ABILITY_ID::TRAIN_OBSERVER)):
            return UNIT_TYPEID::PROTOSS_OBSERVER;
        case(uint32_t(ABILITY_ID::TRAIN_ORACLE)):
            return UNIT_TYPEID::PROTOSS_ORACLE;
        case(uint32_t(ABILITY_ID::TRAIN_PHOENIX)):
            return UNIT_TYPEID::PROTOSS_PHOENIX;
        case(uint32_t(ABILITY_ID::TRAIN_PROBE)):
            return UNIT_TYPEID::PROTOSS_PROBE;
        case(uint32_t(ABILITY_ID::TRAIN_SENTRY)):
            return UNIT_TYPEID::PROTOSS_SENTRY;
        case(uint32_t(ABILITY_ID::TRAIN_STALKER)):
            return UNIT_TYPEID::PROTOSS_STALKER;
        case(uint32_t(ABILITY_ID::TRAIN_TEMPEST)):
            return UNIT_TYPEID::PROTOSS_TEMPEST;
        case(uint32_t(ABILITY_ID::TRAIN_VOIDRAY)):
            return UNIT_TYPEID::PROTOSS_VOIDRAY;
        case(uint32_t(ABILITY_ID::TRAIN_WARPPRISM)):
            return UNIT_TYPEID::PROTOSS_WARPPRISM;
        case(uint32_t(ABILITY_ID::TRAIN_ZEALOT)):
            return UNIT_TYPEID::PROTOSS_ZEALOT;
        default:
            return 0;
    }
    return 0;
}

static UpgradeID researchAbilityToUpgrade(AbilityID build_ability) {
    switch (uint32_t(build_ability)) {
        case (uint32_t(ABILITY_ID::RESEARCH_ADEPTRESONATINGGLAIVES)):
            return UPGRADE_ID::ADEPTKILLBOUNCE;
        case(uint32_t(ABILITY_ID::RESEARCH_BLINK)):
            return UPGRADE_ID::BLINKTECH;
        case(uint32_t(ABILITY_ID::RESEARCH_CHARGE)):
            return UPGRADE_ID::CHARGE;
        case(uint32_t(ABILITY_ID::RESEARCH_EXTENDEDTHERMALLANCE)):
            return UPGRADE_ID::EXTENDEDTHERMALLANCE;
        case(uint32_t(ABILITY_ID::RESEARCH_GRAVITICBOOSTER)):
            return UPGRADE_ID::OBSERVERGRAVITICBOOSTER;
        case(uint32_t(ABILITY_ID::RESEARCH_GRAVITICDRIVE)):
            return UPGRADE_ID::GRAVITICDRIVE;
        case (uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMOR)):
            return UPGRADE_ID::INVALID;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL1)):
            return UPGRADE_ID::PROTOSSAIRARMORSLEVEL1;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL2)):
            return UPGRADE_ID::PROTOSSAIRARMORSLEVEL2;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRARMORLEVEL3)):
            return UPGRADE_ID::PROTOSSAIRARMORSLEVEL3;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONS)):
            return UPGRADE_ID::INVALID;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL1)):
            return UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL1;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL2)):
            return UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL2;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSAIRWEAPONSLEVEL3)):
            return UPGRADE_ID::PROTOSSAIRWEAPONSLEVEL3;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMOR)):
            return UPGRADE_ID::INVALID;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL1)):
            return UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL1;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL2)):
            return UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL2;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDARMORLEVEL3)):
            return UPGRADE_ID::PROTOSSGROUNDARMORSLEVEL3;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONS)):
            return UPGRADE_ID::INVALID;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL1)):
            return UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL1;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL2)):
            return UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL2;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSGROUNDWEAPONSLEVEL3)):
            return UPGRADE_ID::PROTOSSGROUNDWEAPONSLEVEL3;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDS)):
            return UPGRADE_ID::INVALID;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL1)):
            return UPGRADE_ID::PROTOSSSHIELDSLEVEL1;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL2)):
            return UPGRADE_ID::PROTOSSSHIELDSLEVEL2;
        case(uint32_t(ABILITY_ID::RESEARCH_PROTOSSSHIELDSLEVEL3)):
            return UPGRADE_ID::PROTOSSSHIELDSLEVEL3;
        case(uint32_t(ABILITY_ID::RESEARCH_PSIONICAMPLIFIERS)):
            return UPGRADE_ID::PSIONICAMPLIFIERS;
        case(uint32_t(ABILITY_ID::RESEARCH_PSISTORM)):
            return UPGRADE_ID::PSISTORMTECH;
        case(uint32_t(ABILITY_ID::RESEARCH_SHADOWSTRIKE)):
            return UPGRADE_ID::DARKTEMPLARBLINKUPGRADE;
        case(uint32_t(ABILITY_ID::RESEARCH_TEMPESTRANGEUPGRADE)):
            return UPGRADE_ID::TEMPESTRANGEUPGRADE;
        case(uint32_t(ABILITY_ID::RESEARCH_TEMPESTRESEARCHGROUNDATTACKUPGRADE)):
            return UPGRADE_ID::TEMPESTGROUNDATTACKUPGRADE;  
        case(uint32_t(ABILITY_ID::RESEARCH_VOIDRAYSPEEDUPGRADE)):
            return UPGRADE_ID::VOIDRAYSPEEDUPGRADE;
        case(uint32_t(ABILITY_ID::RESEARCH_WARPGATE)):
            return UPGRADE_ID::WARPGATERESEARCH;
    }
    return 0;
}

static Cost buildAbilityToCost(AbilityID build_ability, Agent *agent) {
    if (build_ability == ABILITY_ID::MOVE_MOVE)
        return {0, 0, 0};
    sc2::UnitTypeData unit_stats =
        agent->Observation()->GetUnitTypeData().at(static_cast<uint32_t>(buildAbilityToUnit(build_ability)));
    return {unit_stats.mineral_cost, unit_stats.vespene_cost, 0};
}

static Cost UpgradeToCost(AbilityID research_ability, Agent *agent) {
    UpgradeData upgrade_stats =
        agent->Observation()->GetUpgradeData().at(static_cast<uint32_t>(researchAbilityToUpgrade(research_ability)));
    return {upgrade_stats.mineral_cost, upgrade_stats.vespene_cost, 0};
}

static int structureDiameter(UnitTypeID type) {
    switch (uint32_t(type)) {
        case (uint32_t(UNIT_TYPEID::PROTOSS_PYLON)):
            return 2;
        case (uint32_t(UNIT_TYPEID::PROTOSS_GATEWAY)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_NEXUS)):
            return 5;
        case (uint32_t(UNIT_TYPEID::PROTOSS_ASSIMILATOR)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_ROBOTICSFACILITY)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_WARPGATE)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_ASSIMILATORRICH)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_FORGE)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_PHOTONCANNON)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_ROBOTICSBAY)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_SHIELDBATTERY)):
            return 2;
        case (uint32_t(UNIT_TYPEID::PROTOSS_STARGATE)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_TEMPLARARCHIVE)):
            return 3;
        case (uint32_t(UNIT_TYPEID::PROTOSS_TWILIGHTCOUNCIL)):
            return 3;
        default:
            break;
    }

    /*PROTOSS_ASSIMILATOR = 61,
    PROTOSS_ASSIMILATORRICH = 1994, PROTOSS_CYBERNETICSCORE = 72,
    PROTOSS_DARKSHRINE = 69,
    PROTOSS_FLEETBEACON = 64, PROTOSS_FORGE = 63, PROTOSS_GATEWAY = 62,
    PROTOSS_NEXUS = 59, PROTOSS_PHOTONCANNON = 66,
    PROTOSS_PYLON = 60, PROTOSS_ROBOTICSBAY = 70, PROTOSS_ROBOTICSFACILITY = 71,
    PROTOSS_SHIELDBATTERY = 1910, PROTOSS_STARGATE = 67,
    PROTOSS_TEMPLARARCHIVE = 68, PROTOSS_TWILIGHTCOUNCIL = 65,
    PROTOSS_WARPGATE = 133,*/
}

static int theorySupply(Agent *agent) {
    Units nexi = agent->Observation()->GetUnits(Unit::Alliance::Self, Aux::isNexus);
    Units pylons = agent->Observation()->GetUnits(Unit::Alliance::Self, Aux::isPylon);
    return (nexi.size() * 15) + (pylons.size() * 8);
}

}  // namespace Aux

constexpr float timeSpeed = 1.4;
constexpr float fps = 16 * timeSpeed;

//BUILDINGS
constexpr int COST_NEXUS[2] = {400, 0};
constexpr int COST_ASSIMILATOR[2] = {75, 0};
constexpr int COST_PYLON[2] = {100, 0};
constexpr int COST_GATEWAY[2] = {150, 0};
constexpr int COST_FORGE[2] = {150, 0};
constexpr int COST_CYBERNETICS_CORE[2] = {150, 0};
constexpr int COST_PHOTON_CANNON[2] = {150, 0};
constexpr int COST_SHIELD_BATTERY[2] = {100, 0};

constexpr int COST_TWILIGHT_COUNCIL[2] = {150, 100};
constexpr int COST_STARGATE[2] = {150, 150};
constexpr int COST_ROBOTICS_FACILITY[2] = {150, 100};

constexpr int COST_TEMPLAR_ARCHIVES[2] = {150, 200};
constexpr int COST_DARK_SHRINE[2] = {150, 150};
constexpr int COST_FLEET_BEACON[2] = {300, 200};
constexpr int COST_ROBOTICS_BAY[2] = {150, 150};

//UNITS
constexpr int COST_PROBE[2] = {50, 0};
constexpr int COST_ZEALOT[2] = {100, 0};
constexpr int COST_SENTRY[2] = {50, 100};
constexpr int COST_STALKER[2] = {125, 50};
constexpr int COST_ADEPT[2] = {100, 25};

constexpr int COST_PHEONIX[2] = {150, 100};
constexpr int COST_ORACLE[2] = {150, 150};
constexpr int COST_VOID_RAY[2] = {250, 150};
constexpr int COST_TEMPEST[2] = {250, 175};
constexpr int COST_CARRIER[2] = {350, 250};

constexpr int COST_MOTHERSHIP[2] = {300, 300};

constexpr int COST_OBSERVER[2] = {25, 75};
constexpr int COST_WARP_PRISM[2] = {250, 0};
constexpr int COST_IMMORTAL[2] = {275, 100};
constexpr int COST_COLOSSUS[2] = {300, 200};
constexpr int COST_DISRUPTOR[2] = {150, 150};

constexpr int COST_HIGH_TEMPLAR[2] = {50, 150};
constexpr int COST_DARK_TEMPLAR[2] = {125, 125};

//UPGRADES
constexpr int COST_UPGRADE_AIR_WEAPON_1[2] = {100, 100};
constexpr int COST_UPGRADE_AIR_WEAPON_2[2] = {175, 175};
constexpr int COST_UPGRADE_AIR_WEAPON_3[2] = {250, 250};

constexpr int COST_UPGRADE_AIR_ARMOR_1[2] = {100, 100};
constexpr int COST_UPGRADE_AIR_ARMOR_2[2] = {175, 175};
constexpr int COST_UPGRADE_AIR_ARMOR_3[2] = {250, 250};

constexpr int COST_UPGRADE_WARP_GATE[2] = {50, 50};

constexpr int COST_UPGRADE_GROUND_WEAPON_1[2] = {100, 100};
constexpr int COST_UPGRADE_GROUND_WEAPON_2[2] = {150, 150};
constexpr int COST_UPGRADE_GROUND_WEAPON_3[2] = {200, 200};

constexpr int COST_UPGRADE_GROUND_ARMOR_1[2] = {100, 100};
constexpr int COST_UPGRADE_GROUND_ARMOR_2[2] = {150, 150};
constexpr int COST_UPGRADE_GROUND_ARMOR_3[2] = {200, 200};

constexpr int COST_UPGRADE_SHIELDS_1[2] = {150, 150};
constexpr int COST_UPGRADE_SHIELDS_2[2] = {200, 200};
constexpr int COST_UPGRADE_SHIELDS_3[2] = {250, 250};

constexpr int COST_UPGRADE_ZEALOT_CHARGE[2] = {100, 100};
constexpr int COST_UPGRADE_STALKER_BLINK[2] = {150, 150};
constexpr int COST_UPGRADE_ADEPT_GLAIVES[2] = {100, 100};

constexpr int COST_UPGRADE_HT_STORM[2] = {200, 200};
constexpr int COST_UPGRADE_DT_BLINK[2] = {100, 100};

constexpr int COST_UPGRADE_PHEONIX_RANGE[2] = {150, 150};
constexpr int COST_UPGRADE_VOID_RAY_SPEED[2] = {100, 100};
constexpr int COST_UPGRADE_TEMPEST_DAMAGE[2] = {150, 150};

constexpr int COST_UPGRADE_OBSERVER_SPEED[2] = {100, 100};
constexpr int COST_UPGRADE_WARP_PRISM_SPEED[2] = {100, 100};
constexpr int COST_UPGRADE_COLOSSUS_RANGE[2] = {150, 150};
