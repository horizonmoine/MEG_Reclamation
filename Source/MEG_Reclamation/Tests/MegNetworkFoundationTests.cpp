#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

#include "Player/LiminalPlayerState.h"
#include "GameModes/LiminalGameState.h"
#include "GameModes/LiminalZoneRulesSubsystem.h"
#include "Objects/LiminalSafeZoneVolume.h"
#include "AI/LiminalStimulusSubsystem.h"
#include "Inventory/LiminalInventoryTypes.h"

namespace MegNetTestUtils
{
	/** Monde de jeu autonome (NM_Standalone => HasAuthority() vrai). */
	struct FScopedTestWorld
	{
		UWorld* World = nullptr;

		FScopedTestWorld()
		{
			if (!GEngine)
			{
				return;
			}
			World = UWorld::CreateWorld(EWorldType::Game, false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
		}

		~FScopedTestWorld()
		{
			if (World && GEngine)
			{
				GEngine->DestroyWorldContext(World);
				World->DestroyWorld(false);
			}
		}
	};
}

// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegPlayerStateAuthorityTest, "Project.Functional Tests.MEG.Network.PlayerState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegPlayerStateAuthorityTest::RunTest(const FString& Parameters)
{
	MegNetTestUtils::FScopedTestWorld Scoped;
	if (!TestNotNull("Monde de test", Scoped.World))
	{
		return false;
	}

	ALiminalPlayerState* PS = Scoped.World->SpawnActor<ALiminalPlayerState>();
	if (!TestNotNull("PlayerState spawne", PS))
	{
		return false;
	}

	TestTrue("Autorite en standalone", PS->HasAuthority());
	TestEqual("Sanite initiale", PS->GetSanity(), 100.0f);
	TestTrue("Statut initial Alive", PS->GetStatus() == EScavengerStatus::Alive);

	PS->AuthApplySanityDelta(-150.0f);
	TestEqual("Sanite clampee a 0", PS->GetSanity(), 0.0f);
	TestTrue("Sanite nulle => Downed", PS->GetStatus() == EScavengerStatus::Downed);

	TestFalse("Downed -> Spectating refusee", PS->AuthSetStatus(EScavengerStatus::Spectating));
	TestTrue("Statut inchange apres refus", PS->GetStatus() == EScavengerStatus::Downed);

	PS->AuthRevive(0.4f, 0.5f);
	TestEqual("Sante 40 apres revive", PS->GetHealth(), 40.0f);
	TestEqual("Sanite 50 apres revive", PS->GetSanity(), 50.0f);
	TestTrue("Alive apres revive", PS->GetStatus() == EScavengerStatus::Alive);

	PS->AuthRevive();
	TestEqual("Revive ignore si deja Alive", PS->GetSanity(), 50.0f);

	PS->AuthAddCarriedCredits(120);
	PS->AuthAddCarriedCredits(-200);
	TestEqual("Credits jamais negatifs", PS->GetCarriedCredits(), 0);

	PS->AuthApplyHealthDelta(-500.0f);
	TestTrue("Sante nulle => Downed", PS->GetStatus() == EScavengerStatus::Downed);
	TestTrue("Downed -> Dead", PS->AuthSetStatus(EScavengerStatus::Dead));
	TestTrue("Dead -> Spectating", PS->AuthSetStatus(EScavengerStatus::Spectating));

	PS->AuthResetForNewRun();
	TestTrue("Reset => Alive", PS->GetStatus() == EScavengerStatus::Alive);
	TestEqual("Reset => sante max", PS->GetHealthPercent(), 1.0f);

	TestFalse("Table statique : meme statut refuse", ALiminalPlayerState::IsStatusTransitionValid(EScavengerStatus::Alive, EScavengerStatus::Alive));
	TestTrue("Table statique : Alive -> Extracted", ALiminalPlayerState::IsStatusTransitionValid(EScavengerStatus::Alive, EScavengerStatus::Extracted));

	return true;
}

// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegMissionPhaseTimerTest, "Project.Functional Tests.MEG.Network.MissionPhaseTimer",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegMissionPhaseTimerTest::RunTest(const FString& Parameters)
{
	MegNetTestUtils::FScopedTestWorld Scoped;
	if (!TestNotNull("Monde de test", Scoped.World))
	{
		return false;
	}

	ALiminalGameState* GS = Scoped.World->SpawnActor<ALiminalGameState>();
	if (!TestNotNull("GameState spawne", GS))
	{
		return false;
	}

	TestTrue("Phase initiale Hub", GS->GetMissionPhase() == EMissionPhase::Hub);
	TestFalse("Hub -> Extracted refusee", GS->AuthSetMissionPhase(EMissionPhase::Extracted));
	TestTrue("Hub -> Airlock", GS->AuthSetMissionPhase(EMissionPhase::Airlock));
	TestTrue("Airlock -> Incursion", GS->AuthSetMissionPhase(EMissionPhase::Incursion));

	TestFalse("Timer arrete au depart", GS->IsCollapseTimerRunning());
	GS->AuthStartCollapseTimer(480.0f);
	TestTrue("Timer demarre", GS->IsCollapseTimerRunning());
	TestEqual("Temps restant initial 480 s", GS->GetCollapseTimeRemaining(), 480.0f, 0.1f);
	TestEqual("Progression initiale 0", GS->GetCollapseProgress(), 0.0f, 0.001f);
	TestTrue("Stabilite derivee : Normal", GS->GetStabilityPhase() == EStabilityPhase::Normal);

	TestTrue("Incursion -> Collapsing", GS->AuthSetMissionPhase(EMissionPhase::Collapsing));
	TestTrue("Collapsing -> Failed", GS->AuthSetMissionPhase(EMissionPhase::Failed));
	TestFalse("Failed -> Incursion refusee", GS->AuthSetMissionPhase(EMissionPhase::Incursion));
	TestTrue("Failed -> Hub (retour base)", GS->AuthSetMissionPhase(EMissionPhase::Hub));

	GS->AuthStopCollapseTimer();
	TestFalse("Timer arrete", GS->IsCollapseTimerRunning());

	GS->AuthAddTeamBankCredits(300);
	GS->AuthAddTeamBankCredits(-50);
	TestEqual("Banque equipe 250", GS->GetTeamBankCredits(), 250);
	GS->AuthAddTeamBankCredits(-1000);
	TestEqual("Banque jamais negative", GS->GetTeamBankCredits(), 0);

	GS->AuthSetMissionSeed(1337);
	TestEqual("Seed repliquee", GS->GetMissionSeed(), 1337);

	return true;
}

// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegStimulusSubsystemTest, "Project.Functional Tests.MEG.Network.StimulusSubsystem",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegStimulusSubsystemTest::RunTest(const FString& Parameters)
{
	MegNetTestUtils::FScopedTestWorld Scoped;
	if (!TestNotNull("Monde de test", Scoped.World))
	{
		return false;
	}

	ULiminalStimulusSubsystem* Sub = Scoped.World->GetSubsystem<ULiminalStimulusSubsystem>();
	if (!TestNotNull("Subsystem stimuli", Sub))
	{
		return false;
	}

	Sub->ClearAll();

	TestFalse("Bruit sous seuil RMS ignore", Sub->ReportNoise(FVector::ZeroVector, 0.01f, nullptr));
	TestTrue("Bruit fort enregistre", Sub->ReportNoise(FVector::ZeroVector, 1.0f, nullptr));

	FLiminalStimulus Out;
	TestTrue("Hound a 30 m entend", Sub->HasStimulusInRange(ELiminalStimulusType::Noise, FVector(3000.0f, 0.0f, 0.0f), 5.0f, Out));
	TestEqual("Intensite conservee", Out.Strength, 1.0f);
	TestFalse("Hound a 90 m n'entend pas", Sub->HasStimulusInRange(ELiminalStimulusType::Noise, FVector(9000.0f, 0.0f, 0.0f), 5.0f, Out));
	TestFalse("Aucune lumiere signalee", Sub->HasStimulusInRange(ELiminalStimulusType::Light, FVector::ZeroVector, 5.0f, Out));

	TestTrue("Lumiere enregistree", Sub->ReportLight(FVector(100.0f, 0.0f, 0.0f), 0.5f, nullptr));
	TestTrue("Smiler a 10 m voit", Sub->HasStimulusInRange(ELiminalStimulusType::Light, FVector(1000.0f, 0.0f, 0.0f), 5.0f, Out));
	TestFalse("Smiler a 30 m ne voit pas (rayon 20 m)", Sub->HasStimulusInRange(ELiminalStimulusType::Light, FVector(3000.0f, 0.0f, 0.0f), 5.0f, Out));

	TArray<FLiminalStimulus> Recent;
	TestEqual("Un seul bruit recent", Sub->GetRecentStimuli(ELiminalStimulusType::Noise, 5.0f, Recent), 1);

	Sub->ClearAll();
	TestEqual("Purge totale", Sub->GetRecentStimuli(ELiminalStimulusType::Noise, 5.0f, Recent), 0);

	return true;
}

// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegInventoryListTest, "Project.Functional Tests.MEG.Network.InventoryList",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegInventoryListTest::RunTest(const FString& Parameters)
{
	FLiminalInventoryList List;
	List.GridSize = FIntPoint(6, 4);

	FLiminalInventoryEntry A;
	A.ItemId = TEXT("Artefact_A");
	A.GridPos = FIntPoint(0, 0);
	A.Size = FIntPoint(2, 2);
	A.WeightKg = 3.5f;
	A.Value = 120;
	const int32 IdA = List.AddEntry(A);
	TestTrue("A place", IdA != INDEX_NONE);

	FLiminalInventoryEntry B;
	B.ItemId = TEXT("Artefact_B");
	B.GridPos = FIntPoint(1, 1);
	B.Size = FIntPoint(1, 1);
	TestTrue("B chevauche A : refuse", List.AddEntry(B) == INDEX_NONE);

	B.GridPos = FIntPoint(2, 0);
	B.Size = FIntPoint(2, 1);
	B.WeightKg = 1.0f;
	B.Value = 40;
	const int32 IdB = List.AddEntry(B);
	TestTrue("B place a cote", IdB != INDEX_NONE);
	TestTrue("Ids distincts", IdA != IdB);

	FLiminalInventoryEntry C;
	C.ItemId = TEXT("Artefact_C");
	C.GridPos = FIntPoint(5, 3);
	C.Size = FIntPoint(2, 1);
	TestTrue("C hors grille : refuse", List.AddEntry(C) == INDEX_NONE);

	C.GridPos = FIntPoint(4, 3);
	C.bRotated = true; // 2x1 tourne => 1x2, depasse en Y
	TestTrue("C tourne hors grille : refuse", List.AddEntry(C) == INDEX_NONE);

	C.bRotated = false;
	C.WeightKg = 0.5f;
	C.Value = 10;
	const int32 IdC = List.AddEntry(C);
	TestTrue("C place", IdC != INDEX_NONE);

	TestEqual("Poids total", List.GetTotalWeightKg(), 5.0f, 0.001f);
	TestEqual("Valeur totale", List.GetTotalValue(), 170);

	TestTrue("Retrait de B", List.RemoveEntry(IdB));
	TestFalse("Retrait de B deja retire", List.RemoveEntry(IdB));
	TestNotNull("A toujours present", List.FindEntry(IdA));
	TestNull("B absent", List.FindEntry(IdB));
	TestNotNull("C conserve son id", List.FindEntry(IdC));

	TestTrue("Deplacement de A vers zone libre", List.MoveEntry(IdA, FIntPoint(2, 0), false));
	TestFalse("Deplacement de A sur C refuse", List.MoveEntry(IdA, FIntPoint(3, 2), false));

	FIntPoint Free;
	TestTrue("Slot libre 1x1 trouve", List.FindFreeSlot(FIntPoint(1, 1), Free));
	TestFalse("Slot 7x1 impossible", List.FindFreeSlot(FIntPoint(7, 1), Free));

	return true;
}

// ---------------------------------------------------------------------------

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegZoneRulesTest, "Project.Functional Tests.MEG.Network.ZoneRules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegZoneRulesTest::RunTest(const FString& Parameters)
{
	MegNetTestUtils::FScopedTestWorld Scoped;
	if (!TestNotNull("Monde de test", Scoped.World))
	{
		return false;
	}

	ULiminalZoneRulesSubsystem* Rules = Scoped.World->GetSubsystem<ULiminalZoneRulesSubsystem>();
	if (!TestNotNull("Subsystem regles de zone", Rules))
	{
		return false;
	}

	// Sans ALiminalGameState (Hub, menu, lobby) : jamais hostile.
	TestFalse("Sans GameState liminal : non hostile", Rules->IsMissionHostile());

	ALiminalGameState* GS = Scoped.World->SpawnActor<ALiminalGameState>();
	if (!TestNotNull("GameState spawne", GS))
	{
		return false;
	}
	Scoped.World->SetGameState(GS);

	TestFalse("Phase Hub : non hostile", Rules->IsMissionHostile());
	GS->AuthSetMissionPhase(EMissionPhase::Airlock);
	TestFalse("Phase Airlock : non hostile", Rules->IsMissionHostile());
	GS->AuthSetMissionPhase(EMissionPhase::Incursion);
	TestTrue("Phase Incursion : hostile", Rules->IsMissionHostile());

	ALiminalSafeZoneVolume* Zone = Scoped.World->SpawnActor<ALiminalSafeZoneVolume>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (!TestNotNull("Zone sure spawnee", Zone))
	{
		return false;
	}
	TestEqual("Zone enregistree", Rules->GetSafeZoneCount(), 1);

	TestTrue("Dans la zone (5 m)", Rules->IsLocationInSafeZone(FVector(500.0f, 0.0f, 0.0f)));
	TestFalse("Hors zone (50 m)", Rules->IsLocationInSafeZone(FVector(5000.0f, 0.0f, 0.0f)));
	TestFalse("Spawn d'entite bloque dans la zone", Rules->IsEntitySpawnAllowedAt(FVector::ZeroVector));
	TestTrue("Spawn d'entite autorise hors zone", Rules->IsEntitySpawnAllowedAt(FVector(5000.0f, 0.0f, 0.0f)));

	GS->AuthSetMissionPhase(EMissionPhase::Collapsing);
	TestTrue("Phase Collapsing : hostile", Rules->IsMissionHostile());
	GS->AuthSetMissionPhase(EMissionPhase::Failed);
	TestFalse("Phase Failed : non hostile", Rules->IsMissionHostile());
	TestFalse("Aucun spawn hors incursion", Rules->IsEntitySpawnAllowedAt(FVector(5000.0f, 0.0f, 0.0f)));

	Zone->Destroy();
	Rules->UnregisterSafeZone(Zone);
	TestEqual("Zone desenregistree", Rules->GetSafeZoneCount(), 0);

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
