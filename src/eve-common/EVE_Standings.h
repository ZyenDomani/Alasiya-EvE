
/*
 *  EVE_Standings.h
 *   standings-specific data
 *
 */

#ifndef EVE_STANDINGS_H
#define EVE_STANDINGS_H


namespace Standings {

//these are agent/corp/faction -> char
    static const float Bad    = -1.0f;
    static const float Lo     =  1.5f;
    static const float LoMid  =  3.5f;
    static const float MidHi  =  5.5f;
    static const float Hi     =  7.5f;

//eve standing change messages in db.repStandingChanges.eventTypeID
//  these come from /eve/common/script/mgt/appLogConst.py

    enum {
        PodKill                         = 9,
        PodKilled                       = 10,
        StandingReset                   = 25,   //Reset by a GM.
        ShipKill                        = 26,
        ShipKilled                      = 27,
        UpdateStanding                  = 45,
        Decay                           = 49,   //All standing decays except when user isn't logged in
        PlayerSet                       = 65,   //Set by player him/herself. Reason: _msg
        CorpSet                         = 68,   //Corp stand set by _int1. Reason: _msg
        MissionCompleted                = 73,   //_msg: name of mission
        MissionFailure                  = 74,   //_msg: name of mission
        MissionDeclined                 = 75,   //_msg: name of mission
        CombatAggression                = 76,   //Combat - Aggression
        CombatShipKill                  = 77,   //Combat - Ship Kill
        CombatPodKill                   = 78,   //Combat - Pod Kill
        CombatOther                     = 79,
        MissionBonus                    = 80,   //_msg: name of mission
        DerivedModificationPleased      = 82,   //<name> was pleased
        DerivedModificationDispleased   = 83,   //<name> was displeased
        GMInterventionDirect            = 84,   //Mod directly by _int1. Reason: _msg
        SucceededMission                = 86,
        MissionAccepted                 = 88,   //_msg: name of mission
        LawEnforcement                  = 89,   //Granted by Concord for actions against _int1
        MissionOfferExpired             = 90,   //Mission Offer Expired - _msg
        StandingCorrection              = 96,
        MissionFailedRollback           = 97,   //_msg: name of mission
        StandingRollback                = 98,
        CombatAssistance                = 112,  //Combat - Assistance
        PropertyDamage                  = 154,  //Property Damage
        CombatShipKillOwnFaction        = 223,
        CombatPodKillOwnFaction         = 224,
        CombatAggressionOwnFaction      = 225,
        CombatAssistanceOwnFaction      = 226,
        CombatOtherOwnFaction           = 228
        //anything up to 500 is 'Standing Change'
    };

    /* UI/Generic/FormatStandingTransactions/subjectDecay: 235078: Standing Decay
     * UI/Generic/FormatStandingTransactions/messageDecay: 235079: All standings decay by a certain amount on a regular basis.\r\n\r\nAn exception to this rule is made for players that haven't logged in\r\nrecently.
     * UI/Generic/FormatStandingTransactions/subjectDerivedModificatonPositive: 235080: Derived Modification
     * UI/Generic/FormatStandingTransactions/messageDerivedModificatonPositive: 235081: {name} was pleased by actions on you performed for {name}'s friends, or against {name}'s enemies
     * UI/Generic/FormatStandingTransactions/subjectDerivedModificatonNegitive: 235082: Derived Modification
     * UI/Generic/FormatStandingTransactions/messageDerivedModificatonNegitive: 235083: {name} was displeased by actions on you performed against {name}'s friends, or for {name}'s enemies.
     * UI/Generic/FormatStandingTransactions/subjectCombatAgression: 235084: Combat - Aggression
     * UI/Generic/FormatStandingTransactions/messageCombatAgression: 235085: This penalty was incurred for attacking {ownerName}'s {[item]typeID.name} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/subjectCombatAssistence: 235086: Combat - Assistance
     * UI/Generic/FormatStandingTransactions/messageCombatAssistence: 235087: This penalty was incurred for Assisting {name}'s {[item]typeID.name} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/subjectCombatShipKill: 235088: Combat - Ship Kill
     * UI/Generic/FormatStandingTransactions/messageCombatShipKill: 235089: This penalty was incurred for destroying {name}'s {[item]typeID.name} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/subjectPropertyDamage: 235090: Property Damage
     * UI/Generic/FormatStandingTransactions/messagePropertyDamage: 235091: This penalty was incurred for destroying {name}'s {[item]typeID.name} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/subjectCombatPodKill: 235092: Combat - Pod Kill
     * UI/Generic/FormatStandingTransactions/messageCombatPodKill: 235093: This penalty was incurred for podding {name} in {locationName}
     * UI/Generic/FormatStandingTransactions/subjectSetBySlashCmd: 235094: GM Intervention
     * UI/Generic/FormatStandingTransactions/messageSetBySlashCmd: 235095: This modification was performed directly by {name}. The reason specified was: {message}
     * UI/Generic/FormatStandingTransactions/messageResetBySlashCmd: 235096: This standing was reset by a GM.
     * UI/Generic/FormatStandingTransactions/subjectPlayerSet: 235097: Player Set
     * UI/Generic/FormatStandingTransactions/messagePlayerSet: 235098: This player standing was set by the player himself/herself. The reason specified was: {message}
     * UI/Generic/FormatStandingTransactions/subjectCorporationSet: 235099: Corp Set
     * UI/Generic/FormatStandingTransactions/messageCorporationSet: 235100: This player corp standing was set by {name}. The reason specified was: {message}
     * UI/Generic/FormatStandingTransactions/subjectMissionComplete: 235101: Mission Completed - {message}
     * UI/Generic/FormatStandingTransactions/messageMissionComplete: 235102: This standing increase was granted for the successful completion of the mission '{message}'
     * UI/Generic/FormatStandingTransactions/subjectMissionDeclined: 235103: Mission Declined - {message}
     * UI/Generic/FormatStandingTransactions/messageMissionDecline: 235104: This standing penalty was incurred by declining the mission '{message}'
     * UI/Generic/FormatStandingTransactions/subjectMissionFailed: 235105: Mission Failed - {message}
     * UI/Generic/FormatStandingTransactions/messageMissionFailed: 235106: This standing penalty was incurred for failing mission '{message}'
     * UI/Generic/FormatStandingTransactions/subjectMissionExpired: 235107: Mission Offer Expired - {message}
     * UI/Generic/FormatStandingTransactions/messageMissionExpiredNoMsg: 235108: This standing penalty was incurred for not accepting a mission
     * UI/Generic/FormatStandingTransactions/messageMissionExpiredMsg: 235109: This standing penalty was incurred for not accepting mission '{message}'
     * UI/Generic/FormatStandingTransactions/subjectMissionBonus: 235110: Mission Bonus - {message}
     * UI/Generic/FormatStandingTransactions/messageMissionBonus: 235111: This standing change was granted as a bonus within the mission '{message}'
     * UI/Generic/FormatStandingTransactions/subjectMissionPenalty: 235112: Mission Penalty - {message}
     * UI/Generic/FormatStandingTransactions/messageMissionPenalty: 235113: This standing change was granted as a penalty within the mission '{message}'
     * UI/Generic/FormatStandingTransactions/subjectLawEnforcmentGain: 235114: Law Enforcement - Security Status Gain
     * UI/Generic/FormatStandingTransactions/messageLawEnforcmentGain: 235115: This standing change was granted by CONCORD as an award for actions performed against {name}
     * UI/Generic/FormatStandingTransactions/subjectFacwarPromotion: 235116: Promotion
     * UI/Generic/FormatStandingTransactions/messageFacwarPromotion: 235117: This standing change was granted for promotion to rank {rankName} within the {corpName}
     * UI/Generic/FormatStandingTransactions/messageCombatSkipKillOwnFaction: 235118: This penalty was incurred for destroying {[item]typeID.name} belonging to a member of the {factionName} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/messageCombatPodKillOwnFaction: 235119: This penalty was incurred for podding a member of the {factionName} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/messageCombatAgressionOwnFaction: 235120: This penalty was incurred for attacking {[item]typeID.name} belonging to a member of the {factionName} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/messageCombatAssistanceOwnFaction: 235121: This penalty was incurred for assisting {[item]typeID.name} belonging to a member of the {factionName} when attacking a member of your militia in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/messageCombatProprtyDamageOwnFaction: 235122: This penalty was incurred for destroying {[item]typeID.name} belonging to a member of the {factionName} in {[location]locationID.name}
     * UI/Generic/FormatStandingTransactions/subjectFacwarSiteDefened: 235127: Tactical site defended
     * UI/Generic/FormatStandingTransactions/messageFacwarSiteDefened: 235128: This standing change was granted for helping the {factionName} defend a tactical site from the {enemyFactionName}
     * UI/Generic/FormatStandingTransactions/subjectFacwarSiteConquered: 235129: Tactical site conquered
     * UI/Generic/FormatStandingTransactions/messageFacwarSiteConquered: 235130: This standing change was granted for helping the {factionName} conquer a tactical site from the {enemyFactionName}
     * UI/Generic/FormatStandingTransactions/subjectRecomendationLetterUsed: 235131: Letter of Recommendation used
     * UI/Generic/FormatStandingTransactions/messageRecomendationLetterUsed: 235132: A Letter of Recommendation used to instantly join the war effort for this faction.
     * UI/Generic/FormatStandingTransactions/subjectStandingChange: 235133: Standing Change
     * UI/Generic/FormatStandingTransactions/messageGraduation: 235134: This standing increase was granted as part of the capsuleer graduation process.
     * UI/Generic/FormatStandingTransactions/subjectContraband: 235135: Contraband
     * UI/Generic/FormatStandingTransactions/messageContraband: 235136: Your standing was lowered with the {factionName} for transporting contraband in {systemName}.
     * UI/Generic/FormatStandingTransactions/labelSomeone: 235137: Someone
     * UI/Generic/FormatStandingTransactions/labelSomewhere: 235138: Somewhere
     */
    /*Standing Events
     *  SlashHeal = 100
     *  SlashSetQty = 30
     *  SlashSpawn = 28
     *  SlashTransfer = 99
     *  SlashUnspawn = 29
     *  AddCorporation = 12
     *  AddSuper = 18
     *  Add = 1
     *  Aborted = 158
     *  AcceptApplication = 138
     *  AcceptedOffer = 326
     *  ActivateGate = 147
     *  AddAlliance = 131
     *  AddFacwar = 217
     *  AddOffice = 46
     *  AgentBuyOff = 71
     *  AgentDonation = 72
     *  AllocationFailure_ItemDeclarationError = 124
     *  AllocationFailure_ItemResolutionFailure = 123
     *  AllocationFailure_SanityCheckFailure = 122
     *  AllocationFailure_UnexpectedException = 125
     *  ApplicationClosed = 157
     *  ApplyAlliance = 133
     *  ApplyForCorporationMembership = 15
     *  AttributeRespecFree = 51
     *  AttributeRespecScheduled = 50
     *  AttributeRespecPlexTrade = 311
     *  BatchLpModification = 66
     *  BatchUpdateLP = 263
     *  BecomeExecutor = 136
     *  BecomeACeoInACorporation = 16
     *  BlueprintAccepted = 106
     *  BlueprintOfferExpired = 105
     *  BlueprintOfferInvalid = 111
     *  BlueprintOfferRejectedIncompatibleAgent = 110
     *  BlueprintOfferRejectedInvalidBlueprint = 109
     *  BlueprintOfferRejectedRecently = 108
     *  BlueprintOfferRejectedTooLowStandings = 107
     *  BlueprintOffered = 101
     *  BlueprintRejected = 102
     *  BoardedShipFrom = 184
     *  BoosterAdded = 172
     *  BoosterRemoved = 173
     *  BountySources = 325
     *  BunkerConquered = 221
     *  BunkerLost = 222
     *  CancelRemovePrepare = 41
     *  ChangeAppearance = 117
     *  ChangeEmploymentRecord = 149
     *  ChangeName = 31
     *  ChangeShortName = 127
     *  ChangeTicker = 115
     *  ChangeUser = 114
     *  CapitalStationDeclared = 202
     *  CapitalStationLost = 195
     *  CertificateGranted = 231
     *  CharacterCreationStarted = 246
     *  CharacterPaused = 305
     *  CharacterRescued = 191
     *  CharacterResumed = 306
     *  CloneDestroyedWithLocation = 190
     *  CloneDestruction = 166
     *  CloneImplantInstallation = 168
     *  CloneInstallation = 167
     *  CloneJumpTimeReset = 169
     *  CloneJump = 165
     *  Closed = 156
     *  CombatAggressionOwnFaction = 225
     *  CombatAggression = 76
     *  CombatAssistanceOwnFaction = 226
     *  CombatAssistance = 112
     *  CombatOtherOwnFaction = 228
     *  CombatOther = 79
     *  CombatPodKillOwnFaction = 224
     *  CombatPodKill = 78
     *  CombatShipKillOwnFaction = 223
     *  CombatShipKill = 77
     *  CommandUnspecified = 153
     *  CommodityExported = 298
     *  CommodityImported = 297
     *  CompleteSlashAgent = 146
     *  CompleteSlashDistribution = 176
     *  CompleteSlashPathDungeon = 179
     *  CompleteDungeon = 145
     *  CompletedTutorial = 155
     *  ConstellationCapitalSystemLost = 201
     *  ConstellationSovereigntyContested = 199
     *  ConstellationSovereigntyGained = 196
     *  ConstellationSovereigntyLost = 200
     *  ConstellationSovereigntyRecoveredInGrace = 198
     *  ContrabandTrafficking = 126
     *  ContractMarkedAsFinished = 212
     *  ContractDelete = 187
     *  ControlTowerSovereignityClaimStatusChanged = 255
     *  CreatedBySlashCommand = 358
     *  CreatedItem = 91
     *  CreditGift = 22
     *  CynosuralGeneratorArrayJump = 208
     *  DeleteBookmark = 116
     *  Deployment = 19
     *  Decay = 49
     *  DeclineApplication = 139
     *  DeclinedMission = 120
     *  DeployPermissions = 163
     *  DepositSelected = 295
     *  DividendsPayed = 193
     *  Dock = 4
     *  EditItemFrom = 141
     *  EditItemTo = 142
     *  EditItem = 140
     *  ErrorControlTower = 151
     *  EditShipName = 213
     *  EditAssemblyLine = 170
     *  EnterSlashAgent = 144
     *  EnterSlashDistribution = 175
     *  EnterSlashPathDungeon = 178
     *  EnterDungeon = 143
     *  EpicArcCompleted = 244
     *  EpicArcStarted = 243
     *  EpicArcTerminated = 261
     *  ExitTimeSetTo = 268
     *  ExitTimeChanged = 262
     *  ExitTimeChangedBy = 269
     *  ExpireSlashDistribution = 186
     *  ExpireSlashPathDungeon = 180
     *  FailSlashAgent = 310
     *  FailedMission = 87
     *  FittedItemExpired = 174
     *  ForcefieldSettings = 161
     *  FreeSkillPointsSet = 308
     *  FreeSkillPointsUsed = 307
     *  Gag = 20
     *  GaveCredits = 327
     *  GaveDogmaTypeID = 329
     *  GivenSlashPathDungeon = 181
     *  GMCalendarCcpEvent = 302
     *  GMCalendarDelete = 300
     *  GMCalendarEdit = 299
     *  GMCalendarRecover = 301
     *  GMCertificateGranted = 232
     *  GMCertificateRevoked = 233
     *  GMCommodityGift = 284
     *  GMCommodityTake = 285
     *  GMDeletionSlashPathDungeon = 182
     *  GMDepositInstall = 296
     *  GMGiftSkill = 39
     *  GMLPChanged = 205
     *  GMMailDeleteBy = 275
     *  GMMailUndeleteBy = 276
     *  GMMassMedalRemoval = 241
     *  GMMedalEdit = 242
     *  GMMedalRemoved = 240
     *  GMPinCreated = 292
     *  GMPinRemoved = 294
     *  GMResearchEdit = 189
     *  GMReverseFreeSkillPointsUsed = 309
     *  GMUnrentOffice = 211
     *  HubExploded = 281
     *  HadShipBoardedBy = 185
     *  HubInvulnerable = 267
     *  HubVulnerable = 266
     *  ImplantAdded = 94
     *  ImplantRemoved = 95
     *  InitialCorpAgent = 52
     *  InitialFactionAlly = 70
     *  InitialFactionCorp = 54
     *  InitialFactionEnemy = 69
     *  InsuranceNoPayoutGCC = 343
     *  ItemOwnerChanged = 93
     *  ItemReverseRedeemed = 62
     *  JetcanStolenFrom = 183
     *  JoinMinigame = 235
     *  JoinAlliance = 134
     *  JoinCorporation = 44
     *  Jump = 6
     *  LPExchange = 322
     *  LPRewardPoolLost = 314
     *  LPRewardPoolPayedOut = 313
     *  LPRewardStoredInPool = 312
     *  LPGainedFromMission = 203
     *  LPPaidForOffer = 204
     *  LeaveAlliance = 135
     *  LeaveCorporation = 14
     *  LeaveMinigame = 237
     *  LootGift = 23
     *  LootTrackingTookFromContainer = 214
     *  LootTrackingTookFromContainerNotTheirs = 215
     *  Lose = 236
     *  MakeSuper = 128
     *  MailingListChangeOwner = 278
     *  MailingListDelete = 277
     *  MassRedeemTokenAdded = 247
     *  MassRedeemTokenExpired = 248
     *  MassTokenClaimed = 254
     *  NpcAttackPoliceArrivedWithLowFactionStanding = 48
     *  NpcAttackPoliceArrivedWithLowSecurityStatus = 47
     *  NoLongerExecutor = 137
     *  OfferClosed = 334
     *  OfferDeleted = 335
     *  OfferEdit = 332
     *  OfferError = 331
     *  OfferExpired = 121
     *  OfferPublished = 333
     *  OfferedMission = 118
     *  OutpostInvulnerable = 265
     *  OutpostMadeInvulnerable = 271
     *  OutpostMadeVulnerable = 270
     *  OutpostVulnerable = 264
     *  OwnerChangedBy = 272
     *  OwnerSetRelationshipLevelToAContactToANewLevel = 303
     *  PinCreated = 291
     *  PinRemoved = 293
     *  Password = 160
     *  PirateKillSecurityStatus = 89
     *  PlayerCorpSetStanding = 68
     *  PlayerSetStanding = 65
     *  PlayerChange = 152
     *  PodKill = 9
     *  PodKilled = 10
     *  PortalArrayJumpMovement = 209
     *  PortalArrayJumpStarbase = 210
     *  PortedStanding = 85
     *  PrepareRemove = 40
     *  PromotionFactionStandingIncrease = 216
     *  PropertyDamageOwnFaction = 227
     *  PropertyDamage = 154
     *  QueuedSkillTrainingCompleted = 53
     *  QuitCeoPosition = 17
     *  Quit = 119
     *  Reimburse = 113
     *  RemoveCorporation = 13
     *  RemoveEmploymentRecord = 150
     *  Remove = 3
     *  Restore = 43
     *  RecalcEntityKills = 58
     *  RecalcMissionFailure = 61
     *  RecalcMissionSuccess = 55
     *  RecalcPirateKills = 57
     *  RecalcPlayerSetStanding = 67
     *  RecommendationLetterUsed = 60
     *  RedeemTokenAddedToUser = 249
     *  RedeemTokenRemovedFromUser = 250
     *  RemoveAlliance = 132
     *  RemoveFacwar = 218
     *  RemovingAContact = 304
     *  RewardDisqualified = 315
     *  RigDestroyedManuallyByPilot = 192
     *  RocketCanBurnUp = 288
     *  RocketCanClaimed = 290
     *  RocketCanLaunch = 286
     *  RocketCanSpawn = 287
     *  RocketCanUnburnedup = 289
     *  SBUExploded = 279
     *  SBUOffline = 257
     *  SBUOnline = 256
     *  Select = 2
     *  SceneAdded = 320
     *  SceneAddedToOther = 321
     *  SelfDestruct = 42
     *  SentrySettings = 162
     *  ServerPythonConsole = 188
     *  SetStatusActive = 219
     *  SetStatusLeaving = 220
     *  ShipHideCompleted = 342
     *  ShipHideExtendTimer = 341
     *  ShipHideStartTimer = 337
     *  ShipHideStartTimerModule = 340
     *  ShipHideStartTimerPve = 338
     *  ShipHideStartTimerPvp = 339
     *  ShipKill = 26
     *  ShipKilled = 27
     *  SingletonItemMadecore = 92
     *  SkillInjected = 56
     *  SkillQueueHaltedLapsed = 260
     *  SkillClonePenalty = 34
     *  SkillGift = 24
     *  SkillReceivedInCharacterCreation = 33
     *  SkillRemoval = 177
     *  SkillTaskMaster = 35
     *  SkillTrainingCancelled = 38
     *  SkillTrainingComplete = 37
     *  SkillTrainingStarted = 36
     *  SlashBlueprint = 357
     *  SlashCreateItem = 354
     *  SlashFit = 355
     *  SlashLoad = 356
     *  SlashSetStanding = 84
     *  SovereigntyClaimed = 197
     *  SovereigntyLost = 194
     *  SpawnBlockedByOtherPlayer = 59
     *  StandingCorrection = 96
     *  StandingRollback = 98
     *  StandingReset = 25
     *  StarbaseStructureControlLost = 207
     *  StarbaseStructureControlTaken = 206
     *  StartMiniGame = 239
     *  StartedResearch = 103
     *  StartedTutorial = 245
     *  StateChange = 159
     *  StationMoveSystemFull = 234
     *  StationMove = 7
     *  StoleFromJetcan = 171
     *  StoppedResearch = 104
     *  SucceededMission = 86
     *  Suicide = 11
     *  SystemInfluenceChanged = 319
     *  SystemMove = 8
     *  TacticalSiteConquered = 230
     *  TacticalSiteDefended = 229
     *  TCUExploded = 280
     *  TCUInvulnerable = 283
     *  TCUOffline = 259
     *  TCUOnline = 258
     *  TCUVulnerable = 282
     *  TaleEndedGmPlayersLose = 318
     *  TaleEndedGmPlayersWin = 324
     *  TaleEndedPlayerWins = 317
     *  TaleExpiredNoRewards = 323
     *  TaleStarted = 316
     *  TokenClaimed = 253
     *  TokenOwnershipChanged = 63
     *  TokenQuantityDecreased = 64
     *  TookCredits = 328
     *  TookDogmaTypeID = 330
     *  Transaction = 129
     *  TutorialAgentInitial = 81
     *  TypeAddedToWhitelist = 251
     *  TypeRemovedFromWhitelist = 252
     *  Undock = 5
     *  Ungag = 21
     *  UnrentOffice = 344
     *  UpdateMember = 148
     *  UpdateSkill = 32
     *  UpdateStanding = 45
     *  UpgradeInstalledBy = 273
     *  UpgradeInstalled = 274
     *  UsagePermissions = 164
     *  WinMinigame = 238
     *  WormholeJump = 130
     *  EnableBillTypeAutopay = 347
     *  DisableBillTypeAutopay = 348
     *  AutopaySuccess = 349
     *  AutopayFailure = 350
     *  ControlTowerAnchored = 364
     *  ControlTowerUnanchored = 365
     *  ControlTowerDestroyed = 366
     *  AddSkill = 10301
     *  DecreaseExtraSkillPoints = 10304
     *  IncreaseExtraSkillPoints = 10303
     *  InsertAutoRecreate = 10006
     *  Insert = 10001
     *  PrimaryMarketPurchase = 10203
     *  RemoveAllSkills = 10305
     *  RemoveSkill = 10302
     *  UpdateMachine = 10003
     *  UpdatePortSlashProcess = 10005
     *  UpdateServerStatus = 10004
     *  UpdateStatus = 10002
     *  UpdateQuantity = 10201
     *  UseQuantity = 10202
     */

}


#endif  // EVE_STANDINGS_H
