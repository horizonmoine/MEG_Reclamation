#if WITH_DEV_AUTOMATION_TESTS

#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "Objects/LiminalDoorActor.h"
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
	for (const FName Name : { FName(TEXT("ServerInteract")), FName(TEXT("ServerRequestRevive")) })
	{
		const UFunction* Function = AScavengerCharacter::StaticClass()->FindFunctionByName(Name);
		if (TestNotNull(*Name.ToString(), Function))
		{
			TestTrue(TEXT("Intent travels to server"), Function->HasAllFunctionFlags(FUNC_Net | FUNC_NetServer));
		}
	}
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
