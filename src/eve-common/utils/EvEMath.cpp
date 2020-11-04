//////////////////////////////////////////////////////////////////////////////////////////
//
// EVE Math Equations for in-game features
// (pulled directly from client code and http://wiki.eve-id.net/Equations)
//
// Latest Update:       Allan  18Oct20
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "../eve-common.h"

// skill Equations
uint32 EvEMath::Skill::PointsAtLevel(uint8 level, uint8 rank)
{
    if (level > EvESkill::MAXSKILLLEVEL)
        level = EvESkill::MAXSKILLLEVEL;
    float ret = pow(sqrt(32), (level -1)) * EvESkill::skillPointMultiplier * rank;
    return (uint32)ceil(ret);
}

uint8 EvEMath::Skill::LevelForPoints(uint32 currentSP, uint8 rank)
{
    uint16 baseSLC = rank * EvESkill::skillPointMultiplier;
    if (baseSLC == 0)
        return 0;
    if (baseSLC > currentSP)
        return 0;
    int8 ret = log(currentSP / baseSLC) / EvESkill::DIVCONSTANT + 1;
    return (uint8)EvE::min(ret, EvESkill::MAXSKILLLEVEL);
}

uint8 EvEMath::Skill::PointsPerMinute(uint8 pAttr, uint8 sAttr)
{
    return (pAttr + (0.5f * sAttr));
}

int64 EvEMath::Skill::StartTime(uint32 currentSP, uint32 nextSP, uint8 SPMin, int64 timeNow)
{
    return (timeNow - (((nextSP - currentSP) / SPMin) * EvE::Time::Minute));
}

int64 EvEMath::Skill::EndTime(uint32 currentSP, uint32 nextSP, uint8 SPMin, int64 timeNow)
{
    if (currentSP >= nextSP)
        return 0;
    return ((((nextSP - currentSP) / SPMin) * EvE::Time::Minute) + timeNow);
}


//RAM Equations
float EvEMath::RAM::ProductionTimeModifier(uint8 IndustrySkillLevel, float ProductionSlotModifier/*1*/, float ImplantModifier/*1*/ )
{
    if (ImplantModifier == 0)
        ImplantModifier = 1;
    if (ProductionSlotModifier == 0)
        ProductionSlotModifier = 1;
    return (1.0f - (0.04f * IndustrySkillLevel)) * ImplantModifier * ProductionSlotModifier;
}

uint32 EvEMath::RAM::ProductionTime(uint32 BaseProductionTime, float ProductivityModifier, float ProductionLevel, float ProductionTimeModifier )
{
    float PE_Factor(0.0f);
    if (ProductionLevel >= 0.0f)
        PE_Factor = (ProductionLevel / (1.0f + ProductionLevel));
    else
        PE_Factor = (ProductionLevel - 1.0f);

    float effModifier(1.0f - (ProductivityModifier / BaseProductionTime) * PE_Factor);
    return (BaseProductionTime * effModifier * ProductionTimeModifier);
}

float EvEMath::RAM::ME_EffectOnWaste( float MaterialAmount, float BaseWasteFactor, float MaterialEfficiency )
{
    float ME_Factor(0.0f);
    if (MaterialEfficiency >= 0.0f)
        ME_Factor = 1.0f / (1.0f + MaterialEfficiency);
    else
        ME_Factor = 1.0f - MaterialEfficiency;

    return (floor(0.5f + (MaterialAmount * (BaseWasteFactor / 100.0f) * ME_Factor)));
}

uint32 EvEMath::RAM::PerfectME(uint32 MaterialAmount, uint8 BaseWasteFactor)
{
    return floor(0.02f * BaseWasteFactor * MaterialAmount);
}

float EvEMath::RAM::ResearchPointsPerDay( float Multiplier, float AgentEffectiveQuality, uint8 CharSkillLevel, uint8 AgentSkillLevel )
{
     return (Multiplier * (2.0f + (AgentEffectiveQuality / 100.0f)) * pow(CharSkillLevel + AgentSkillLevel,2));
}


float EvEMath::Refine::StationTaxesForReprocessing( float CharacterStandingWithStationOwner )
{
    return 5.0f - (0.75f * CharacterStandingWithStationOwner);
}

float EvEMath::Refine::EffectiveRefiningYield( float StationEquipmentYield, uint8 RefiningSkillLevel, uint8 RefiningEfficiencySkillLevel, uint8 OreProcessingSkillLevel )
{
    return (StationEquipmentYield + 0.375f * (1.0f + (RefiningSkillLevel * 0.02f))
            * (1.0f + (RefiningEfficiencySkillLevel * 0.04f)) * (1.0f + (OreProcessingSkillLevel * 0.05f)));
}

/** @todo update and verify these before use...and remove the fucking EvilNumber bullshit...NOT needed here. */
EvilNumber EvEMath::RAM::WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency )
{
	 return (floor(0.5f + (MaterialAmount.get_double() * ((25.0f - (5.0f * ProductionEfficiency.get_double())) / 100.0f))));
}

EvilNumber EvEMath::RAM::ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
{
	 return (BlueprintBaseResearchTime.get_double() * (13.0f - (0.05f * MetallurgySkillLevel.get_double()))
	 * ResearchSlotModifier.get_double() * ImplantModifier.get_double());
}

EvilNumber EvEMath::RAM::PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
{
	 return (BlueprintBaseResearchTime.get_double() * (15.0f - (0.05f * ResearchSkillLevel.get_double()))
	 * ResearchSlotModifier.get_double() * ImplantModifier.get_double());
}

float EvEMath::RAM::BpCopyTime( uint16 BaseCopyTime, uint8 ScienceLevel, float CopySlotModifier, float ImplantModifier/*1*/ )
{
	 return (BaseCopyTime * (1.0f - (0.05f * ScienceLevel)) * CopySlotModifier * ImplantModifier);
}

EvilNumber EvEMath::RAM::BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier )
{
     return BlueprintBaseInventionTime * InventionSlotModifier * ImplantModifier;
}

EvilNumber EvEMath::RAM::BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier )
{
     return (BaseChance.get_double() * (1 + 0.11f * EncryptionSkillLevel.get_double())
     * (1.0f + (DataCore1SkillLevel.get_double()+DataCore2SkillLevel.get_double())
     * (0.8f / (5.0f - MetaLevel.get_double())) * DecryptorModifier.get_double()));
}


// Agent Equations
float EvEMath::Agent::EffectiveQuality(int8 AgentQuality, uint8 NegotiationSkillLevel, float AgentPersonalStanding)
{
    return (AgentQuality + (5.0f * NegotiationSkillLevel) + AgentPersonalStanding);
}

float EvEMath::Agent::EffectiveStanding(float YourStanding, double standingBonus)
{
    return (1.0f - (1.0f - YourStanding / 10.0f) * (1.0f - standingBonus / 10.0f)) * 10.0f;
}

float EvEMath::Agent::RequiredStanding( uint8 AgentLevel, int8 AgentQuality )
{
    return (((AgentLevel - 1.0f) * 2.0f) + (AgentQuality/20.0f));
}

float EvEMath::Agent::MissionStandingIncrease( float BaseMissionIncrease, uint8 YourSocialSkillLevel )
{
    return (BaseMissionIncrease * (1.0f + 0.05f * YourSocialSkillLevel));
}

float EvEMath::Agent::Efficiency( uint8 AgentLevel, int8 AgentQuality )
{
    return (0.01f * ((8.0f * AgentLevel) + (0.1f * AgentQuality) - 4.0f));
}

float EvEMath::Agent::AgentStandingIncrease(float CurrentStanding, float PercentIncrease)
{
    return (((10.0f - CurrentStanding) * PercentIncrease) + CurrentStanding);
}

float EvEMath::Agent::GetStandingBonus(float fromStanding, uint32 fromFactionID, uint8 ConnectionsSkillLevel, uint8 DiplomacySkillLevel, uint8 CriminalConnectionsSkillLevel)
{
    float bonus(0.0f);
    if (fromStanding < 0.0f) {
        bonus = DiplomacySkillLevel * 0.4f;
    } else if (fromStanding > 0.0f) {
        switch (fromFactionID) {
            case 500010:
            case 500011:
            case 500012:
            case 500019:
            case 500020: {
                bonus = CriminalConnectionsSkillLevel * 0.4f;
            } break;
            default: {
                bonus = ConnectionsSkillLevel * 0.4f;
            } break;
        }
    }
    return bonus;
}

float EvEMath::Market::BrokerFee(uint8 brSkillLvl, float fStanding, float cStanding)
{
    float wStanding = (0.7f * fStanding + 0.3f * cStanding) / 10.0f;
    float fee = 0.01f * (1.0f - (0.05f * brSkillLvl)) * pow(2, -2 * wStanding);
    return EvE::max(fee, 100.0f);
}

float EvEMath::Market::RelistFee(float oldPrice, float newPrice, float brokerPercent/*0.01*/, float discount/*0*/)
{
    // this needs a 'Relist Discount' but no clue where to find data for it yet
    return EvE::max(brokerPercent * (newPrice -oldPrice)) + (1 -discount) *brokerPercent *newPrice;
}

float EvEMath::Market::SalesTax(uint8 accountingLvl/*0*/, uint8 taxEvasionLvl/*0*/)
{
    /** @todo  add skillTaxEvasion to this formula; its not calculated in client... */
    float tax = 0.01f * (1 - 0.1f * accountingLvl);
    return EvE::max(tax, 100.0f);
}

void EvEMath::PI::Dijkstra(uint32 sourcePin, uint32 destinationPin)
{
    // not used yet...
}


EvilNumber EvEMath::EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus )
{
    return (BaseAttribute + ImplantAttributeBonus);
}

EvilNumber EvEMath::TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius )
{
    return (40000.0f / (YourEffectiveScanResolution.get_double() * pow(asinh(TargetEffectiveSignatureRadius.get_double()),2)));
}

EvilNumber EvEMath::AlignTimeInSeconds( EvilNumber inertia, EvilNumber Mass )
{
    return ((log(2.0f) * inertia * Mass) / 500000);
}

/*
 *
 * Research Points Per Day
 * Research_Points_Per_Day = Multiplier * ((1 + (Agent_Effective_Quality / 100)) * ((Your_Skill + Agent_Skill) ^ 2))
 * Multiplier is a specific multiplier for the research field you want to do research in. Like 3x for starship engineering
 * Your_Skill is your skill level in the research field
 * Agent_Skill is the agent's skill level in the research field
 *
 *
 * Station take when refining/reprocessing
 * Station_Take = Max((5 - (0.75 * Your_Standing)), 0)
 * For the station to take 0% you need a standing to the station owner of at least: 5 / 0.75 = 6.67
 *
 *
 * Agent Effective Quality - Removed in Incursion 1.5 (2011-05-19)
 * Agent_Effective_Quality = Agent_Quality + (5 * Negotiation_Skill_Level) + Round_Down(Effective_Standing)
 * Effective_Standing is the highest effective of either personal, corp. or faction standing.
 *
 *
 * Blueprint Material Requirement - Before Crius 1.0 (2014-07-22)
 * Required_Amount = Round(Base_Amount * ((1 + (Default_Blueprint_Waste_Factor / (1 + Blueprint_Material_Level))) + (0.25 - (0.05 * Production_Efficiency_Skill_Level))), 0)
 *
 *
 * Invention Chance - Before Phoebe 1.0 (2014-11-04)
 * Invention_Chance = Base_Chance * (1 + (0.01 * Encryption_Skill_Level)) * (1 + ((Datacore_1_Skill_Level + Datacore_2_Skill_Level) * (0.1 / (5 - Meta_Level)))) * Decryptor_Modifier
 * Meta_Level of the base items used. No base items is the same as metalevel 0 = useless.
 * Decryptor_Modifier is optional :-)
 *
 *
 * Reverse Engineering Chance - Merged with Invention in Phoebe 1.0 (2014-11-04)
 * Reverse_Chance = Base_Chance * (1 + (0.01 * Reverse_Engineering_Skill_Level)) * (1 + (0.1 * (Datacore_1_Skill_Level + Datacore_2_Skill_Level)))
 */