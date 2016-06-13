 
 
 
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
		 ME_Factor = (1.0 / (MaterialEfficiency.get_float() + 1.0));
	 else
		 ME_Factor = (1.0 - MaterialEfficiency.get_float());
	 
	 return (floor(0.5 + (MaterialAmount.get_float() * (BaseWasteFactor.get_float() / 100.0) * ME_Factor.get_float())));
 }
 
 EvilNumber ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor )
 {
	 return (floor(0.02 * BaseWasteFactor.get_float() * MaterialAmount.get_float()));
 }
 
 EvilNumber WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency )
 {
	 return (floor(0.5 + (MaterialAmount.get_float() * ((25.0 - (5.0 * ProductionEfficiency.get_float())) / 100.0))));
 }
 
 EvilNumber ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
 {
	 return (BlueprintBaseResearchTime.get_float() * (13.0 - (0.05 * MetallurgySkillLevel.get_float()))
	 * ResearchSlotModifier.get_float() * ImplantModifier.get_float());
 }
 
 EvilNumber PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier )
 {
	 return (BlueprintBaseResearchTime.get_float() * (15.0 - (0.05 * ResearchSkillLevel.get_float()))
	 * ResearchSlotModifier.get_float() * ImplantModifier.get_float());
 }
 
 EvilNumber BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier )
 {
	 return (BlueprintBaseCopyTime.get_float() * (1.0 - (0.05 * ScienceSkillLevel.get_float()))
	 * CopySlotModifier.get_float() * ImplantModifier.get_float());
 }
 
 EvilNumber ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier )
 {
	 return (8.0 - (0.04 * IndustrySkillLevel.get_float()) * ImplantModifier.get_float() * ProductionSlotModifier.get_float());
 }
 
 EvilNumber ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier )
 {
	 EvilNumber PE_Factor(0.0);
	 
	 if( ProductionEfficiency >= 0.0 )
		 PE_Factor = (ProductionEfficiency.get_float() / (1.0 + ProductionEfficiency.get_float()));
	 else
		 PE_Factor = (ProductionEfficiency.get_float() - 1.0);
	 
	 return (BaseProductionTime.get_float()
	 * (1.0 - (ProductivityModifier.get_float() / BaseProductionTime.get_float())
	 * (PE_Factor.get_float()))
	 * ProductionTimeModifier.get_float());
 }
 
 EvilNumber StationTaxesForReprocessing( EvilNumber CharacterStandingWithStationOwner )
 {
	 return (5.0 - 0.75 * CharacterStandingWithStationOwner.get_float());
 }
 
 EvilNumber EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel )
 {
	 return (StationEquipmentYield.get_float() + 0.375 * (1 + (RefiningSkillLevel.get_float() * 0.02))
	 * (1 + (RefiningEfficiencySkillLevel.get_float() * 0.04))
	 * (1 + (OreSpecificProcessingSkillLevel.get_float() * 0.05)));
 }
 
 EvilNumber BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier )
 {
	 return BlueprintBaseInventionTime * InventionSlotModifier * ImplantModifier;
 }
 
 EvilNumber BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier )
 {
	 return (BaseChance.get_float() * (1+0.11*EncryptionSkillLevel.get_float())
	 * (1+(DataCore1SkillLevel.get_float()+DataCore2SkillLevel.get_float())
	 * (0.8 / (5 - MetaLevel.get_float())) * DecryptorModifier.get_float()));
 }
 
 EvilNumber ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel )
 {
	 return (Multiplier.get_float() * (2 + (AgentEffectiveQuality.get_float() / 100.0))
	 * pow(YourResearchSkillLevel.get_float() + AgentResearchSkillLevel.get_float(),2));
 }
 
 EvilNumber AgentEffectiveQuality( EvilNumber AgentQuality, EvilNumber NegotiationSkillLevel, EvilNumber AgentPersonalStanding )
 {
	 return (AgentQuality.get_float() + (5.0 * NegotiationSkillLevel.get_float()) + AgentPersonalStanding.get_float());
 }
 
 EvilNumber EffectiveStanding( EvilNumber YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel )
 {
	 EvilNumber SkillLevel(0.0);
	 
	 if( YourStanding < 0.0 )
		 SkillLevel = DiplomacySkillLevel;
	 else
		 SkillLevel = ConnectionsSkillLevel;
	 
	 return (YourStanding.get_float() + ((10.0 - YourStanding.get_float()) * (0.04 * (SkillLevel.get_float()))));
 }
 
 EvilNumber RequiredAgentStanding( EvilNumber AgentLevel, EvilNumber AgentQuality )
 {
	 return (((AgentLevel.get_float() - 1) * 2) + (AgentQuality.get_float()/20.0));
 }
 
 EvilNumber MissionStandingIncrease( EvilNumber BaseMissionIncrease, EvilNumber YourSocialSkillLevel )
 {
	 return (BaseMissionIncrease * (1 + 0.05 * YourSocialSkillLevel.get_float()));
 }
 
 EvilNumber AgentEfficiency( EvilNumber AgentLevel, EvilNumber AgentQuality )
 {
	 return (0.01 * ((8 * AgentLevel) + (0.1 * AgentQuality) - 4));
 }
 
 EvilNumber SkillPointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank )
 {
	 return (pow( 2, (2.5 * SkillLevel.get_float()) - 2.5 ) * 250.0 * SkillRank);
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
	 return (40000.0 / (YourEffectiveScanResolution.get_float() * pow(asinh(TargetEffectiveSignatureRadius.get_float()),2)));
 }
 
 EvilNumber AlignTimeInSeconds( EvilNumber InertiaFactor, EvilNumber Mass )
 {
	 return ((log(2.0) * InertiaFactor * Mass) / 500000);
 }
 
 EvilNumber TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding )
 {
	 return (100.0 * ((0.01 - 0.0005 * BrokerRelationsSkillLevel.get_float())
	 / (pow( 2, (0.14 * FactionStanding.get_float() + 0.06 * CorporationStanding.get_float()) ))));
 }
 
 EvilNumber SkillStartingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow )
 {
	 return (timeNow - ((currentSkillSP / effectiveSPperMinute) * EvilTime_Minute));
 }
 
 EvilNumber SkillEndingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow )
 {
	 return ((((nextLevelSkillSP - currentSkillSP) / effectiveSPperMinute) * EvilTime_Minute) + timeNow);
 }
 
 