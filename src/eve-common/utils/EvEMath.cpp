//////////////////////////////////////////////////////////////////////////////////////////
//
// EVE Math Equations for in-game features
// (pulled directly from http://wiki.eve-id.net/Equations)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include "../eve-common.h"
#include "utils/EvEMath.h"

// skill Equations
EvilNumber EvEMath::Skill::PointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank )
{
    //math.ceil(250 * skillTimeConstant * 2 ** (2.5 * (level - 1)))
    return pow(sqrt(32), (SkillLevel.get_int() -1)) * 250 * SkillRank.get_int();
}

EvilNumber EvEMath::Skill::PointsPerMinute( EvilNumber EffectivePrimaryAttribute, EvilNumber EffectiveSecondaryAttribute )
{
    return (EffectivePrimaryAttribute + (0.5 * EffectiveSecondaryAttribute));
}

EvilNumber EvEMath::Skill::StartTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, int64 timeNow )
{
    return (timeNow - (((nextLevelSkillSP - currentSkillSP) / effectiveSPperMinute) * Win32Time_Minute));
}

EvilNumber EvEMath::Skill::EndTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, int64 timeNow )
{
    return ((((nextLevelSkillSP - currentSkillSP) / effectiveSPperMinute) * Win32Time_Minute) + timeNow);
}


//RAM Equations
/** @todo update and verify these before use...and remove the fucking EvilNumber bullshit...NOT needed here. */
EvilNumber EvEMath::RAM::ME_EffectOnWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor, EvilNumber MaterialEfficiency )
{
	 EvilNumber ME_Factor(0.0);

	 if( MaterialEfficiency >= 0 )
		 ME_Factor = (1.0 / (MaterialEfficiency.get_double() + 1.0));
	 else
		 ME_Factor = (1.0 - MaterialEfficiency.get_double());

	 return (floor(0.5 + (MaterialAmount.get_double() * (BaseWasteFactor.get_double() / 100.0) * ME_Factor.get_double())));
}

EvilNumber EvEMath::RAM::ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor )
{
	 return (floor(0.02 * BaseWasteFactor.get_double() * MaterialAmount.get_double()));
}

EvilNumber EvEMath::RAM::WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency )
{
	 return (floor(0.5 + (MaterialAmount.get_double() * ((25.0 - (5.0 * ProductionEfficiency.get_double())) / 100.0))));
}

EvilNumber EvEMath::RAM::ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
{
	 return (BlueprintBaseResearchTime.get_double() * (13.0 - (0.05 * MetallurgySkillLevel.get_double()))
	 * ResearchSlotModifier.get_double() * ImplantModifier.get_double());
}

EvilNumber EvEMath::RAM::PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
{
	 return (BlueprintBaseResearchTime.get_double() * (15.0 - (0.05 * ResearchSkillLevel.get_double()))
	 * ResearchSlotModifier.get_double() * ImplantModifier.get_double());
}

EvilNumber EvEMath::RAM::BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier )
{
	 return (BlueprintBaseCopyTime.get_double() * (1.0 - (0.05 * ScienceSkillLevel.get_double()))
	 * CopySlotModifier.get_double() * ImplantModifier.get_double());
}

EvilNumber EvEMath::RAM::ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier )
{
    return (8.0 - (0.04 * IndustrySkillLevel.get_double()) * ImplantModifier.get_double() * ProductionSlotModifier.get_double());
}

EvilNumber EvEMath::RAM::ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier )
{
	 EvilNumber PE_Factor(0.0);

	 if( ProductionEfficiency >= 0.0 )
		 PE_Factor = (ProductionEfficiency.get_double() / (1.0 + ProductionEfficiency.get_double()));
	 else
		 PE_Factor = (ProductionEfficiency.get_double() - 1.0);

	 return (BaseProductionTime.get_double()
	 * (1.0 - (ProductivityModifier.get_double() / BaseProductionTime.get_double())
	 * (PE_Factor.get_double()))
	 * ProductionTimeModifier.get_double());
}

EvilNumber EvEMath::RAM::BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier )
{
     return BlueprintBaseInventionTime * InventionSlotModifier * ImplantModifier;
}

EvilNumber EvEMath::RAM::BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier )
{
     return (BaseChance.get_double() * (1+0.11*EncryptionSkillLevel.get_double())
     * (1+(DataCore1SkillLevel.get_double()+DataCore2SkillLevel.get_double())
     * (0.8 / (5 - MetaLevel.get_double())) * DecryptorModifier.get_double()));
}

EvilNumber EvEMath::RAM::ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel )
{
     return (Multiplier.get_double() * (2 + (AgentEffectiveQuality.get_double() / 100.0))
     * pow(YourResearchSkillLevel.get_double() + AgentResearchSkillLevel.get_double(),2));
}


float EvEMath::Refine::StationTaxesForReprocessing( float CharacterStandingWithStationOwner )
{
    return (5.0 - 0.75 * CharacterStandingWithStationOwner);
}

EvilNumber EvEMath::Refine::EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel )
{
    return (StationEquipmentYield.get_double() + 0.375 * (1 + (RefiningSkillLevel.get_double() * 0.02))
    * (1 + (RefiningEfficiencySkillLevel.get_double() * 0.04))
    * (1 + (OreSpecificProcessingSkillLevel.get_double() * 0.05)));
}



float EvEMath::Agent::EffectiveQuality( float AgentQuality, EvilNumber NegotiationSkillLevel, float AgentPersonalStanding )
{
    return (AgentQuality + (5.0 * NegotiationSkillLevel.get_double()) + AgentPersonalStanding);
}

float EvEMath::Agent::EffectiveStanding( float YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel )
{
    EvilNumber SkillLevel(0.0);

    if( YourStanding < 0.0 )
        SkillLevel = DiplomacySkillLevel;
    else
        SkillLevel = ConnectionsSkillLevel;

    return (YourStanding + ((10.0 - YourStanding) * (0.04 * (SkillLevel.get_double()))));
}

float EvEMath::Agent::RequiredStanding( uint8 AgentLevel, float AgentQuality )
{
    return (((AgentLevel - 1) * 2) + (AgentQuality/20.0));
}

float EvEMath::Agent::MissionStandingIncrease( float BaseMissionIncrease, EvilNumber YourSocialSkillLevel )
{
    return (BaseMissionIncrease * (1 + 0.05 * YourSocialSkillLevel.get_double()));
}

float EvEMath::Agent::Efficiency( uint8 AgentLevel, float AgentQuality )
{
    return (0.01 * ((8 * AgentLevel) + (0.1 * AgentQuality) - 4));
}

float EvEMath::Agent::AgentStandingIncrease(float CurrentStanding, float PercentIncrease)
{
    return (((10 - CurrentStanding) * PercentIncrease) + CurrentStanding);
}



EvilNumber EvEMath::EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus )
{
    return (BaseAttribute + ImplantAttributeBonus);
}

EvilNumber EvEMath::TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius )
{
    return (40000.0 / (YourEffectiveScanResolution.get_double() * pow(asinh(TargetEffectiveSignatureRadius.get_double()),2)));
}

EvilNumber EvEMath::AlignTimeInSeconds( EvilNumber inertia, EvilNumber Mass )
{
    return ((log(2.0) * inertia * Mass) / 500000);
}

EvilNumber EvEMath::TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding )
{
    return (100.0 * ((0.01 - 0.0005 * BrokerRelationsSkillLevel.get_double())
    / (pow( 2, (0.14 * FactionStanding.get_double() + 0.06 * CorporationStanding.get_double()) ))));
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