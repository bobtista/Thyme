/**
 * @file
 *
 * @author OmniBlade
 *
 * @brief Custom memory manager designed to limit OS calls to allocate heap memory.
 *
 * @copyright Thyme is free software: you can redistribute it and/or
 *            modify it under the terms of the GNU General Public License
 *            as published by the Free Software Foundation, either version
 *            2 of the License, or (at your option) any later version.
 *            A full copy of the GNU General Public License can be found in
 *            LICENSE
 */
#include "gamememoryinit.h"
#include "gamedebug.h"
#include "rawalloc.h"
#include <algorithm>
#include <cstdio>
#include <cstring>

#ifndef PLATFORM_WINDOWS
#include <libgen.h>
#include <unistd.h>
#endif

using std::strcat;
using std::strcmp;
using std::strlen;

static PoolInitRec const UserDMAParameters[7] = {
    {"dmaPool_16", 16, 130000, 10000},
    {"dmaPool_32", 32, 250000, 10000},
    {"dmaPool_64", 64, 100000, 10000},
    {"dmaPool_128", 128, 80000, 10000},
    {"dmaPool_256", 256, 20000, 5000},
    {"dmaPool_512", 512, 16000, 5000},
    {"dmaPool_1024", 1024, 6000, 1024},
};

static PoolSizeRec UserMemoryPools[] = {
    {"PartitionContactListNode", 2048, 512},
    {"BattleshipUpdate", 32, 32},
    {"FlyToDestAndDestroyUpdate", 32, 32},
    {"MusicTrack", 32, 32},
    {"PositionalSoundPool", 32, 32},
    {"GameMessage", 2048, 32},
    {"NameKeyBucketPool", 9000, 1024},
    {"ObjectSellInfo", 16, 16},
    {"ProductionPrerequisitePool", 1024, 32},
    {"RadarObject", 512, 32},
    {"ResourceGatheringManager", 16, 16},
    {"SightingInfo", 8192, 2048},
    {"SpecialPowerTemplate", 84, 32},
    {"StateMachinePool", 32, 32},
    {"TeamPool", 128, 32},
    {"PlayerRelationMapPool", 128, 32},
    {"TeamRelationMapPool", 128, 32},
    {"TeamPrototypePool", 256, 32},
    {"TerrainType", 256, 32},
    {"ThingTemplatePool", 2120, 32},
    {"TunnelTracker", 16, 16},
    {"Upgrade", 16, 16},
    {"UpgradeTemplate", 128, 16},
    {"Anim2D", 32, 32},
    {"CommandButton", 1024, 256},
    {"CommandSet", 820, 16},
    {"DisplayString", 32, 32},
    {"WebBrowserURL", 16, 16},
    {"Drawable", 4096, 32},
    {"Image", 2048, 32},
    {"ParticlePool", 1400, 1024},
    {"ParticleSystemTemplatePool", 1100, 32},
    {"ParticleSystemPool", 1024, 32},
    {"TerrainRoadType", 100, 32},
    {"WindowLayoutPool", 32, 32},
    {"AnimatedParticleSysBoneClientUpdate", 16, 16},
    {"SwayClientUpdate", 32, 32},
    {"BeaconClientUpdate", 64, 32},
    {"AIGroupPool", 64, 32},
    {"AIDockMachinePool", 256, 32},
    {"AIGuardMachinePool", 32, 32},
    {"AIGuardRetaliateMachinePool", 32, 32},
    {"AITNGuardMachinePool", 32, 32},
    {"PathNodePool", 8192, 1024},
    {"PathPool", 256, 16},
    {"WorkOrder", 32, 32},
    {"TeamInQueue", 32, 32},
    {"AIPlayer", 12, 4},
    {"AISkirmishPlayer", 8, 8},
    {"AIStateMachine", 600, 32},
    {"JetAIStateMachine", 64, 32},
    {"HeliAIStateMachine", 64, 32},
    {"AIAttackMoveStateMachine", 2048, 32},
    {"AIAttackThenIdleStateMachine", 512, 32},
    {"AttackStateMachine", 512, 32},
    {"CrateTemplate", 32, 32},
    {"ExperienceTrackerPool", 2048, 512},
    {"FiringTrackerPool", 4096, 256},
    {"ObjectRepulsorHelper", 1024, 256},
    {"ObjectSMCHelperPool", 2048, 256},
    {"ObjectWeaponStatusHelperPool", 4096, 256},
    {"ObjectDefectionHelperPool", 2048, 256},
    {"StatusDamageHelper", 1500, 256},
    {"SubdualDamageHelper", 1500, 256},
    {"TempWeaponBonusHelper", 4096, 256},
    {"Locomotor", 2048, 32},
    {"LocomotorTemplate", 192, 32},
    {"ObjectPool", 1500, 256},
    {"SimpleObjectIteratorPool", 32, 32},
    {"SimpleObjectIteratorClumpPool", 4096, 32},
    {"PartitionDataPool", 2048, 512},
    {"BuildEntry", 32, 32},
    {"Weapon", 4096, 32},
    {"WeaponTemplate", 360, 32},
    {"AIUpdateInterface", 600, 32},
    {"ActiveBody", 1024, 32},
    {"ActiveShroudUpgrade", 32, 32},
    {"AssistedTargetingUpdate", 32, 32},
    {"AudioEventInfo", 4096, 64},
    {"AudioRequest", 256, 8},
    {"AutoHealBehavior", 1024, 256},
    {"WeaponBonusUpdate", 16, 16},
    {"GrantStealthBehavior", 4096, 32},
    {"NeutronBlastBehavior", 4096, 32},
    {"CountermeasuresBehavior", 256, 32},
    {"BaseRegenerateUpdate", 128, 32},
    {"BoneFXDamage", 64, 32},
    {"BoneFXUpdate", 64, 32},
    {"BridgeBehavior", 4, 4},
    {"BridgeTowerBehavior", 32, 32},
    {"BridgeScaffoldBehavior", 32, 32},
    {"CaveContain", 16, 16},
    {"HealContain", 32, 32},
    {"CreateCrateDie", 256, 128},
    {"CreateObjectDie", 1024, 32},
    {"EjectPilotDie", 1024, 32},
    {"CrushDie", 1024, 32},
    {"DamDie", 8, 8},
    {"DeliverPayloadStateMachine", 32, 32},
    {"DeliverPayloadAIUpdate", 32, 32},
    {"DeletionUpdate", 128, 32},
    {"SmartBombTargetHomingUpdate", 8, 8},
    {"DynamicAudioEventInfo", 16, 256},
    {"HackInternetStateMachine", 32, 32},
    {"HackInternetAIUpdate", 32, 32},
    {"MissileAIUpdate", 512, 32},
    {"DumbProjectileBehavior", 64, 32},
    {"DestroyDie", 1024, 32},
    {"UpgradeDie", 128, 32},
    {"KeepObjectDie", 128, 32},
    {"DozerAIUpdate", 32, 32},
    {"DynamicGeometryInfoUpdate", 16, 16},
    {"DynamicShroudClearingRangeUpdate", 128, 16},
    {"FXListDie", 1024, 32},
    {"FireSpreadUpdate", 2048, 128},
    {"FirestormDynamicGeometryInfoUpdate", 16, 16},
    {"FireWeaponCollide", 2048, 32},
    {"FireWeaponUpdate", 32, 32},
    {"FlammableUpdate", 512, 256},
    {"FloatUpdate", 512, 128},
    {"TensileFormationUpdate", 256, 32},
    {"GarrisonContain", 256, 32},
    {"HealCrateCollide", 32, 32},
    {"HeightDieUpdate", 32, 32},
    {"FireWeaponWhenDamagedBehavior", 32, 32},
    {"FireWeaponWhenDeadBehavior", 128, 64},
    {"GenerateMinefieldBehavior", 32, 32},
    {"HelicopterSlowDeathBehavior", 64, 32},
    {"ParkingPlaceBehavior", 32, 32},
    {"FlightDeckBehavior", 8, 8},
    {"PropagandaTowerBehavior", 16, 16},
    {"BunkerBusterBehavior", 16, 16},
    {"ObjectTracker", 128, 32},
    {"OCLUpdate", 16, 16},
    {"BodyParticleSystem", 196, 64},
    {"HighlanderBody", 2048, 128},
    {"UndeadBody", 32, 32},
    {"HordeUpdate", 128, 32},
    {"ImmortalBody", 128, 256},
    {"InactiveBody", 2048, 32},
    {"InstantDeathBehavior", 512, 32},
    {"LaserUpdate", 32, 32},
    {"PointDefenseLaserUpdate", 32, 32},
    {"CleanupHazardUpdate", 32, 32},
    {"AutoFindHealingUpdate", 256, 32},
    {"CommandButtonHuntUpdate", 512, 8},
    {"PilotFindVehicleUpdate", 256, 32},
    {"DemoTrapUpdate", 32, 32},
    {"ParticleUplinkCannonUpdate", 16, 16},
    {"SpectreGunshipUpdate", 8, 8},
    {"SpectreGunshipDeploymentUpdate", 8, 8},
    {"BaikonurLaunchPower", 4, 4},
    {"RadiusDecalUpdate", 16, 16},
    {"BattlePlanUpdate", 32, 32},
    {"LifetimeUpdate", 32, 32},
    {"LocomotorSetUpgrade", 512, 128},
    {"LockWeaponCreate", 64, 128},
    {"AutoDepositUpdate", 256, 32},
    {"NeutronMissileUpdate", 512, 32},
    {"MoneyCrateCollide", 48, 16},
    {"NeutronMissileSlowDeathBehavior", 8, 8},
    {"OpenContain", 128, 32},
    {"OverchargeBehavior", 32, 32},
    {"OverlordContain", 32, 32},
    {"HelixContain", 32, 32},
    {"ParachuteContain", 128, 32},
    {"PhysicsBehavior", 600, 32},
    {"PoisonedBehavior", 512, 64},
    {"ProductionEntry", 32, 32},
    {"ProductionUpdate", 256, 32},
    {"ProjectileStreamUpdate", 32, 32},
    {"ProneUpdate", 128, 32},
    {"QueueProductionExitUpdate", 32, 32},
    {"RadarUpdate", 16, 16},
    {"RadarUpgrade", 16, 16},
    {"AnimationSteeringUpdate", 1024, 32},
    {"SupplyWarehouseCripplingBehavior", 16, 16},
    {"CostModifierUpgrade", 32, 32},
    {"CashBountyPower", 32, 32},
    {"CleanupAreaPower", 32, 32},
    {"ObjectCreationUpgrade", 196, 32},
    {"MinefieldBehavior", 256, 32},
    {"JetSlowDeathBehavior", 64, 32},
    {"BattleBusSlowDeathBehavior", 64, 32},
    {"RebuildHoleBehavior", 64, 32},
    {"RebuildHoleExposeDie", 64, 32},
    {"RepairDockUpdate", 32, 32},
    {"RailedTransportDockUpdate", 16, 16},
    {"RailedTransportAIUpdate", 16, 16},
    {"RailedTransportContain", 16, 16},
    {"RailroadBehavior", 16, 16},
    {"SalvageCrateCollide", 32, 32},
    {"ShroudCrateCollide", 32, 32},
    {"SlavedUpdate", 64, 32},
    {"SlowDeathBehavior", 1400, 256},
    {"SpyVisionUpdate", 16, 16},
    {"DefaultProductionExitUpdate", 32, 32},
    {"SpawnPointProductionExitUpdate", 32, 32},
    {"SpawnBehavior", 32, 32},
    {"SpecialPowerCompletionDie", 32, 32},
    {"SpecialPowerCreate", 32, 32},
    {"PreorderCreate", 32, 32},
    {"SpecialAbility", 512, 32},
    {"SpecialAbilityUpdate", 512, 32},
    {"MissileLauncherBuildingUpdate", 32, 32},
    {"SquishCollide", 512, 32},
    {"StructureBody", 512, 64},
    {"HiveStructureBody", 64, 32},
    {"StructureCollapseUpdate", 32, 32},
    {"StructureToppleUpdate", 32, 32},
    {"SupplyCenterCreate", 32, 32},
    {"SupplyCenterDockUpdate", 32, 32},
    {"SupplyCenterProductionExitUpdate", 32, 32},
    {"SupplyTruckStateMachine", 256, 32},
    {"SupplyTruckAIUpdate", 32, 32},
    {"SupplyWarehouseCreate", 48, 16},
    {"SupplyWarehouseDockUpdate", 48, 16},
    {"EnemyNearUpdate", 1024, 32},
    {"TechBuildingBehavior", 32, 32},
    {"ToppleUpdate", 256, 128},
    {"TransitionDamageFX", 384, 128},
    {"TransportAIUpdate", 64, 32},
    {"TransportContain", 128, 32},
    {"RiderChangeContain", 128, 32},
    {"InternetHackContain", 16, 16},
    {"TunnelContain", 8, 8},
    {"TunnelContainDie", 32, 32},
    {"TunnelCreate", 32, 32},
    {"TurretAI", 256, 32},
    {"TurretStateMachine", 128, 32},
    {"TurretSwapUpgrade", 512, 128},
    {"UnitCrateCollide", 32, 32},
    {"UnpauseSpecialPowerUpgrade", 32, 32},
    {"VeterancyCrateCollide", 32, 32},
    {"VeterancyGainCreate", 512, 128},
    {"ConvertToCarBombCrateCollide", 256, 128},
    {"ConvertToHijackedVehicleCrateCollide", 256, 128},
    {"SabotageCommandCenterCrateCollide", 256, 128},
    {"SabotageFakeBuildingCrateCollide", 256, 128},
    {"SabotageInternetCenterCrateCollide", 256, 128},
    {"SabotageMilitaryFactoryCrateCollide", 256, 128},
    {"SabotagePowerPlantCrateCollide", 256, 128},
    {"SabotageSuperweaponCrateCollide", 256, 128},
    {"SabotageSupplyCenterCrateCollide", 256, 128},
    {"SabotageSupplyDropzoneCrateCollide", 256, 128},
    {"JetAIUpdate", 64, 32},
    {"ChinookAIUpdate", 32, 32},
    {"WanderAIUpdate", 32, 32},
    {"WaveGuideUpdate", 16, 16},
    {"WeaponBonusUpgrade", 512, 128},
    {"WeaponSetUpgrade", 512, 128},
    {"ArmorUpgrade", 512, 128},
    {"WorkerAIUpdate", 128, 128},
    {"WorkerStateMachine", 128, 128},
    {"ChinookAIStateMachine", 32, 32},
    {"DeployStyleAIUpdate", 32, 32},
    {"AssaultTransportAIUpdate", 64, 32},
    {"StreamingArchiveFile", 8, 8},
    {"DozerActionStateMachine", 256, 32},
    {"DozerPrimaryStateMachine", 256, 32},
    {"W3DDisplayString", 1400, 128},
    {"W3DDefaultDraw", 1024, 128},
    {"W3DDebrisDraw", 128, 128},
    {"W3DDependencyModelDraw", 64, 64},
    {"W3DLaserDraw", 32, 32},
    {"W3DModelDraw", 2048, 512},
    {"W3DOverlordTankDraw", 64, 64},
    {"W3DOverlordTruckDraw", 64, 64},
    {"W3DOverlordAircraftDraw", 64, 64},
    {"W3DPoliceCarDraw", 32, 32},
    {"W3DProjectileStreamDraw", 32, 32},
    {"W3DRopeDraw", 32, 32},
    {"W3DScienceModelDraw", 32, 32},
    {"W3DSupplyDraw", 40, 16},
    {"W3DTankDraw", 256, 32},
    {"W3DTreeDraw", 16, 16},
    {"W3DPropDraw", 16, 16},
    {"W3DTracerDraw", 64, 32},
    {"W3DTruckDraw", 128, 32},
    {"W3DTankTruckDraw", 32, 16},
    {"W3DTreeTextureClass", 4, 4},
    {"DefaultSpecialPower", 32, 32},
    {"OCLSpecialPower", 96, 32},
    {"FireWeaponPower", 32, 32},
    {"CashHackSpecialPower", 32, 32},
    {"CommandSetUpgrade", 32, 32},
    {"PassengersFireUpgrade", 32, 32},
    {"GrantUpgradeCreate", 256, 32},
    {"GrantScienceUpgrade", 256, 32},
    {"ReplaceObjectUpgrade", 32, 32},
    {"ModelConditionUpgrade", 32, 32},
    {"SpyVisionSpecialPower", 256, 32},
    {"StealthDetectorUpdate", 256, 32},
    {"StealthUpdate", 512, 128},
    {"StealthUpgrade", 256, 32},
    {"StatusBitsUpgrade", 128, 128},
    {"SubObjectsUpgrade", 128, 128},
    {"ExperienceScalarUpgrade", 256, 128},
    {"MaxHealthUpgrade", 128, 128},
    {"WeaponBonusUpgrade", 128, 64},
    {"StickyBombUpdate", 64, 32},
    {"FireOCLAfterWeaponCooldownUpdate", 64, 32},
    {"HijackerUpdate", 64, 32},
    {"ChinaMinesUpgrade", 64, 32},
    {"PowerPlantUpdate", 48, 16},
    {"PowerPlantUpgrade", 48, 16},
    {"DefectorSpecialPower", 16, 16},
    {"CheckpointUpdate", 16, 16},
    {"MobNexusContain", 128, 32},
    {"MobMemberSlavedUpdate", 64, 32},
    {"EMPUpdate", 64, 32},
    {"LeafletDropBehavior", 64, 32},
    {"Overridable", 32, 32},
    {"W3DGameWindow", 700, 256},
    {"SuccessState", 32, 32},
    {"FailureState", 32, 32},
    {"ContinueState", 32, 32},
    {"SleepState", 32, 32},
    {"AIDockWaitForClearanceState", 256, 32},
    {"AIDockProcessDockState", 256, 32},
    {"AIGuardInnerState", 32, 32},
    {"AIGuardIdleState", 32, 32},
    {"AIGuardOuterState", 32, 32},
    {"AIGuardReturnState", 32, 32},
    {"AIGuardPickUpCrateState", 32, 32},
    {"AIGuardAttackAggressorState", 32, 32},
    {"AIGuardRetaliateInnerState", 32, 32},
    {"AIGuardRetaliateIdleState", 32, 32},
    {"AIGuardRetaliateOuterState", 32, 32},
    {"AIGuardRetaliateReturnState", 32, 32},
    {"AIGuardRetaliatePickUpCrateState", 32, 32},
    {"AIGuardRetaliateAttackAggressorState", 32, 32},
    {"AITNGuardInnerState", 32, 32},
    {"AITNGuardIdleState", 32, 32},
    {"AITNGuardOuterState", 32, 32},
    {"AITNGuardReturnState", 32, 32},
    {"AITNGuardPickUpCrateState", 32, 32},
    {"AITNGuardAttackAggressorState", 32, 32},
    {"AIIdleState", 2400, 32},
    {"AIRappelState", 600, 32},
    {"AIBusyState", 600, 32},
    {"AIWaitState", 600, 32},
    {"AIAttackState", 4096, 32},
    {"AIAttackSquadState", 600, 32},
    {"AIDeadState", 600, 32},
    {"AIDockState", 600, 32},
    {"AIExitState", 600, 32},
    {"AIExitInstantlyState", 600, 32},
    {"AIGuardState", 600, 32},
    {"AIGuardRetaliateState", 600, 32},
    {"AITunnelNetworkGuardState", 600, 32},
    {"AIHuntState", 600, 32},
    {"AIAttackAreaState", 600, 32},
    {"AIFaceState", 1200, 32},
    {"ApproachState", 600, 32},
    {"DeliveringState", 600, 32},
    {"ConsiderNewApproachState", 600, 32},
    {"RecoverFromOffMapState", 600, 32},
    {"HeadOffMapState", 600, 32},
    {"CleanUpState", 600, 32},
    {"HackInternetState", 600, 32},
    {"PackingState", 600, 32},
    {"UnpackingState", 600, 32},
    {"SupplyTruckWantsToPickUpOrDeliverBoxesState", 600, 32},
    {"RegroupingState", 600, 32},
    {"DockingState", 600, 32},
    {"ChinookEvacuateState", 32, 32},
    {"ChinookHeadOffMapState", 32, 32},
    {"ChinookTakeoffOrLandingState", 32, 32},
    {"ChinookCombatDropState", 32, 32},
    {"DozerActionPickActionPosState", 256, 32},
    {"DozerActionMoveToActionPosState", 256, 32},
    {"DozerActionDoActionState", 256, 32},
    {"DozerPrimaryIdleState", 256, 32},
    {"DozerActionState", 256, 32},
    {"DozerPrimaryGoingHomeState", 256, 32},
    {"JetAwaitingRunwayState", 64, 32},
    {"JetOrHeliCirclingDeadAirfieldState", 64, 32},
    {"HeliTakeoffOrLandingState", 64, 32},
    {"JetOrHeliParkOrientState", 64, 32},
    {"JetOrHeliReloadAmmoState", 64, 32},
    {"SupplyTruckBusyState", 600, 32},
    {"SupplyTruckIdleState", 600, 32},
    {"ActAsDozerState", 600, 32},
    {"ActAsSupplyTruckState", 600, 32},
    {"AIDockApproachState", 256, 32},
    {"AIDockAdvancePositionState", 256, 32},
    {"AIDockMoveToEntryState", 256, 32},
    {"AIDockMoveToDockState", 256, 32},
    {"AIDockMoveToExitState", 256, 32},
    {"AIDockMoveToRallyState", 256, 32},
    {"AIMoveToState", 600, 32},
    {"AIMoveOutOfTheWayState", 600, 32},
    {"AIMoveAndTightenState", 600, 32},
    {"AIMoveAwayFromRepulsorsState", 600, 32},
    {"AIAttackApproachTargetState", 96, 32},
    {"AIAttackPursueTargetState", 96, 32},
    {"AIAttackAimAtTargetState", 96, 32},
    {"AIAttackFireWeaponState", 256, 32},
    {"AIPickUpCrateState", 4096, 32},
    {"AIFollowWaypointPathState", 1200, 32},
    {"AIFollowWaypointPathExactState", 1200, 32},
    {"AIWanderInPlaceState", 600, 32},
    {"AIFollowPathState", 1200, 32},
    {"AIMoveAndEvacuateState", 1200, 32},
    {"AIMoveAndDeleteState", 600, 32},
    {"AIEnterState", 600, 32},
    {"JetOrHeliReturningToDeadAirfieldState", 64, 32},
    {"JetOrHeliReturnForLandingState", 64, 32},
    {"TurretAIIdleState", 600, 32},
    {"TurretAIIdleScanState", 600, 32},
    {"TurretAIAimTurretState", 600, 32},
    {"TurretAIRecenterTurretState", 600, 32},
    {"TurretAIHoldTurretState", 600, 32},
    {"JetOrHeliTaxiState", 64, 32},
    {"JetTakeoffOrLandingState", 64, 32},
    {"JetPauseBeforeTakeoffState", 64, 32},
    {"AIAttackMoveToState", 600, 32},
    {"AIAttackFollowWaypointPathState", 1200, 32},
    {"AIWanderState", 600, 32},
    {"AIPanicState", 600, 32},
    {"ChinookMoveToBldgState", 32, 32},
    {"ChinookRecordCreationState", 32, 32},
    {"ScienceInfo", 96, 32},
    {"RankInfo", 32, 32},
    {"FireWeaponNugget", 32, 32},
    {"AttackNugget", 32, 32},
    {"DeliverPayloadNugget", 48, 32},
    {"ApplyRandomForceNugget", 32, 32},
    {"GenericObjectCreationNugget", 632, 32},
    {"SoundFXNugget", 320, 32},
    {"TracerFXNugget", 32, 32},
    {"RayEffectFXNugget", 32, 32},
    {"LightPulseFXNugget", 68, 32},
    {"ViewShakeFXNugget", 140, 32},
    {"TerrainScorchFXNugget", 48, 32},
    {"ParticleSystemFXNugget", 832, 32},
    {"FXListAtBonePosFXNugget", 32, 32},
    {"Squad", 256, 32},
    {"BuildListInfo", 400, 64},
    {"ScriptGroup", 128, 32},
    {"OrCondition", 1024, 256},
    {"ScriptAction", 2600, 512},
    {"Script", 1024, 256},
    {"Parameter", 8192, 1024},
    {"Condition", 2048, 256},
    {"Template", 32, 32},
    {"ScriptList", 32, 32},
    {"AttackPriorityInfo", 32, 32},
    {"SequentialScript", 32, 32},
    {"Win32LocalFile", 1024, 256},
    {"RAMFile", 32, 32},
    {"BattlePlanBonuses", 32, 32},
    {"KindOfPercentProductionChange", 32, 32},
    {"UserParser", 4096, 256},
    {"XferBlockData", 32, 32},
    {"EvaCheckInfo", 52, 16},
    {"SuperweaponInfo", 32, 32},
    {"NamedTimerInfo", 32, 32},
    {"PopupMessageData", 32, 32},
    {"FloatingTextData", 32, 32},
    {"MapObject", 5000, 1024},
    {"Waypoint", 1024, 32},
    {"PolygonTrigger", 64, 64},
    {"Bridge", 32, 32},
    {"Mapping", 384, 64},
    {"OutputChunk", 32, 32},
    {"InputChunk", 32, 32},
    {"AnimateWindow", 32, 32},
    {"GameFont", 32, 32},
    {"NetCommandRef", 256, 32},
    {"GameMessageArgument", 1024, 256},
    {"GameMessageParserArgumentType", 32, 32},
    {"GameMessageParser", 32, 32},
    {"WeaponBonusSet", 96, 32},
    {"Campaign", 32, 32},
    {"Mission", 88, 32},
    {"ModalWindow", 32, 32},
    {"NetPacket", 32, 32},
    {"AISideInfo", 32, 32},
    {"AISideBuildList", 32, 32},
    {"MetaMapRec", 256, 32},
    {"TransportStatus", 32, 32},
    {"Anim2DTemplate", 32, 32},
    {"ObjectTypes", 32, 32},
    {"NetCommandList", 512, 32},
    {"TurretAIData", 256, 32},
    {"NetCommandMsg", 32, 32},
    {"NetGameCommandMsg", 64, 32},
    {"NetAckBothCommandMsg", 32, 32},
    {"NetAckStage1CommandMsg", 32, 32},
    {"NetAckStage2CommandMsg", 32, 32},
    {"NetFrameCommandMsg", 32, 32},
    {"NetPlayerLeaveCommandMsg", 32, 32},
    {"NetRunAheadMetricsCommandMsg", 32, 32},
    {"NetRunAheadCommandMsg", 32, 32},
    {"NetDestroyPlayerCommandMsg", 32, 32},
    {"NetDisconnectFrameCommandMsg", 32, 32},
    {"NetDisconnectScreenOffCommandMsg", 32, 32},
    {"NetFrameResendRequestCommandMsg", 32, 32},
    {"NetKeepAliveCommandMsg", 32, 32},
    {"NetDisconnectKeepAliveCommandMsg", 32, 32},
    {"NetDisconnectPlayerCommandMsg", 32, 32},
    {"NetPacketRouterQueryCommandMsg", 32, 32},
    {"NetPacketRouterAckCommandMsg", 32, 32},
    {"NetDisconnectChatCommandMsg", 32, 32},
    {"NetChatCommandMsg", 32, 32},
    {"NetDisconnectVoteCommandMsg", 32, 32},
    {"NetProgressCommandMsg", 32, 32},
    {"NetWrapperCommandMsg", 32, 32},
    {"NetFileCommandMsg", 32, 32},
    {"NetFileAnnounceCommandMsg", 32, 32},
    {"NetFileProgressCommandMsg", 32, 32},
    {"NetCommandWrapperListNode", 32, 32},
    {"NetCommandWrapperList", 32, 32},
    {"Connection", 32, 32},
    {"User", 32, 32},
    {"FrameDataManager", 32, 32},
    {"DrawableIconInfo", 32, 32},
    {"TintEnvelope", 128, 32},
    {"DynamicAudioEventRTS", 4000, 256},
    {"DrawableLocoInfo", 128, 32},
    {"W3DPrototypeClass", 512, 256},
    {"EnumeratedIP", 32, 32},
    {"WaterTransparencySetting", 4, 4},
    {"WeatherSetting", 4, 4},
    {"BoxPrototypeClass", 128, 128},
    {"SpherePrototypeClass", 32, 32},
    {"SoundRenderObjPrototypeClass", 32, 32},
    {"RingPrototypeClass", 32, 32},
    {"PrimitivePrototypeClass", 8192, 32},
    {"HModelPrototypeClass", 256, 32},
    {"ParticleEmitterPrototypeClass", 32, 32},
    {"NullPrototypeClass", 32, 32},
    {"HLodPrototypeClass", 700, 128},
    {"HLodDefClass", 700, 128},
    {"DistLODPrototypeClass", 32, 32},
    {"DazzlePrototypeClass", 32, 32},
    {"CollectionPrototypeClass", 32, 32},
    {"BoxPrototypeClass", 256, 32},
    {"AggregatePrototypeClass", 32, 32},
    {"OBBoxRenderObjClass", 512, 128},
    {"AABoxRenderObjClass", 32, 32},
    {"VertexMaterialClass", 6000, 2048},
    {"TextureClass", 1200, 256},
    {"CloudMapTerrainTextureClass", 4, 4},
    {"ScorchTextureClass", 4, 4},
    {"LightMapTerrainTextureClass", 4, 4},
    {"AlphaEdgeTextureClass", 4, 4},
    {"AlphaTerrainTextureClass", 4, 4},
    {"TerrainTextureClass", 4, 4},
    {"MeshClass", 14000, 2000},
    {"HTreeClass", 2048, 512},
    {"HLodClass", 2048, 512},
    {"MeshModelClass", 8192, 32},
    {"ShareBufferClass", 32768, 1024},
    {"AABTreeClass", 300, 128},
    {"MotionChannelClass", 16384, 32},
    {"BitChannelClass", 84, 32},
    {"TimeCodedMotionChannelClass", 116, 32},
    {"AdaptiveDeltaMotionChannelClass", 32, 32},
    {"TimeCodedBitChannelClass", 32, 32},
    {"UVBufferClass", 8192, 32},
    {"TexBufferClass", 384, 128},
    {"MatBufferClass", 256, 128},
    {"MatrixMapperClass", 32, 32},
    {"ScaleTextureMapperClass", 32, 32},
    {"LinearOffsetTextureMapperClass", 96, 32},
    {"GridTextureMapperClass", 32, 32},
    {"RotateTextureMapperClass", 32, 32},
    {"SineLinearOffsetTextureMapperClass", 32, 32},
    {"StepLinearOffsetTextureMapperClass", 32, 32},
    {"ZigZagLinearOffsetTextureMapperClass", 32, 32},
    {"ClassicEnvironmentMapperClass", 32, 32},
    {"EnvironmentMapperClass", 256, 32},
    {"EdgeMapperClass", 32, 32},
    {"WSClassicEnvironmentMapperClass", 32, 32},
    {"WSEnvironmentMapperClass", 32, 32},
    {"GridClassicEnvironmentMapperClass", 32, 32},
    {"GridEnvironmentMapperClass", 32, 32},
    {"ScreenMapperClass", 32, 32},
    {"RandomTextureMapperClass", 32, 32},
    {"BumpEnvTextureMapperClass", 32, 32},
    {"MeshLoadContextClass", 4, 4},
    {"MaterialInfoClass", 8192, 32},
    {"MeshMatDescClass", 8192, 32},
    {"TextureLoadTaskClass", 256, 32},
    {"SortingNodeStruct", 288, 32},
    {"ProxyArrayClass", 32, 32},
    {"Line3DClass", 8, 8},
    {"Render2DClass", 64, 32},
    {"SurfaceClass", 128, 32},
    {"FontCharsClassCharDataStruct", 1024, 32},
    {"FontCharsBuffer", 16, 4},
    {"FVFInfoClass", 152, 64},
    {"TerrainTracksRenderObjClass", 128, 32},
    {"DynamicIBAccessClass", 32, 32},
    {"DX8IndexBufferClass", 128, 32},
    {"SortingIndexBufferClass", 32, 32},
    {"DX8VertexBufferClass", 128, 32},
    {"SortingVertexBufferClass", 32, 32},
    {"DynD3DMATERIAL8", 8192, 32},
    {"DynamicMatrix3D", 512, 32},
    {"MeshGeometryClass", 32, 32},
    {"DynamicMeshModel", 32, 32},
    {"GapFillerClass", 32, 32},
    {"FontCharsClass", 64, 32},
    {"ThumbnailManagerClass", 32, 32},
    {"SmudgeSet", 32, 32},
    {"Smudge", 128, 32},
    {nullptr, 0, 0} // Last entry always null.
};

void User_Memory_Adjust_Pool_Size(const char *name, int &initial_alloc, int &overflow_alloc)
{
    if (initial_alloc > 0) {
        return;
    }

    for (PoolSizeRec *psr = UserMemoryPools; psr->pool_name != nullptr; ++psr) {
        if (strcmp(psr->pool_name, name) == 0) {
            initial_alloc = psr->initial_allocation_count;
            overflow_alloc = psr->overflow_allocation_count;
        }
    }
}

void User_Memory_Get_DMA_Params(int *count, PoolInitRec const **params)
{
    // DEBUG_LOG("Retrieving user DynamicMemoryAllocator parameters.\n");

    *count = 7; // Array size UserDMAParameters
    *params = UserDMAParameters;
}

void User_Memory_Init_Pools()
{
    char path[PATH_MAX];
    char pool_name[256];
    int initial_alloc;
    int overflow_alloc;

#ifdef PLATFORM_WINDOWS
    GetModuleFileNameA(0, path, PATH_MAX);
#elif defined PLATFORM_LINUX // posix otherwise, really just linux currently
    // TODO /proc/curproc/file for FreeBSD /proc/self/path/a.out Solaris
    readlink("/proc/self/exe", path, PATH_MAX);
    dirname(path);
#elif defined(PLATFORM_OSX) // osx otherwise
    int size = PATH_MAX;
    _NSGetExecutablePath(path, &size);
    dirname(path);
#else //
#error Platform not supported for Set_Working_Directory()!
#endif

    // Get path to current exe without filename.
    char *path_end = &path[strlen(path)];

    if (path_end != path) {
        while (path_end != path) {
            if (*path_end == '\\' || *path_end == '/') {
                break;
            }

            --path_end;
        }

        // Replace path separator with null teminator.
        *path_end = '\0';
    }

    // Get the path to the user configurable memory pool ini.
    strcat(path, "/Data/INI/MemoryPools.ini");

    FILE *fp = fopen(path, "r");

    // Go through file and match entries against internal table and update
    // table as needed. If a pool name is specified twice, last entry wins.
    if (fp != nullptr) {
        while (fgets(path, PATH_MAX, fp) != nullptr) {
            if (*path != ';' && sscanf(path, "%s %d %d", pool_name, &initial_alloc, &overflow_alloc) == 3) {
                for (PoolSizeRec *psr = UserMemoryPools; psr->pool_name != nullptr; ++psr) {
                    if (strcasecmp(psr->pool_name, pool_name) == 0) {
                        psr->initial_allocation_count = std::max((int)sizeof(void *), Round_Up_Word_Size(initial_alloc));
                        psr->overflow_allocation_count = std::max((int)sizeof(void *), Round_Up_Word_Size(overflow_alloc));
                    }
                }
            }
        }
    }
}
