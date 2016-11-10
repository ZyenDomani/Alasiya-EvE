 
 
 
 //////////////////////////////////////////////////////////////////////////////////////////
 //
 // EVE Math Equations for in-game features
 // (pulled directly from http://wiki.eve-id.net/Equations)
 //
 //////////////////////////////////////////////////////////////////////////////////////////
 
#include "utils/EVE_Equations.h"

 EvilNumber ME_EffectOnWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor, EvilNumber MaterialEfficiency )
 {
	 EvilNumber ME_Factor(0.0);
	 
	 if( MaterialEfficiency >= 0 )
		 ME_Factor = (1.0 / (MaterialEfficiency.get_double() + 1.0));
	 else
		 ME_Factor = (1.0 - MaterialEfficiency.get_double());
	 
	 return (floor(0.5 + (MaterialAmount.get_double() * (BaseWasteFactor.get_double() / 100.0) * ME_Factor.get_double())));
 }
 
 EvilNumber ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor )
 {
	 return (floor(0.02 * BaseWasteFactor.get_double() * MaterialAmount.get_double()));
 }
 
 EvilNumber WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency )
 {
	 return (floor(0.5 + (MaterialAmount.get_double() * ((25.0 - (5.0 * ProductionEfficiency.get_double())) / 100.0))));
 }
 
 EvilNumber ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
 {
	 return (BlueprintBaseResearchTime.get_double() * (13.0 - (0.05 * MetallurgySkillLevel.get_double()))
	 * ResearchSlotModifier.get_double() * ImplantModifier.get_double());
 }
 
 EvilNumber PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
 {
	 return (BlueprintBaseResearchTime.get_double() * (15.0 - (0.05 * ResearchSkillLevel.get_double()))
	 * ResearchSlotModifier.get_double() * ImplantModifier.get_double());
 }
 
 EvilNumber BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier )
 {
	 return (BlueprintBaseCopyTime.get_double() * (1.0 - (0.05 * ScienceSkillLevel.get_double()))
	 * CopySlotModifier.get_double() * ImplantModifier.get_double());
 }
 
 EvilNumber ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier )
 {
	 return (8.0 - (0.04 * IndustrySkillLevel.get_double()) * ImplantModifier.get_double() * ProductionSlotModifier.get_double());
 }
 
 EvilNumber ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier )
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
 
 EvilNumber StationTaxesForReprocessing( EvilNumber CharacterStandingWithStationOwner )
 {
	 return (5.0 - 0.75 * CharacterStandingWithStationOwner.get_double());
 }
 
 EvilNumber EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel )
 {
	 return (StationEquipmentYield.get_double() + 0.375 * (1 + (RefiningSkillLevel.get_double() * 0.02))
	 * (1 + (RefiningEfficiencySkillLevel.get_double() * 0.04))
	 * (1 + (OreSpecificProcessingSkillLevel.get_double() * 0.05)));
 }
 
 EvilNumber BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier )
 {
	 return BlueprintBaseInventionTime * InventionSlotModifier * ImplantModifier;
 }
 
 EvilNumber BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier )
 {
	 return (BaseChance.get_double() * (1+0.11*EncryptionSkillLevel.get_double())
	 * (1+(DataCore1SkillLevel.get_double()+DataCore2SkillLevel.get_double())
	 * (0.8 / (5 - MetaLevel.get_double())) * DecryptorModifier.get_double()));
 }
 
 EvilNumber ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel )
 {
	 return (Multiplier.get_double() * (2 + (AgentEffectiveQuality.get_double() / 100.0))
	 * pow(YourResearchSkillLevel.get_double() + AgentResearchSkillLevel.get_double(),2));
 }
 
 EvilNumber AgentEffectiveQuality( EvilNumber AgentQuality, EvilNumber NegotiationSkillLevel, EvilNumber AgentPersonalStanding )
 {
	 return (AgentQuality.get_double() + (5.0 * NegotiationSkillLevel.get_double()) + AgentPersonalStanding.get_double());
 }
 
 EvilNumber EffectiveStanding( EvilNumber YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel )
 {
	 EvilNumber SkillLevel(0.0);
	 
	 if( YourStanding < 0.0 )
		 SkillLevel = DiplomacySkillLevel;
	 else
		 SkillLevel = ConnectionsSkillLevel;
	 
	 return (YourStanding.get_double() + ((10.0 - YourStanding.get_double()) * (0.04 * (SkillLevel.get_double()))));
 }
 
 EvilNumber RequiredAgentStanding( EvilNumber AgentLevel, EvilNumber AgentQuality )
 {
	 return (((AgentLevel.get_double() - 1) * 2) + (AgentQuality.get_double()/20.0));
 }
 
 EvilNumber MissionStandingIncrease( EvilNumber BaseMissionIncrease, EvilNumber YourSocialSkillLevel )
 {
	 return (BaseMissionIncrease * (1 + 0.05 * YourSocialSkillLevel.get_double()));
 }
 
 EvilNumber AgentEfficiency( EvilNumber AgentLevel, EvilNumber AgentQuality )
 {
	 return (0.01 * ((8 * AgentLevel) + (0.1 * AgentQuality) - 4));
 }
 
 EvilNumber SkillPointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank )
 {
	 return (pow( 2, (2.5 * SkillLevel.get_double()) - 2.5 ) * 250.0 * SkillRank);
 }
 
 EvilNumber EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus )
 {
	 return (BaseAttribute + ImplantAttributeBonus);
 }
 
 EvilNumber SkillPointsPerMinute( EvilNumber EffectivePrimaryAttribute, EvilNumber EffectiveSecondaryAttribute )
 {
	 return (EffectivePrimaryAttribute + (0.5 * EffectiveSecondaryAttribute));
 }
 
 EvilNumber TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius )
 {
	 return (40000.0 / (YourEffectiveScanResolution.get_double() * pow(asinh(TargetEffectiveSignatureRadius.get_double()),2)));
 }
 
 EvilNumber AlignTimeInSeconds( EvilNumber InertiaFactor, EvilNumber Mass )
 {
	 return ((log(2.0) * InertiaFactor * Mass) / 500000);
 }
 
 EvilNumber TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding )
 {
	 return (100.0 * ((0.01 - 0.0005 * BrokerRelationsSkillLevel.get_double())
	 / (pow( 2, (0.14 * FactionStanding.get_double() + 0.06 * CorporationStanding.get_double()) ))));
 }
 
 EvilNumber SkillStartingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow )
 {
	 return (timeNow - ((currentSkillSP / effectiveSPperMinute) * EvilTime_Minute));
 }
 
 EvilNumber SkillEndingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow )
 {
	 return ((((nextLevelSkillSP - currentSkillSP) / effectiveSPperMinute) * EvilTime_Minute) + timeNow);
 }
 
 