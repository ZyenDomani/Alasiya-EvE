

 //////////////////////////////////////////////////////////////////////////////////////////
 //
 // EVE Math Equations for in-game features
 // (pulled directly from http://wiki.eve-id.net/Equations)
 //
 //////////////////////////////////////////////////////////////////////////////////////////

#ifndef EVE_COMMON_UTILS_MATH_H
#define EVE_COMMON_UTILS_MATH_H

#include "../../eve-core/eve-core.h"

class EvilNumber;

namespace EvESkill {
    // skill constants
    const uint8 MAXSKILLLEVEL = 5;
    const uint8 skillPointMultiplier = 250;
    const float DIVCONSTANT = std::log(2) * 2.5;
}

namespace EvEMath {
    namespace Skill {
        uint32 PointsAtLevel(uint8 level, uint8 rank);
        uint8 PointsPerMinute(uint8 pAttr, uint8 sAttr);
        uint8 LevelForPoints(uint32 currentSP, uint8 rank);
        int64 EndTime(uint32 currentSP, uint32 nextSP, uint8 SPMin, int64 timeNow);
        int64 StartTime(uint32 currentSP, uint32 nextSP, uint8 SPMin, int64 timeNow);
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
        float EffectiveQuality( int8 AgentQuality, uint8 NegotiationSkillLevel, float AgentPersonalStanding );
        float EffectiveStanding( float YourStanding, double standingBonus );
        float RequiredStanding( uint8 AgentLevel, int8 AgentQuality );
        float MissionStandingIncrease( float BaseMissionIncrease, uint8 YourSocialSkillLevel );
        float Efficiency( uint8 AgentLevel, int8 AgentQuality );
        float AgentStandingIncrease(float CurrentStanding, float PercentIncrease);
        float GetStandingBonus(float fromStanding, uint32 fromFactionID, uint8 ConnectionsSkillLevel, uint8 DiplomacySkillLevel, uint8 CriminalConnectionsSkillLevel);
    }

    namespace Market {
        float BrokerFee( uint8 brSkillLvl, float fStanding, float cStanding );
        float RelistFee(float oldPrice, float newPrice, float brokerPercent=0.05, float discount=0);
        float SalesTax(uint8 accountingSkillLvl=0, uint8 taxEvasionSkillLvl=0);
    }

    namespace PI {
        void Dijkstra(uint32 sourcePin, uint32 destinationPin);

        /**  @note client code for shortest path algorithm (PI shit)
         *
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
         */
    }

    EvilNumber EffectiveAttribute( EvilNumber BaseAttribute, EvilNumber ImplantAttributeBonus );
    EvilNumber TargetingLockTime( EvilNumber YourEffectiveScanResolution, EvilNumber TargetEffectiveSignatureRadius );
    EvilNumber AlignTimeInSeconds( EvilNumber inertia, EvilNumber Mass );

}

#endif  // EVE_COMMON_UTILS_MATH_H

