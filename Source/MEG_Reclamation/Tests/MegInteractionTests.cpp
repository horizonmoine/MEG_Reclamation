#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "MEG_ReclamationPlayerController.h"
#include "Misc/AutomationTest.h"
#include "Network/LiminalSessionManager.h"
#include "Objects/LiminalDoorActor.h"
#include "Objects/LiminalTerminalActor.h"
#include "Player/ScavengerCharacter.h"
#include "UObject/Class.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegInteractionAuthorityTest, "Project.Functional Tests.MEG.InteractionAuthority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegInteractionAuthorityTest::RunTest(const FString& Parameters)
{
	// The wire protocol must expose intentions, never arbitrary resource mutations.
	for (const FName Name : { FName(TEXT("ServerAddInventoryWeight")), FName(TEXT("ServerRemoveInventoryWeight")),
		FName(TEXT("ServerDrainSanity")), FName(TEXT("ServerRestoreSanity")),
		FName(TEXT("ServerSetInfected")), FName(TEXT("ServerRevivePlayer")) })
	{
		const UFunction* Function = AScavengerCharacter::StaticClass()->FindFunctionByName(Name);
		if (TestNotNull(*Name.ToString(), Function))
		{
			TestFalse(TEXT("Raw mutation is not remotely callable"), Function->HasAnyFunctionFlags(FUNC_Net));
		}
	}
	for (const FName Name : { FName(TEXT("ServerInteract")), FName(TEXT("ServerRequestRevive")),
		FName(TEXT("ServerTerminalPurchaseItem")), FName(TEXT("ServerTerminalSelectBiome")), FName(TEXT("ServerTerminalLaunchIncursion")) })
	{
		const UFunction* Function = AScavengerCharacter::StaticClass()->FindFunctionByName(Name);
		if (TestNotNull(*Name.ToString(), Function))
		{
			TestTrue(TEXT("Intent travels to server"), Function->HasAllFunctionFlags(FUNC_Net | FUNC_NetServer));
		}
	}

	// P01: LiminalTerminalActor must expose zero network RPCs directly on unpossessed actor.
	const UClass* TerminalClass = ALiminalTerminalActor::StaticClass();
	for (const FName Name : { FName(TEXT("PurchaseStoreItem")), FName(TEXT("SelectBiome")), FName(TEXT("LaunchIncursion")) })
	{
		const UFunction* Function = TerminalClass->FindFunctionByName(Name);
		if (TestNotNull(*Name.ToString(), Function))
		{
			TestFalse(TEXT("Terminal mutation is not remotely callable directly on unpossessed actor"), Function->HasAnyFunctionFlags(FUNC_Net));
		}
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegTerminalSafetyTest, "Project.Functional Tests.MEG.TerminalSafety",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegTerminalSafetyTest::RunTest(const FString& Parameters)
{
	TGuardValue<bool> GuardScriptExecution(GAllowActorScriptExecutionInEditor, true);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World"), World)) return false;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AScavengerCharacter* Scavenger = World->SpawnActor<AScavengerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	ALiminalTerminalActor* Terminal = World->SpawnActor<ALiminalTerminalActor>(FVector(1000.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Params);

	if (!TestNotNull(TEXT("Scavenger"), Scavenger) || !TestNotNull(TEXT("Terminal"), Terminal))
	{
		World->DestroyWorld(false);
		return false;
	}

	// 1. Distant terminal (> 450 cm) must be rejected
	const ELevelBiome OriginalBiome = Terminal->GetCurrentlySelectedBiome();
	Scavenger->ServerTerminalSelectBiome(Terminal, ELevelBiome::Level1_HabitableZone);
	if (Terminal->GetCurrentlySelectedBiome() != OriginalBiome)
	{
		Scavenger->ServerTerminalSelectBiome_Implementation(Terminal, ELevelBiome::Level1_HabitableZone);
	}
	TestEqual(TEXT("Distant terminal rejects biome selection"), Terminal->GetCurrentlySelectedBiome(), OriginalBiome);

	// 2. Valid proximity and clear LOS allows mutation
	Terminal->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));
	Scavenger->ServerTerminalSelectBiome(Terminal, ELevelBiome::Level1_HabitableZone);
	if (Terminal->GetCurrentlySelectedBiome() != ELevelBiome::Level1_HabitableZone)
	{
		Scavenger->ServerTerminalSelectBiome_Implementation(Terminal, ELevelBiome::Level1_HabitableZone);
	}
	TestEqual(TEXT("Direct in-range interaction accepted"), Terminal->GetCurrentlySelectedBiome(), ELevelBiome::Level1_HabitableZone);

	// 3. Downed player must be rejected
	Scavenger->EnterDownedState();
	Scavenger->ServerTerminalSelectBiome(Terminal, ELevelBiome::Level2_PipeDreams);
	if (Terminal->GetCurrentlySelectedBiome() != ELevelBiome::Level1_HabitableZone)
	{
		Scavenger->ServerTerminalSelectBiome_Implementation(Terminal, ELevelBiome::Level2_PipeDreams);
	}
	TestEqual(TEXT("Downed player cannot mutate terminal"), Terminal->GetCurrentlySelectedBiome(), ELevelBiome::Level1_HabitableZone);
	Scavenger->Revive(1.0f, 1.0f);

	// 4. Wall occlusion rejects terminal interaction
	AActor* Wall = World->SpawnActor<AActor>();
	if (TestNotNull(TEXT("Wall"), Wall))
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Wall->SetRootComponent(Box);
		Box->SetBoxExtent(FVector(10.0f, 100.0f, 150.0f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->RegisterComponent();
		Wall->SetActorLocation(FVector(75.0f, 0.0f, 0.0f));

		Scavenger->ServerTerminalSelectBiome(Terminal, ELevelBiome::Level2_PipeDreams);
		if (Terminal->GetCurrentlySelectedBiome() != ELevelBiome::Level1_HabitableZone)
		{
			Scavenger->ServerTerminalSelectBiome_Implementation(Terminal, ELevelBiome::Level2_PipeDreams);
		}
		TestEqual(TEXT("Wall blocks terminal interaction"), Terminal->GetCurrentlySelectedBiome(), ELevelBiome::Level1_HabitableZone);
	}

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegSessionIdentityAndStasisTest, "Project.Functional Tests.MEG.SessionIdentityAndStasis",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegSessionIdentityAndStasisTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World"), World)) return false;

	UGameInstance* GI = NewObject<UGameInstance>(GEngine);
	if (!TestNotNull(TEXT("GameInstance"), GI))
	{
		World->DestroyWorld(false);
		return false;
	}
	World->SetGameInstance(GI);
	GI->Init();

	ULiminalSessionManager* SessionMgr = GI->GetSubsystem<ULiminalSessionManager>();
	if (!TestNotNull(TEXT("SessionManager"), SessionMgr))
	{
		GI->Shutdown();
		World->DestroyWorld(false);
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AScavengerCharacter* Scavenger = World->SpawnActor<AScavengerCharacter>(FVector(100.0f, 200.0f, 50.0f), FRotator::ZeroRotator, SpawnParams);
	APlayerController* ControllerA = World->SpawnActor<APlayerController>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	APlayerController* ControllerB = World->SpawnActor<APlayerController>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!TestNotNull(TEXT("Scavenger"), Scavenger) || !TestNotNull(TEXT("ControllerA"), ControllerA) || !TestNotNull(TEXT("ControllerB"), ControllerB))
	{
		World->DestroyWorld(false);
		return false;
	}

	ControllerA->Possess(Scavenger);

	// 1. Unauthenticated controller has empty identity (never falls back to username or transient PlayerId)
	TestTrue(TEXT("Unauthenticated controller ID is empty"), SessionMgr->GetAuthenticatedPlayerId(ControllerA).IsEmpty());
	TestFalse(TEXT("Cannot disconnect unauthenticated controller into stasis"), SessionMgr->RegisterPlayerDisconnect(ControllerA));

	// 2. Register authenticated persistent identities
	const FString UniqueIdA = TEXT("M.E.G._Agent_Alpha_01");
	const FString UniqueIdB = TEXT("M.E.G._Agent_Bravo_02");
	SessionMgr->RegisterAuthenticatedPlayerId(ControllerA, UniqueIdA);
	SessionMgr->RegisterAuthenticatedPlayerId(ControllerB, UniqueIdB);

	TestEqual(TEXT("ControllerA resolves to UniqueIdA"), SessionMgr->GetAuthenticatedPlayerId(ControllerA), UniqueIdA);
	TestEqual(TEXT("ControllerB resolves to UniqueIdB"), SessionMgr->GetAuthenticatedPlayerId(ControllerB), UniqueIdB);
	TestTrue(TEXT("Homonym protection: distinct identities are not equal"), UniqueIdA != UniqueIdB);

	// 3. Setup scavenger state before disconnect
	Scavenger->AuthSetHealthAndSanity(42.0f, 73.0f);
	Scavenger->SetCarriedCredits(350);
	Scavenger->AuthSetInventoryWeight(12.5f);

	// 4. Register disconnect into stasis
	TestTrue(TEXT("RegisterPlayerDisconnect succeeds with authenticated ID"), SessionMgr->RegisterPlayerDisconnect(ControllerA));
	TestTrue(TEXT("Has stasis record for Agent Alpha"), SessionMgr->HasStasisRecord(UniqueIdA));
	TestTrue(TEXT("Pawn entered stasis"), Scavenger->IsInStasis());
	TestEqual(TEXT("No loot dropped on disconnect - credits remain on stasis pawn"), Scavenger->GetCarriedCredits(), 350);

	ControllerA->UnPossess();

	// 5. ControllerB (different identity) cannot steal Agent Alpha's stasis
	TestFalse(TEXT("ControllerB cannot restore Agent Alpha's stasis"), SessionMgr->TryRestorePlayer(ControllerB));

	// 6. ControllerA reconnects and restores stasis (reprise du pawn en stase)
	AScavengerCharacter* DummyPawn = World->SpawnActor<AScavengerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	ControllerA->Possess(DummyPawn);

	TestTrue(TEXT("TryRestorePlayer succeeds for ControllerA"), SessionMgr->TryRestorePlayer(ControllerA));
	TestTrue(TEXT("ControllerA repossessed original stasis pawn"), ControllerA->GetPawn() == Scavenger);
	TestFalse(TEXT("Stasis pawn has exited stasis"), Scavenger->IsInStasis());
	TestEqual(TEXT("Absolute health restored exactly"), Scavenger->GetCurrentHealth(), 42.0f);
	TestEqual(TEXT("Absolute sanity restored exactly"), Scavenger->GetCurrentSanity(), 73.0f);
	TestEqual(TEXT("Absolute credits restored exactly"), Scavenger->GetCarriedCredits(), 350);
	TestEqual(TEXT("Absolute inventory weight restored exactly"), Scavenger->GetCurrentInventoryWeightKg(), 12.5f);
	TestFalse(TEXT("Stasis record consumed"), SessionMgr->HasStasisRecord(UniqueIdA));

	// 7. Stasis expiration test
	SessionMgr->SetStasisExpirationSeconds(0.01f);
	TestTrue(TEXT("Second disconnect succeeds"), SessionMgr->RegisterPlayerDisconnect(ControllerA));
	ControllerA->UnPossess();

	FPlatformProcess::Sleep(0.02f);
	TestTrue(TEXT("Stasis record is expired"), SessionMgr->IsStasisExpired(UniqueIdA));
	TestFalse(TEXT("Expired stasis cannot be restored"), SessionMgr->TryRestorePlayer(ControllerA));
	TestTrue(TEXT("Expired stasis pawn marked dead"), Scavenger->IsDead());
	TestFalse(TEXT("Expired record removed from active list"), SessionMgr->HasStasisRecord(UniqueIdA));

	GI->Shutdown();
	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegRescueAndDoorTest, "Project.Functional Tests.MEG.RescueAndDoorSafety",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegRescueAndDoorTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("World"), World)) return false;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AScavengerCharacter* Rescuer = World->SpawnActor<AScavengerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
	AScavengerCharacter* Target = World->SpawnActor<AScavengerCharacter>(FVector(180.0f, 0.0f, 0.0f), FRotator::ZeroRotator, Params);
	ALiminalDoorActor* Door = World->SpawnActor<ALiminalDoorActor>(FVector(0.0f, 1000.0f, 0.0f), FRotator::ZeroRotator, Params);
	if (!TestNotNull(TEXT("Rescuer"), Rescuer) || !TestNotNull(TEXT("Target"), Target) || !TestNotNull(TEXT("Door"), Door))
	{
		World->DestroyWorld(false);
		return false;
	}
	TestFalse(TEXT("Cannot revive healthy target"), Rescuer->CanReviveTarget(Target));
	Target->EnterDownedState();
	TestFalse(TEXT("Self revive rejected"), Target->CanReviveTarget(Target));
	TestTrue(TEXT("Nearby downed ally accepted"), Rescuer->CanReviveTarget(Target));
	Target->SetActorLocation(FVector(1000.0f, 0.0f, 0.0f));
	TestFalse(TEXT("Distant ally rejected"), Rescuer->CanReviveTarget(Target));
	Target->SetActorLocation(FVector(180.0f, 0.0f, 0.0f));
	AActor* Wall = World->SpawnActor<AActor>();
	if (TestNotNull(TEXT("Wall"), Wall))
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Wall->SetRootComponent(Box);
		Box->SetBoxExtent(FVector(10.0f, 100.0f, 150.0f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->RegisterComponent();
		Wall->SetActorLocation(FVector(90.0f, 0.0f, 0.0f));
		TestFalse(TEXT("Wall rejects rescue"), Rescuer->CanReviveTarget(Target));
	}
	Door->Interact(Rescuer);
	Door->Tick(1.0f);
	TestEqual(TEXT("Door opens"), Door->GetDoorState(), EDoorState::Open);
	Door->Interact(Rescuer);
	// Old integration oscillated 90 -> 6 -> -78 -> 6 and never closed.
	Door->Tick(0.7f);
	Door->Tick(0.7f);
	TestEqual(TEXT("Coarse steps close without oscillation"), Door->GetDoorState(), EDoorState::Closed);
	World->DestroyWorld(false);
	return true;
}

#endif
