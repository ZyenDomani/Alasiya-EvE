
/*
 *  Account and Journal typedefs
 *
 */



#ifndef EVE_ACCOUNT_H
#define EVE_ACCOUNT_H


namespace Account {

    namespace CreditType {
        enum {
            ISK = 0,
            AURUM = 1,
            MPLEX = 2
        };
    }

    namespace KeyType {
        enum {
            Cash           = 1000,
            Cash2          = 1001,     //walletDivision2...
            Cash3          = 1002,
            Cash4          = 1003,
            Cash5          = 1004,
            Cash6          = 1005,
            Cash7          = 1006,
            Property       = 1100,
            AUR            = 1200,
            AUR2           = 1201,     //walletDivision2...
            AUR3           = 1202,
            AUR4           = 1203,
            AUR5           = 1204,
            AUR6           = 1205,
            AUR7           = 1206,
            Escrow         = 1500,
            Receivables    = 1800,
            Payables       = 2000,
            Gold           = 2010,
            Equity         = 2900,
            Sales          = 3000,
            Purchases      = 4000,

            // dust shit isnt used, and may not be...but here for completeness
            DUST_ISK = 10000,
            DUST_MODIFYISK = 10001,
            DUST_PRIMARYMARKETPURCHASE = 10002,
            DUST_NEW_CHARACTER_MONEY = 10004,
            DUST_CORP_WITHDRAWAL = 10005,
            DUST_CORP_DEPOSIT = 10006,
            DUST_BATTLEREWARD_WIN = 10009,
            DUST_BATTLEREWARD_LOSS = 10010,
            DUST_ISK_RESET_FOR_CHARACTER_WIPE = 10011,
            DUST_AUR = 11000,
            DUST_MODIFYRMC = 11001,
            DUST_AUR_FROM_ENTITLEMENT = 11003,
            DUST_AUR_FROM_RESET_REIMBURSEMENT = 11004,
            DUST_AUR_RESET_FOR_CHARACTER_WIPE = 11005
        };
    }
}

//  -allan 20Dec14    UD 9Dec17
// '*' denotes populated referenceID field (and with what)  - used for popup descriptions
/*  these are put in description field for "popup" descriptions.
 *   NOTE:  npc bounties using BountyPrize MUST have npc typeID as refereceID.
 *          those using BountyPrizes MUST have dict of [typeID/qty] in description
 * recDescription = 'DESC'                     <-- defines a custom description
 * recDescNpcBountyList = 'NBL'                <-- descrives a full list
 * recDescNpcBountyListTruncated = 'NBLT'      <-- describes a trunicated list
 * recStoreItems = 'STOREITEMS'                <-- not sure here
 */

namespace Journal {
    namespace EntryType {
        enum {
            SkipLog = -1,
            Undefined = 0,
            PlayerTrading = 1,      // * stationID
            MarketTransaction = 2,     // * transactionID
            GMCashTransfer = 3,
            ATMWithdraw = 4,
            ATMDeposit = 5,
            BackwardCompatible = 6,
            MissionReward = 7,
            CloneActivation = 8,     // * EVE System    1
            Inheritance = 9,
            PlayerDonation = 10,
            CorporationPayment = 11,
            DockingFee = 12,
            OfficeRentalFee = 13,     // * EVE System    1
            FactorySlotRentalFee = 14,
            RepairBill = 15,
            Bounty = 16,     // * characterID with bounty
            BountyPrize = 17,     // * NPC ID
            Insurance = 19,     // * Destroyed Ship Type ID
            MissionExpiration = 20,
            MissionCompletion = 21,
            Shares = 22,
            CourierMissionEscrow = 23,
            MissionCost = 24,
            AgentMiscellaneous = 25,
            PaymentToLPStore = 26,
            AgentLocationServices = 27,
            AgentDonation = 28,
            AgentSecurityServices = 29,
            AgentMissionCollateralPaid = 30,
            AgentMissionCollateralRefunded = 31,
            AgentMissionReward = 33,     // * agent ID
            AgentMissionTimeBonusReward = 34,     // * agent ID
            CSPA = 35,     // * character ID  you're trying to contact.
            CSPAOfflineRefund = 36,
            CorporationAccountWithdrawal = 37,     // *Player character ID that performed withdrawal
            CorporationDividendPayment = 38,
            CorporationRegistrationFee = 39,    // * Corporation ID
            CorporationLogoChangeCost = 40,     // * Corporation ID
            ReleaseOfImpoundedProperty = 41,
            MarketEscrow = 42,
            MarketFinePaid = 44,
            Brokerfee = 46,     // * EVE System    1
            AllianceRegistrationFee = 48,
            WarFee = 49,
            AllianceMaintainanceFee = 50,     // * Alliance ID (to concord)
            ContrabandFine = 51,
            CloneTransfer = 52,
            AccelerationGateFee = 53,
            TransactionTax = 54,     // *
            JumpCloneInstallationFee = 55,     // * EVE System    1
            Manufacturing = 56,     // *
            ResearchingTechnology = 57,
            ResearchingTimeProductivity = 58,
            ResearchingMaterialProductivity = 59,
            Copying = 60,
            Duplicating = 61,
            ReverseEngineering = 62,
            ContractAuctionBid = 63,     // *Contract ID
            ContractAuctionBidRefund = 64,     // *Contract ID
            ContractCollateral = 65,
            ContractRewardRefund = 66,
            ContractAuctionSold = 67,
            ContractReward = 68,
            ContractCollateralRefund = 69,
            ContractCollateralPayout = 70,
            ContractPrice = 71,     // *Contract ID
            ContractBrokersFee = 72,     // *Contract ID
            ContractSalesTax = 73,     // *Contract ID
            ContractDeposit = 74,     // *Contract ID
            ContractDepositSalesTax = 75,
            SecureEVETimeCodeExchange = 76,
            ContractAuctionBidCorp = 77,
            ContractCollateralCorp = 78,
            ContractPriceCorp = 79,     // *Contract ID
            ContractBrokersFeeCorp = 80,     // *Contract ID
            ContractDepositCorp = 81,     // *Contract ID
            ContractDepositRefund = 82,     // *Contract ID
            ContractRewardAdded = 83,
            ContractRewardAddedCorp = 84,
            BountyPrizes = 85,     // * systemID (and type:amt,type:amt,etc  in description (max ~60chars))
            CorporationAdvertisementFee = 86,
            MedalCreation = 87,     // * Character ID of the player creating the medal
            MedalIssuing = 88,     // * Character ID of the player issuing the medal
            AttributeRespecification = 90,
            SovereignityRegistrarFee = 91,
            CorporationTaxNpcBounties = 92,
            CorporationTaxAgentRewards = 93,
            CorporationTaxAgentBonusRewards = 94,
            SovereignityUpkeepAdjustment = 95,
            PlanetaryImportTax = 96,     // * Planet ID
            PlanetaryExportTax = 97,     // * Planet ID
            PlanetaryConstruction = 98,
            RewardManager = 99,
            BountySurcharge = 101,
            ContractReversal = 102,
            CorporationTaxRewards = 103,
            StorePurchase = 106,
            StoreRefund = 107,
            PlexConversion = 108,
            AurumGiveAway = 109,
            MiniGameHouseCut = 110,
            AurumTokenConversion = 111,
            Max = 120,
            /* not sure on these
            ModifyISK = 10001,
            PrimaryMarketplacePurchase = 10002,
            BattleReward = 10003,
            NewCharacterStartingFunds = 10004,
            CorporationAccountWithdrawal = 10005,
            CorporationAccountDeposit = 10006,
            BattleWPWinReward = 10007,
            BattleWPLossReward = 10008,
            BattleWinReward = 10009,
            BattleLossReward = 10010,
            ModifyAUR = 11001,
            RespecPayment = 11002
            */
        };
    }
}


#endif  // EVE_ACCOUNT_H