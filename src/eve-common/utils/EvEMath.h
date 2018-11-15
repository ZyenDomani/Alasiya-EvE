

 //////////////////////////////////////////////////////////////////////////////////////////
 //
 // EVE Math Equations for in-game features
 // (pulled directly from http://wiki.eve-id.net/Equations)
 //
 //////////////////////////////////////////////////////////////////////////////////////////

#include "../../eve-core/eve-core.h"

class EvilNumber;

namespace EvEMath {
    namespace Skill {
        EvilNumber EndTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, int64 timeNow );
        EvilNumber StartTime( EvilNumber currentSkillSP, EvilNumber nextLevelSkillSP, EvilNumber effectiveSPperMinute, int64 timeNow );
        EvilNumber PointsAtLevel( EvilNumber SkillLevel, EvilNumber SkillRank );
        EvilNumber PointsPerMinute( EvilNumber EffectivePrimaryAttribute, EvilNumber EffectiveSecondaryAttribute );
    }

    namespace RAM {
        EvilNumber ME_EffectOnWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor, EvilNumber MaterialEfficiency );
        EvilNumber ME_LevelToEliminateWaste( EvilNumber MaterialAmount, EvilNumber BaseWasteFactor );
        EvilNumber WasteSkillBased( EvilNumber MaterialAmount, EvilNumber ProductionEfficiency );
        EvilNumber ME_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber MetallurgySkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier );
        EvilNumber PE_ResearchTime( EvilNumber BlueprintBaseResearchTime, EvilNumber ResearchSkillLevel, EvilNumber ResearchSlotModifier, EvilNumber ImplantModifier );
        EvilNumber BluePrintCopyTime( EvilNumber BlueprintBaseCopyTime, EvilNumber ScienceSkillLevel, EvilNumber CopySlotModifier, EvilNumber ImplantModifier );
        EvilNumber ProductionTimeModifier( EvilNumber IndustrySkillLevel, EvilNumber ImplantModifier, EvilNumber ProductionSlotModifier );
        EvilNumber ProductionTime( EvilNumber BaseProductionTime, EvilNumber ProductivityModifier, EvilNumber ProductionEfficiency, EvilNumber ProductionTimeModifier );
        EvilNumber BlueprintInventionTime( EvilNumber BlueprintBaseInventionTime, EvilNumber InventionSlotModifier, EvilNumber ImplantModifier );
        EvilNumber BlueprintInventionChance( EvilNumber BaseChance, EvilNumber EncryptionSkillLevel, EvilNumber DataCore1SkillLevel, EvilNumber DataCore2SkillLevel, EvilNumber MetaLevel, EvilNumber DecryptorModifier );
        EvilNumber ResearchPointsPerDay( EvilNumber Multiplier, EvilNumber AgentEffectiveQuality, EvilNumber YourResearchSkillLevel, EvilNumber AgentResearchSkillLevel );
    }

    namespace Refine {
        float StationTaxesForReprocessing( float CharacterStandingWithStationOwner );
        EvilNumber EffectiveRefiningYield( EvilNumber StationEquipmentYield, EvilNumber RefiningSkillLevel, EvilNumber RefiningEfficiencySkillLevel, EvilNumber OreSpecificProcessingSkillLevel );
    }

    namespace Agent {
        EvilNumber EffectiveQuality( EvilNumber AgentQuality, EvilNumber NegotiationSkillLevel, EvilNumber AgentPersonalStanding );
        EvilNumber EffectiveStanding( EvilNumber YourStanding, EvilNumber ConnectionsSkillLevel, EvilNumber DiplomacySkillLevel );
        EvilNumber RequiredStanding( EvilNumber AgentLevel, EvilNumber AgentQuality );
        EvilNumber MissionStandingIncrease( EvilNumber BaseMissionIncrease, EvilNumber YourSocialSkillLevel );
        EvilNumber Efficiency( EvilNumber AgentLevel, EvilNumber AgentQuality );
    }


    EvilNumber EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus );
    EvilNumber TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius );
    EvilNumber AlignTimeInSeconds( EvilNumber inertia, EvilNumber Mass );
    EvilNumber TradeBrokerFee( EvilNumber BrokerRelationsSkillLevel, EvilNumber FactionStanding, EvilNumber CorporationStanding );

}


/*
 *    def Dijkstra(self, sourcePin, destinationPin):
 *        D = {}
 *        P = {}
 *        Q = planetCommon.priority_dict()
 *        Q[sourcePin] = 0.0
 *        while len(Q) > 0:
 *            vPin = Q.smallest()
 *            D[vPin] = Q[vPin]
 *            if vPin == destinationPin:
 *                break
 *            Q.pop_smallest()
 *            for wDestinationID in self.colonyData.GetLinksForPin(vPin.id):
 *                wLink = self.GetLink(vPin.id, wDestinationID)
 *                wPin = self.GetPin(wDestinationID)
 *                vwLength = D[vPin] + self._GetLinkWeight(wLink, wPin, vPin)
 *                if wPin in D:
 *                    if vwLength < D[wPin]:
 *                        raise ValueError, 'Dijkstra: found better path to already-final vertex'
 *                elif wPin not in Q or vwLength < Q[wPin]:
 *                    Q[wPin] = vwLength
 *                    P[wPin] = vPin
 *
 *        return (D, P)
 * */