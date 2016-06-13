 
 
 //////////////////////////////////////////////////////////////////////////////////////////
 //
 // EVE Math Equations for in-game features
 // (pulled directly from http://wiki.eve-id.net/Equations)
 //
 //////////////////////////////////////////////////////////////////////////////////////////
 
 #include "eve-common.h"
 
 #include "EvilNumber.h"
 
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber ME_EffectOnWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor, EvilNumber MaterialEfficiency );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber StationTaxesForReprocessing( EvilNumber CharacterStandingWithStationOwner );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber AgentEffectiveQuality( EvilNumber AgentQuality, EvilNumber NegotiationSkillLevel, EvilNumber AgentPersonalStanding );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber EffectiveStanding( EvilNumber YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber RequiredAgentStanding( EvilNumber AgentLevel, EvilNumber AgentQuality );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber MissionStandingIncrease( EvilNumber BaseMissionIncrease, EvilNumber YourSocialSkillLevel );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber AgentEfficiency( EvilNumber AgentLevel, EvilNumber AgentQuality );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber SkillPointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber SkillPointsPerMinute( EvilNumber EffectivePrimaryAttribute, EvilNumber EffectiveSecondaryAttribute );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber AlignTimeInSeconds( EvilNumber intertia, EvilNumber Mass );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber SkillStartingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow );
 
 /**
  * ?
  *
  * @param[in] ?
  * @return ?
  */
 EvilNumber SkillEndingTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, EvilNumber timeNow );
 