#if WITH_DEV_AUTOMATION_TESTS

#include "Data/ItemData.h"
#include "Data/LiminalGameInstance.h"
#include "Data/QuotaManager.h"
#include "Misc/AutomationTest.h"
#include "ProcGen/LiminalLayoutLibrary.h"
#include "ProcGen/LiminalLevelGenerator.h"
#include "ProcGen/LiminalModularDungeonGenerator.h"
#include "ProcGen/LiminalTileSet.h"
#include "Audio/LiminalAudioSubsystem.h"
#include "Sanity/LiminalSanityTypes.h"
#include "AI/VoiceMimicryComponent.h"
#include "AI/LiminalEntity_Duller.h"
#include "AI/LiminalEntity_Partygoer.h"
#include "AI/LiminalEntity_Jerry.h"
#include "Tools/LidarScannerTool.h"
#include "Tools/SignalAnalyzerTool.h"
#include "Tools/RealityAnchorTool.h"
#include "Tools/TetherTool.h"
#include "Tools/ChalkMarkerTool.h"
#include "Hub/LiminalHubProgressionComponent.h"
#include "GameModes/LiminalGameMode.h"
#include "Objects/LiminalTerminalActor.h"
#include "Player/ScavengerCharacter.h"
#include "Player/LiminalSpectatorPawn.h"
#include "Tools/AudioDecoyTool.h"
#include "AI/LiminalEntity_Smiler.h"
#include "Tools/ChalkTraceActor.h"
#include "Objects/LiminalAirlockActor.h"
#include "Objects/LiminalKeypadActor.h"
#include "Objects/LiminalBreakerActor.h"
#include "Tools/AdrenalineInjectorTool.h"
#include "UI/LiminalScavengerHUD.h"
#include "UI/LiminalDeathScreenHUD.h"
#include "Objects/ExtractionZone.h"
#include "Objects/LiminalValvePuzzleActor.h"
#include "Objects/LiminalFuseBoxActor.h"
#include "Tools/WalkieTalkieTool.h"
#include "Player/LiminalBodycamComponent.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "Editor/MegFullGameAutomator.h"
#include "Editor/HoundAIBuilder.h"
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegQuotaManagerTest, "Project.Functional Tests.MEG.QuotaManager",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegQuotaManagerTest::RunTest(const FString& Parameters)
{
	FQuotaState Quota;

	Quota.Required = FQuotaLogic::ClampTarget(100);
	TestEqual("Livraison initiale a zero", Quota.Delivered, 0);
	TestEqual("Dette initiale a zero", Quota.OutstandingDebt, 0);
	TestFalse("Quota non atteint au depart", FQuotaLogic::IsMet(Quota));

	FQuotaLogic::AddDelivered(Quota, 40);
	TestEqual("Livraison apres 40", Quota.Delivered, 40);
	TestFalse("Quota non atteint avec 40/100", FQuotaLogic::IsMet(Quota));

	FQuotaLogic::AddDelivered(Quota, 70);
	TestTrue("Quota atteint avec 110/100", FQuotaLogic::IsMet(Quota));

	FQuotaLogic::ApplyFailure(Quota);
	TestEqual("Pas de dette si sur-livre", Quota.OutstandingDebt, 0);

	const int32 BeforeNegative = Quota.Delivered;
	FQuotaLogic::AddDelivered(Quota, -5);
	TestEqual("Valeur negative ignoree", Quota.Delivered, BeforeNegative);

	Quota.Required = FQuotaLogic::ClampTarget(-50);
	TestEqual("Cible negative clamp a zero", Quota.Required, 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegQuotaDebtTest, "Project.Functional Tests.MEG.QuotaDebt",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegQuotaDebtTest::RunTest(const FString& Parameters)
{
	FQuotaState Quota;
	Quota.Required = FQuotaLogic::ClampTarget(100);

	FQuotaLogic::AddDelivered(Quota, 30);
	TestFalse("Echec du cycle", FQuotaLogic::IsMet(Quota));

	FQuotaLogic::ApplyFailure(Quota);
	TestEqual("Dette = manque (70)", Quota.OutstandingDebt, 70);

	FQuotaLogic::AddDelivered(Quota, 80);
	TestEqual("Total du = cible + dette", FQuotaLogic::TotalDue(Quota), 170);
	TestFalse("110 livres face a 170 dus", FQuotaLogic::IsMet(Quota));

	FQuotaLogic::AddDelivered(Quota, 50);
	TestEqual("160 livres au total", Quota.Delivered, 160);
	TestFalse("Encore insuffisant face a la dette", FQuotaLogic::IsMet(Quota));

	FQuotaLogic::AddDelivered(Quota, 30);
	TestTrue("Quota + dette soldees (190 >= 170)", FQuotaLogic::IsMet(Quota));

	// Validation mathématique de la formule de quota canonique M.E.G.
	const int32 Cycle1Solo = FQuotaLogic::CalculateCycleQuota(1, 1);
	TestEqual("Cycle 1 Solo = 180 BR", Cycle1Solo, 180);

	const int32 Cycle1Squad = FQuotaLogic::CalculateCycleQuota(1, 4);
	TestEqual("Cycle 1 Escouade 4 = 315 BR", Cycle1Squad, 315);

	const int32 Cycle2Solo = FQuotaLogic::CalculateCycleQuota(2, 1);
	TestEqual("Cycle 2 Solo = 292 BR", Cycle2Solo, 292);

	const int32 Cycle2Squad = FQuotaLogic::CalculateCycleQuota(2, 4);
	TestEqual("Cycle 2 Escouade 4 = 427 BR", Cycle2Squad, 427);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegProcGenDeterminismTest, "Project.Functional Tests.MEG.ProcGenDeterminism",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegProcGenDeterminismTest::RunTest(const FString& Parameters)
{
	const FGeneratedLayout A = FLiminalLayoutBuilder::Generate(1234, 24, 24, 8, 3, 6, 0.25f);
	const FGeneratedLayout B = FLiminalLayoutBuilder::Generate(1234, 24, 24, 8, 3, 6, 0.25f);

	TestEqual("Meme seed => meme hash", A.Hash, B.Hash);
	TestEqual("Meme nombre de cellules", A.Cells.Num(), B.Cells.Num());

	bool bIdentical = true;
	for (int32 Index = 0; Index < A.Cells.Num(); ++Index)
	{
		if (A.Cells[Index] != B.Cells[Index])
		{
			bIdentical = false;
			break;
		}
	}
	TestTrue("Layouts bit-a-bit identiques", bIdentical);
	TestEqual("Salles generees", A.Rooms.Num(), 8);

	const FGeneratedLayout C = FLiminalLayoutBuilder::Generate(4321, 24, 24, 8, 3, 6, 0.25f);
	TestNotEqual("Seed differente => hash different", C.Hash, A.Hash);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegProcGenConnectivityTest, "Project.Functional Tests.MEG.ProcGenConnectivity",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegProcGenConnectivityTest::RunTest(const FString& Parameters)
{
	for (int32 Seed = 1; Seed <= 10; ++Seed)
	{
		const FGeneratedLayout Layout = FLiminalLayoutBuilder::Generate(Seed, 24, 24, 8, 3, 6, 0.25f);
		TestTrue(FString::Printf(TEXT("Seed %d : toutes les salles connectees"), Seed),
			Layout.IsEveryRoomConnected());
		TestTrue(FString::Printf(TEXT("Seed %d : salles suffisantes"), Seed), Layout.Rooms.Num() >= 2);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegLootItemDataTableTest, "Project.Functional Tests.MEG.LootItemDataTable",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegLootItemDataTableTest::RunTest(const FString& Parameters)
{
	FLootItem Item;
	Item.ItemId = FName("Scrap_Electronics");
	Item.DisplayName = FText::FromString("Composants Electroniques");
	Item.WeightKg = 8.5f;
	Item.CreditsValue = 120;

	TestEqual("ItemId correct", Item.ItemId, FName("Scrap_Electronics"));
	TestEqual("Poids positif", Item.WeightKg, 8.5f);
	TestEqual("Valeur credits positive", Item.CreditsValue, 120);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegProcGenRoomPlacementTest, "Project.Functional Tests.MEG.ProcGenRoomPlacement",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegProcGenRoomPlacementTest::RunTest(const FString& Parameters)
{
	const int32 Width = 32;
	const int32 Height = 32;
	const FGeneratedLayout Layout = FLiminalLayoutBuilder::Generate(999, Width, Height, 10, 3, 7, 0.3f);

	TestTrue("Au moins une salle generee", Layout.Rooms.Num() > 0);

	for (const FProcRoom& Room : Layout.Rooms)
	{
		TestTrue("Salle dans les limites X", Room.OriginX >= 1 && (Room.OriginX + Room.SizeX) < Width - 1);
		TestTrue("Salle dans les limites Y", Room.OriginY >= 1 && (Room.OriginY + Room.SizeY) < Height - 1);
		TestTrue("Dimensions valides", Room.SizeX >= 3 && Room.SizeY >= 3);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegSaveDataTest, "Project.Functional Tests.MEG.SaveData",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegSaveDataTest::RunTest(const FString& Parameters)
{
	FLiminalSaveData Save;
	TestTrue("Credits de depart suffisants", Save.TotalBankCredits >= 0);
	TestEqual("Cycle de depart a 1", Save.ActiveQuotaCycle, 1);
	TestEqual("Dette initiale a 0", Save.CurrentDebt, 0);

	Save.TotalBankCredits += 300;
	TestEqual("Ajout de credits", Save.TotalBankCredits, 550);

	Save.TotalBankCredits = FMath::Max(Save.TotalBankCredits - 150, 0);
	TestEqual("Depense de credits", Save.TotalBankCredits, 400);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegMultiBiomeLayoutTest, "Project.Functional Tests.MEG.MultiBiomeLayout",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegMultiBiomeLayoutTest::RunTest(const FString& Parameters)
{
	// Verifie que la generation fonctionne pour tous les biomes sans regression
	const int32 BiomeSeeds[] = { 101, 202, 303 };
	for (int32 Seed : BiomeSeeds)
	{
		const FGeneratedLayout Layout = FLiminalLayoutBuilder::Generate(Seed, 28, 28, 9, 3, 6, 0.2f);
		TestTrue(FString::Printf(TEXT("Biome seed %d valide"), Seed), Layout.Rooms.Num() >= 3);
		TestTrue(FString::Printf(TEXT("Biome seed %d connecte"), Seed), Layout.IsEveryRoomConnected());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegAllBiomesIntegrityTest, "Project.Functional Tests.MEG.AllBiomesIntegrity",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegAllBiomesIntegrityTest::RunTest(const FString& Parameters)
{
	// Verifie l'integrite des 11 biomes officiels du lore des Backrooms
	FLiminalSaveData Save;
	Save.UnlockedBiomes.Add(ELevelBiome::Level0_YellowLobby);
	Save.UnlockedBiomes.Add(ELevelBiome::Level1_HabitableZone);
	Save.UnlockedBiomes.Add(ELevelBiome::Level2_PipeDreams);
	Save.UnlockedBiomes.Add(ELevelBiome::Level3_ElectricalStation);
	Save.UnlockedBiomes.Add(ELevelBiome::Level4_AbandonedOffice);
	Save.UnlockedBiomes.Add(ELevelBiome::Level6_LightsOut);
	Save.UnlockedBiomes.Add(ELevelBiome::Level8_CaveSystem);
	Save.UnlockedBiomes.Add(ELevelBiome::Level9_DarkSuburbs);
	Save.UnlockedBiomes.Add(ELevelBiome::Level10_WheatFields);
	Save.UnlockedBiomes.Add(ELevelBiome::Level37_Poolrooms);
	Save.UnlockedBiomes.Add(ELevelBiome::LevelRun_RunForYourLife);

	TestEqual("11 biomes enregistres", Save.UnlockedBiomes.Num(), 11);
	TestTrue("Contient Level 3 Electrical Station", Save.UnlockedBiomes.Contains(ELevelBiome::Level3_ElectricalStation));
	TestTrue("Contient Level 6 Lights Out", Save.UnlockedBiomes.Contains(ELevelBiome::Level6_LightsOut));
	TestTrue("Contient Level 8 Cave System", Save.UnlockedBiomes.Contains(ELevelBiome::Level8_CaveSystem));
	TestTrue("Contient Level 9 Dark Suburbs", Save.UnlockedBiomes.Contains(ELevelBiome::Level9_DarkSuburbs));
	TestTrue("Contient Level 10 Wheat Fields", Save.UnlockedBiomes.Contains(ELevelBiome::Level10_WheatFields));
	TestTrue("Contient Level Run", Save.UnlockedBiomes.Contains(ELevelBiome::LevelRun_RunForYourLife));

	// Verification de l'integrite des profils acoustiques pour les 11 biomes
	const ULiminalAudioSubsystem* AudioCDO = GetDefault<ULiminalAudioSubsystem>();
	TestNotNull(TEXT("AudioSubsystem CDO valide"), AudioCDO);
	if (AudioCDO)
	{
		for (const ELevelBiome Biome : Save.UnlockedBiomes)
		{
			const FLiminalBiomeAudioProfile Profile = AudioCDO->GetProfileForBiome(Biome);
			TestTrue(FString::Printf(TEXT("Biome %d a un profil audio valide"), static_cast<int32>(Biome)),
				!Profile.AmbientLoopId.IsNone() && Profile.AmbientVolume > 0.0f && Profile.ReverbDecaySeconds > 0.0f);
		}
	}

	// Verification de l'existence sur disque des 18 cartes officielles du projet
	const TArray<FString> All18Maps = {
		TEXT("Lvl_MainMenu.umap"),
		TEXT("Lvl_Hub_BaseAlpha.umap"),
		TEXT("Lvl_00_Lobby.umap"),
		TEXT("Lvl_01_HabitableZone.umap"),
		TEXT("Lvl_02_PipeDreams.umap"),
		TEXT("Lvl_03_ElectricalStation.umap"),
		TEXT("Lvl_04_AbandonedOffice.umap"),
		TEXT("Lvl_06_LightsOut.umap"),
		TEXT("Lvl_08_CaveSystem.umap"),
		TEXT("Lvl_09_DarkSuburbs.umap"),
		TEXT("Lvl_10_WheatFields.umap"),
		TEXT("Lvl_37_Poolrooms.umap"),
		TEXT("Lvl_99_RunForYourLife.umap"),
		TEXT("Lvl_Loop.umap"),
		TEXT("Lvl_ProcGen.umap"),
		TEXT("Lvl_Level0_Massive.umap"),
		TEXT("Lvl_Level37_Poolrooms.umap"),
		TEXT("Lvl_LevelRun_Gauntlet.umap")
	};
	for (const FString& MapFile : All18Maps)
	{
		TestTrue(FString::Printf(TEXT("Map %s existe sur disque"), *MapFile),
			FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Maps") / MapFile));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegVoiceMimicryTest, "Project.Functional Tests.MEG.VoiceMimicry",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegVoiceMimicryTest::RunTest(const FString& Parameters)
{
	const UVoiceMimicryComponent* MimicryCDO = GetDefault<UVoiceMimicryComponent>();
	TestNotNull(TEXT("Composant vocal CDO valide"), MimicryCDO);
	if (MimicryCDO)
	{
		TestTrue(TEXT("Capacite buffer circulaire >= 40000 echantillons"), MimicryCDO->GetMaxSampleCapacity() >= 40000);
	}

	// Test de la structure FVoiceSnippet
	FVoiceSnippet Snippet;
	Snippet.SpeakerId = FName("AllyScavenger_01");
	Snippet.DurationSeconds = 1.5f;
	Snippet.SampleCount = 48000;
	TestEqual(TEXT("SpeakerId memorise"), Snippet.SpeakerId, FName("AllyScavenger_01"));
	TestEqual(TEXT("Duree snippet"), Snippet.DurationSeconds, 1.5f);
	TestTrue(TEXT("Nombre d'echantillons > 0"), Snippet.SampleCount > 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegSanityTierTest, "Project.Functional Tests.MEG.SanityTiers",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegSanityTierTest::RunTest(const FString& Parameters)
{
	TestEqual("100% -> Stable", LiminalSanity::GetTierFromPercent(1.0f), ESanityTier::Stable);
	TestEqual("75% -> Stable", LiminalSanity::GetTierFromPercent(0.75f), ESanityTier::Stable);
	TestEqual("65% -> Uneasy", LiminalSanity::GetTierFromPercent(0.65f), ESanityTier::Uneasy);
	TestEqual("55% -> Uneasy", LiminalSanity::GetTierFromPercent(0.55f), ESanityTier::Uneasy);
	TestEqual("35% -> Paranoid", LiminalSanity::GetTierFromPercent(0.35f), ESanityTier::Paranoid);
	TestEqual("28% -> Paranoid", LiminalSanity::GetTierFromPercent(0.28f), ESanityTier::Paranoid);
	TestEqual("10% -> Psychotic", LiminalSanity::GetTierFromPercent(0.10f), ESanityTier::Psychotic);
	TestEqual("0% -> Psychotic", LiminalSanity::GetTierFromPercent(0.0f), ESanityTier::Psychotic);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegPhase2ToolsTest, "Project.Functional Tests.MEG.Phase2Tools",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegPhase2ToolsTest::RunTest(const FString& Parameters)
{
	// Test LIDAR Scanner CDO
	const ALidarScannerTool* Lidar = GetDefault<ALidarScannerTool>();
	TestNotNull(TEXT("LIDAR CDO valide"), Lidar);
	if (Lidar)
	{
		TestEqual("LIDAR batterie max a 100", Lidar->GetMaxBatteryCharge(), 100.0f);
		TestTrue("LIDAR Portee >= 1000", Lidar->GetScanRange() >= 1000.0f);
		TestTrue("LIDAR angle de cone valide", Lidar->GetScanConeAngle() > 0.0f);
	}

	// Test Signal Analyzer CDO
	const ASignalAnalyzerTool* SignalAnalyzer = GetDefault<ASignalAnalyzerTool>();
	TestNotNull(TEXT("Signal Analyzer CDO valide"), SignalAnalyzer);
	if (SignalAnalyzer)
	{
		TestEqual("Signal Analyzer batterie max a 100", SignalAnalyzer->GetMaxBatteryCharge(), 100.0f);
	}

	// Test Reality Anchor CDO
	const ARealityAnchorTool* RealityAnchor = GetDefault<ARealityAnchorTool>();
	TestNotNull(TEXT("Reality Anchor CDO valide"), RealityAnchor);
	if (RealityAnchor)
	{
		TestEqual("Poids Ancre de Realite = 30 kg", RealityAnchor->GetAnchorWeightKg(), 30.0f);
		TestTrue("Rayon de stabilisation >= 600", RealityAnchor->GetStabilizationRadius() >= 600.0f);
		TestTrue("Duree de champ >= 30s", RealityAnchor->GetRemainingFieldDuration() >= 30.0f);
	}

	// Test Tether Tool CDO
	const ATetherTool* Tether = GetDefault<ATetherTool>();
	TestNotNull(TEXT("Tether CDO valide"), Tether);
	if (Tether)
	{
		TestTrue("Longueur max tether >= 1000", Tether->GetMaxTetherLength() >= 1000.0f);
	}

	// Test Chalk Marker CDO
	const AChalkMarkerTool* Chalk = GetDefault<AChalkMarkerTool>();
	TestNotNull(TEXT("Chalk CDO valide"), Chalk);
	if (Chalk)
	{
		TestEqual("Craie 16 marques initiales", Chalk->GetMaxMarks(), 16);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegHubProgressionTest, "Project.Functional Tests.MEG.HubProgression",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegHubProgressionTest::RunTest(const FString& Parameters)
{
	const ULiminalHubProgressionComponent* HubCDO = GetDefault<ULiminalHubProgressionComponent>();
	TestNotNull(TEXT("Hub Progression CDO valide"), HubCDO);
	if (HubCDO)
	{
		TestEqual("Palier initial : Campement de fortune", HubCDO->GetCurrentTier(), EHubTier::MakeshiftCamp);
		TestFalse("Nom du palier non vide", HubCDO->GetTierDisplayName().IsEmpty());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegMissionGameModeTest, "Project.Functional Tests.MEG.MissionGameMode",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegMissionGameModeTest::RunTest(const FString& Parameters)
{
	const ALiminalGameMode* GM = GetDefault<ALiminalGameMode>();
	TestNotNull(TEXT("LiminalGameMode CDO valide"), GM);
	if (GM)
	{
		TestTrue("Pawn par defaut = ScavengerCharacter", GM->DefaultPawnClass == AScavengerCharacter::StaticClass());
		TestTrue("Duree de mission configuree >= 300s", GM->GetTotalMissionDurationSeconds() >= 300.0f);
		TestTrue("Timer d'effondrement >= 300s", GM->GetRealityCollapseRemainingSeconds() >= 300.0f);
		TestEqual("Etat de depart = InMission", GM->GetMatchState(), EExtractionMatchState::InMission);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegTerminalStoreCatalogTest, "Project.Functional Tests.MEG.TerminalStoreCatalog",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegTerminalStoreCatalogTest::RunTest(const FString& Parameters)
{
	const ALiminalTerminalActor* Terminal = GetDefault<ALiminalTerminalActor>();
	TestNotNull(TEXT("LiminalTerminalActor CDO valide"), Terminal);
	if (Terminal)
	{
		const TArray<FTerminalStoreItem>& Catalog = Terminal->GetStoreCatalog();
		TestTrue("Catalogue contient au moins 8 articles", Catalog.Num() >= 8);

		auto HasItem = [&](FName Id) -> bool
		{
			return Catalog.ContainsByPredicate([&](const FTerminalStoreItem& It) { return It.ItemId == Id; });
		};

		TestTrue("Contient Flash Strobe", HasItem(FName("Tool_FlashStrobe")));
		TestTrue("Contient Micro-ondes Sonique", HasItem(FName("Tool_SonicMicrowave")));
		TestTrue("Contient Scanner LIDAR", HasItem(FName("Tool_LidarScanner")));
		TestTrue("Contient Ancre de Realite", HasItem(FName("Tool_RealityAnchor")));
		TestTrue("Contient Pile 9V", HasItem(FName("Item_Battery")));
		TestTrue("Contient Eau d'Amande", HasItem(FName("Item_AlmondWaterBottle")));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegSmilerSensoryTest, "Project.Functional Tests.MEG.SmilerSensory",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegSmilerSensoryTest::RunTest(const FString& Parameters)
{
	const ALiminalEntity_Smiler* Smiler = GetDefault<ALiminalEntity_Smiler>();
	TestNotNull(TEXT("LiminalEntity_Smiler CDO valide"), Smiler);
	if (Smiler)
	{
		TestEqual("Smiler Vitesse de charge = 850", Smiler->GetChargeSpeed(), 850.0f);
		TestEqual("Smiler Vitesse de traque normale = 260", Smiler->GetNormalStalkSpeed(), 260.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegJerryHypnosisGazeTest, "Project.Functional Tests.MEG.JerryHypnosisGaze",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegJerryHypnosisGazeTest::RunTest(const FString& Parameters)
{
	const ALiminalEntity_Jerry* Jerry = GetDefault<ALiminalEntity_Jerry>();
	TestNotNull(TEXT("LiminalEntity_Jerry CDO valide"), Jerry);
	if (Jerry)
	{
		TestTrue("Portee de regard hypnotique >= 800", Jerry->GetHypnosisGazeRange() >= 800.0f);
		TestTrue("Drain de sanite hypnotique >= 8/s", Jerry->GetHypnosisSanityDrainPerSecond() >= 8.0f);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegScavengerCapabilitiesTest, "Project.Functional Tests.MEG.ScavengerCapabilities",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegScavengerCapabilitiesTest::RunTest(const FString& Parameters)
{
	const AScavengerCharacter* Scavenger = GetDefault<AScavengerCharacter>();
	TestNotNull(TEXT("ScavengerCharacter CDO valide"), Scavenger);
	if (Scavenger)
	{
		TestEqual("Capacite max de transport = 60 kg", Scavenger->GetMaxCarryWeightKg(), 60.0f);
		TestTrue("Endurance max valide", Scavenger->GetStaminaPercent() >= 0.0f);
	}

	return true;
}

#if WITH_EDITOR
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegAssetPipelineGenerationTest, "Project.Functional Tests.MEG.AssetPipelineGeneration",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegAssetPipelineGenerationTest::RunTest(const FString& Parameters)
{
	MEG_FullGameAutomator::BuildFullGame();
	MEG_HoundAIBuilder::BuildHoundAssets();

	TestTrue("DataTable DT_LootItems genere", FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Data/DT_LootItems.uasset")));
	TestTrue("Blackboard BB_Hound genere", FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("AI/BB_Hound.uasset")));
	TestTrue("BehaviorTree BT_Hound genere", FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("AI/BT_Hound.uasset")));
	TestTrue("InputContext IMC_Scavenger genere", FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Input/Scavenger/IMC_Scavenger.uasset")));

	return true;
}
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegMassiveLabyrinthScalingTest, "Project.Functional Tests.MEG.MassiveLabyrinthScaling",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegMassiveLabyrinthScalingTest::RunTest(const FString& Parameters)
{
	// 1. Grand Labyrinthe generation (48x48, 24 rooms)
	const FGeneratedLayout GrandLayout = FLiminalLayoutBuilder::Generate(12345, 48, 48, 24, 3, 8, 0.35f);
	TestTrue(TEXT("Grand Labyrinthe hash non-nul"), GrandLayout.Hash != 0);
	TestTrue(TEXT("Grand Labyrinthe au moins 16 salles"), GrandLayout.Rooms.Num() >= 16);
	TestTrue(TEXT("Grand Labyrinthe cellules praticables abondantes (> 300)"), GrandLayout.Cells.Num() > 300);

	// 2. Mega Expedition generation (64x64, 36 rooms)
	const FGeneratedLayout MegaLayout = FLiminalLayoutBuilder::Generate(67890, 64, 64, 36, 3, 8, 0.35f);
	TestTrue(TEXT("Mega Expedition hash non-nul"), MegaLayout.Hash != 0);
	TestTrue(TEXT("Mega Expedition au moins 20 salles"), MegaLayout.Rooms.Num() >= 20);
	TestTrue(TEXT("Mega Expedition cellules praticables abondantes (> 600)"), MegaLayout.Cells.Num() > 600);

	// 3. Generator CDO defaults
	const ALiminalLevelGenerator* GenCDO = GetDefault<ALiminalLevelGenerator>();
	TestNotNull(TEXT("LiminalLevelGenerator CDO valide"), GenCDO);
	if (GenCDO)
	{
		TestEqual(TEXT("MapScale par defaut = GrandLabyrinthe"), GenCDO->MapScale, EMapScalePreset::GrandLabyrinthe);
		TestEqual(TEXT("Largeur de grille par defaut = 48"), GenCDO->GridWidth, 48);
		TestEqual(TEXT("Hauteur de grille par defaut = 48"), GenCDO->GridHeight, 48);
		TestEqual(TEXT("Nombre de salles par defaut = 24"), GenCDO->RoomCount, 24);
		TestEqual(TEXT("Nombre de loots par defaut = 35"), GenCDO->LootCount, 35);
	}

	// 4. Verification de l'existence des maps dediees sur disque
	TestTrue(TEXT("Lvl_Hub_BaseAlpha.umap existe"), FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Maps/Lvl_Hub_BaseAlpha.umap")));
	TestTrue(TEXT("Lvl_Level0_Massive.umap existe"), FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Maps/Lvl_Level0_Massive.umap")));
	TestTrue(TEXT("Lvl_Level37_Poolrooms.umap existe"), FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Maps/Lvl_Level37_Poolrooms.umap")));
	TestTrue(TEXT("Lvl_LevelRun_Gauntlet.umap existe"), FPaths::FileExists(FPaths::ProjectContentDir() / TEXT("Maps/Lvl_LevelRun_Gauntlet.umap")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegTacticalPolishAndSpectatorTest, "Project.Functional Tests.MEG.TacticalPolishAndSpectator",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegTacticalPolishAndSpectatorTest::RunTest(const FString& Parameters)
{
	// 1. Verification du Spectateur Liminal (CDO)
	const ALiminalSpectatorPawn* SpectatorCDO = GetDefault<ALiminalSpectatorPawn>();
	TestNotNull(TEXT("LiminalSpectatorPawn CDO valide"), SpectatorCDO);
	if (SpectatorCDO)
	{
		TestTrue(TEXT("Spectateur suit un joueur par defaut"), SpectatorCDO->IsFollowingPlayer());
	}

	// 2. Verification du Leurre Audio (CDO)
	const AAudioDecoyTool* DecoyCDO = GetDefault<AAudioDecoyTool>();
	TestNotNull(TEXT("AudioDecoyTool CDO valide"), DecoyCDO);

	// 3. Verification de l'Entite Liminale de base (CDO)
	const ALiminalEntity* EntityCDO = GetDefault<ALiminalEntity>();
	TestNotNull(TEXT("LiminalEntity CDO valide"), EntityCDO);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegFieldManualAndAirlockTest, "Project.Functional Tests.MEG.FieldManualAndAirlock",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegFieldManualAndAirlockTest::RunTest(const FString& Parameters)
{
	// 1. Verification du HUD et du Manuel de Terrain
	const ALiminalScavengerHUD* HudCDO = GetDefault<ALiminalScavengerHUD>();
	TestNotNull(TEXT("LiminalScavengerHUD CDO valide"), HudCDO);
	if (HudCDO)
	{
		TestFalse(TEXT("Manuel ferme par defaut"), HudCDO->IsFieldManualOpen());
	}

	// 2. Verification de l'Airlock Sas
	const ALiminalAirlockActor* AirlockCDO = GetDefault<ALiminalAirlockActor>();
	TestNotNull(TEXT("LiminalAirlockActor CDO valide"), AirlockCDO);
	if (AirlockCDO)
	{
		TestFalse(TEXT("Cycle inactif par defaut"), AirlockCDO->IsCycleActive());
	}

	// 3. Verification de la trace de craie
	const AChalkTraceActor* ChalkTraceCDO = GetDefault<AChalkTraceActor>();
	TestNotNull(TEXT("ChalkTraceActor CDO valide"), ChalkTraceCDO);

	// 4. Verification de l'outil craie
	const AChalkMarkerTool* ChalkToolCDO = GetDefault<AChalkMarkerTool>();
	TestNotNull(TEXT("ChalkMarkerTool CDO valide"), ChalkToolCDO);
	if (ChalkToolCDO)
	{
		TestEqual(TEXT("Nombre initial de marques de craie"), ChalkToolCDO->GetMaxMarks(), 16);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegRepoAndEscapeMechanicsTest, "Project.Functional Tests.MEG.RepoAndEscapeMechanics",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegRepoAndEscapeMechanicsTest::RunTest(const FString& Parameters)
{
	// 1. Verification du Clavier de Securite (Keypad)
	const ALiminalKeypadActor* KeypadCDO = GetDefault<ALiminalKeypadActor>();
	TestNotNull(TEXT("LiminalKeypadActor CDO valide"), KeypadCDO);
	if (KeypadCDO)
	{
		TestFalse(TEXT("Keypad initialement verrouille"), KeypadCDO->IsUnlocked());
		TestEqual(TEXT("Affichage initial ----"), KeypadCDO->GetDisplayString(), FString(TEXT("----")));
	}

	// 2. Verification du Disjoncteur Mural (Breaker)
	const ALiminalBreakerActor* BreakerCDO = GetDefault<ALiminalBreakerActor>();
	TestNotNull(TEXT("LiminalBreakerActor CDO valide"), BreakerCDO);
	if (BreakerCDO)
	{
		TestFalse(TEXT("Disjoncteur initialement desactive"), BreakerCDO->IsPowerRestored());
	}

	// 3. Verification de l'Autoinjecteur d'Adrenaline (Outil)
	const AAdrenalineInjectorTool* AdrenalineCDO = GetDefault<AAdrenalineInjectorTool>();
	TestNotNull(TEXT("AdrenalineInjectorTool CDO valide"), AdrenalineCDO);
	if (AdrenalineCDO)
	{
		TestEqual(TEXT("Doses d'adrenaline initiales"), AdrenalineCDO->GetMaxDoses(), 2);
	}

	// 4. Verification de l'OSD Camescope VHS 1998
	const ALiminalScavengerHUD* HudCDO = GetDefault<ALiminalScavengerHUD>();
	TestNotNull(TEXT("HUD CDO valide"), HudCDO);
	if (HudCDO)
	{
		TestTrue(TEXT("Overlay VHS actif par defaut"), HudCDO->IsVHSOverlayActive());
	}

	// 5. Verification de la gestion Blackout GameMode
	const ALiminalGameMode* GameModeCDO = GetDefault<ALiminalGameMode>();
	TestNotNull(TEXT("GameMode CDO valide"), GameModeCDO);
	if (GameModeCDO)
	{
		TestFalse(TEXT("Blackout inactif par defaut"), GameModeCDO->IsBlackoutActive());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegCoopSurvivalAndDownedTest, "Project.Functional Tests.MEG.CoopSurvivalAndDowned",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegCoopSurvivalAndDownedTest::RunTest(const FString& Parameters)
{
	const AScavengerCharacter* ScavengerCDO = GetDefault<AScavengerCharacter>();
	TestNotNull(TEXT("ScavengerCharacter CDO valide"), ScavengerCDO);
	if (ScavengerCDO)
	{
		TestFalse(TEXT("Non K.O. au depart"), ScavengerCDO->IsDowned());
		TestFalse(TEXT("Non mort au depart"), ScavengerCDO->IsDead());
		TestEqual(TEXT("Compteur d'agonie a 45s par defaut"), ScavengerCDO->GetDownedTimeRemaining(), 45.0f);
		TestEqual(TEXT("Flash de degats a 0 par defaut"), ScavengerCDO->GetDamageFlashIntensity(), 0.0f);
		TestTrue(TEXT("Aucune fausse alerte au depart"), ScavengerCDO->GetActiveFakeAlert().IsEmpty());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegConditionalExtractionAndDeathHUDTest, "Project.Functional Tests.MEG.ConditionalExtractionAndDeathHUD",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegConditionalExtractionAndDeathHUDTest::RunTest(const FString& Parameters)
{
	// 1. Verification de l'Extraction Conditionnelle
	const AExtractionZone* ExtractionCDO = GetDefault<AExtractionZone>();
	TestNotNull(TEXT("ExtractionZone CDO valide"), ExtractionCDO);
	if (ExtractionCDO)
	{
		TestTrue(TEXT("Zone deverrouillee sans contrainte par defaut"), ExtractionCDO->IsExtractionUnlocked());
	}

	// 2. Verification de l'Ecran de Mort
	const ALiminalDeathScreenHUD* DeathHUDCDO = GetDefault<ALiminalDeathScreenHUD>();
	TestNotNull(TEXT("LiminalDeathScreenHUD CDO valide"), DeathHUDCDO);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegModularProcGenDungeonTest, "Project.Functional Tests.MEG.ModularProcGenDungeon",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegModularProcGenDungeonTest::RunTest(const FString& Parameters)
{
	// 1. Verification du determinisme
	const int32 TestSeed = 1337;
	const FLiminalModularDungeonLayout LayoutA = ULiminalModularDungeonGenerator::GenerateModularDungeon(
		TestSeed, 36, 36, 12, 400.0f, nullptr);

	const FLiminalModularDungeonLayout LayoutB = ULiminalModularDungeonGenerator::GenerateModularDungeon(
		TestSeed, 36, 36, 12, 400.0f, nullptr);

	TestEqual(TEXT("Determinisme nombre de tuiles"), LayoutA.PlacedTiles.Num(), LayoutB.PlacedTiles.Num());
	TestEqual(TEXT("Determinisme coordonnee Spawn X"), LayoutA.SpawnTileCoord.X, LayoutB.SpawnTileCoord.X);
	TestEqual(TEXT("Determinisme coordonnee Spawn Y"), LayoutA.SpawnTileCoord.Y, LayoutB.SpawnTileCoord.Y);
	TestEqual(TEXT("Determinisme coordonnee Extraction X"), LayoutA.ExtractionTileCoord.X, LayoutB.ExtractionTileCoord.X);
	TestEqual(TEXT("Determinisme coordonnee Extraction Y"), LayoutA.ExtractionTileCoord.Y, LayoutB.ExtractionTileCoord.Y);

	// 2. Verification de la connectivite globale (BFS)
	const bool bConnected = ULiminalModularDungeonGenerator::IsDungeonFullyConnected(LayoutA);
	TestTrue(TEXT("Le donjon modulaire est 100% connecte"), bConnected);

	// 3. Verification de la presence de points d'ancrage (Prop Sockets)
	TestTrue(TEXT("Sockets d'accroche de props generes"), LayoutA.AllWorldPropSockets.Num() > 0);

	// 4. Verification de la distance Spawn -> Extraction
	const float DistSq = FVector2D::DistSquared(
		FVector2D(LayoutA.SpawnTileCoord.X, LayoutA.SpawnTileCoord.Y),
		FVector2D(LayoutA.ExtractionTileCoord.X, LayoutA.ExtractionTileCoord.Y)
	);
	TestTrue(TEXT("Spawn et Extraction sont distants"), DistSq > 4.0f);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegSteamValvePuzzleTest, "Project.Functional Tests.MEG.SteamValvePuzzle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegSteamValvePuzzleTest::RunTest(const FString& Parameters)
{
	const ALiminalValvePuzzleActor* ValveCDO = GetDefault<ALiminalValvePuzzleActor>();
	TestNotNull(TEXT("LiminalValvePuzzleActor CDO valide"), ValveCDO);
	if (ValveCDO)
	{
		TestFalse(TEXT("Puzzle non resolu au depart"), ValveCDO->IsPuzzleSolved());
		TestEqual(TEXT("Pression initiale a 180 PSI"), ValveCDO->GetCurrentPressure(), 180.0f);
		TestFalse(TEXT("180 PSI est hors de la zone de securite (90-110)"), ValveCDO->IsPressureInSafeZone());
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegFuseBoxPuzzleTest, "Project.Functional Tests.MEG.FuseBoxPuzzle",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegFuseBoxPuzzleTest::RunTest(const FString& Parameters)
{
	const ALiminalFuseBoxActor* FuseCDO = GetDefault<ALiminalFuseBoxActor>();
	TestNotNull(TEXT("LiminalFuseBoxActor CDO valide"), FuseCDO);
	if (FuseCDO)
	{
		TestFalse(TEXT("Courant non retabli au depart"), FuseCDO->IsPowerRestored());
		TestEqual(TEXT("Slot 0 vide au depart"), FuseCDO->GetSlottedFuse(0), 0);
		TestEqual(TEXT("Slot 1 vide au depart"), FuseCDO->GetSlottedFuse(1), 0);
		TestEqual(TEXT("Slot 2 vide au depart"), FuseCDO->GetSlottedFuse(2), 0);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegWalkieTalkieRadioTest, "Project.Functional Tests.MEG.WalkieTalkieRadio",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegWalkieTalkieRadioTest::RunTest(const FString& Parameters)
{
	const AWalkieTalkieTool* WalkieCDO = GetDefault<AWalkieTalkieTool>();
	TestNotNull(TEXT("WalkieTalkieTool CDO valide"), WalkieCDO);
	if (WalkieCDO)
	{
		TestEqual(TEXT("Canal 1 selectionne par defaut"), WalkieCDO->GetChannel(), 1);
		TestFalse(TEXT("Non en emission par defaut"), WalkieCDO->IsTransmitting());
		TestEqual(TEXT("Batterie pleine au depart"), WalkieCDO->GetBatteryCharge(), 100.0f);
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegBodycamTelemetryTest, "Project.Functional Tests.MEG.BodycamTelemetry",
	EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::SmokeFilter)

bool FMegBodycamTelemetryTest::RunTest(const FString& Parameters)
{
	const ULiminalBodycamComponent* BodycamCDO = GetDefault<ULiminalBodycamComponent>();
	TestNotNull(TEXT("LiminalBodycamComponent CDO valide"), BodycamCDO);
	if (BodycamCDO)
	{
		const FBodycamTelemetryData& Telemetry = BodycamCDO->GetTelemetryData();
		TestTrue(TEXT("Horodatage 1998 present"), Telemetry.TimestampString.StartsWith(TEXT("1998-")));
		TestEqual(TEXT("BPM au repos"), Telemetry.HeartRateBPM, 72);
		TestEqual(TEXT("Batterie camera a 100%"), Telemetry.BatteryPercent, 100.0f);
		TestFalse(TEXT("Vision nocturne eteinte par defaut"), Telemetry.bNightVisionActive);
		TestTrue(TEXT("Statut Normal au repos"), Telemetry.StatusWarning == TEXT("NORMAL"));
	}
	return true;
}

#endif



