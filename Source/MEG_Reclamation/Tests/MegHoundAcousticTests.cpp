#if WITH_DEV_AUTOMATION_TESTS

#include "AI/LiminalEntity_Hound.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Misc/AutomationTest.h"
#include "Player/ScavengerCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegHoundAcousticTest, "Project.Functional Tests.MEG.HoundAcoustics",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegHoundAcousticTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Transient world"), World))
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALiminalEntity_Hound* Hound = World->SpawnActor<ALiminalEntity_Hound>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AScavengerCharacter* Player1 = World->SpawnActor<AScavengerCharacter>(FVector(300.0f, 0.0f, 0.0f), FRotator(0.0f, 180.0f, 0.0f), SpawnParams);
	AScavengerCharacter* Player2 = World->SpawnActor<AScavengerCharacter>(FVector(0.0f, 400.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);

	if (!TestNotNull(TEXT("Hound"), Hound) || !TestNotNull(TEXT("Player1"), Player1) || !TestNotNull(TEXT("Player2"), Player2))
	{
		World->DestroyWorld(false);
		return false;
	}

	// 1. Distant micro-noise (> MaxHearingRange = 3000cm) does not provoke charge
	Hound->RegisterAcousticStimulus(FVector(4500.0f, 0.0f, 0.0f), 0.5f, nullptr, FName(TEXT("DistantNoise")));
	Hound->Tick(0.1f);
	TestTrue(TEXT("Distant micro-sound does not trigger charge"),
		Hound->GetHoundState() == EHoundAcousticState::Attente);

	// 2. Closed door / wall attenuates sound
	Hound->RegisterAcousticStimulus(FVector(600.0f, 0.0f, 0.0f), 0.7f, nullptr, FName(TEXT("ClearNoise")));
	const float ClearLoudness = Hound->GetLastPerceivedLoudness();

	AActor* Wall = World->SpawnActor<AActor>();
	if (Wall)
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Wall->SetRootComponent(Box);
		Box->SetBoxExtent(FVector(10.0f, 200.0f, 200.0f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->RegisterComponent();
		Wall->SetActorLocation(FVector(300.0f, 0.0f, 0.0f));

		Hound->RegisterAcousticStimulus(FVector(600.0f, 0.0f, 0.0f), 0.7f, nullptr, FName(TEXT("OccludedNoise")));
		const float MuffledLoudness = Hound->GetLastPerceivedLoudness();

		TestTrue(TEXT("Closed wall attenuates perceived intensity"), MuffledLoudness < ClearLoudness);
		Wall->Destroy();
	}

	// 3. Audio Decoy attracts Hound to its exact position
	AActor* Decoy = World->SpawnActor<AActor>();
	if (Decoy)
	{
		USceneComponent* RootComp = NewObject<USceneComponent>(Decoy);
		Decoy->SetRootComponent(RootComp);
		RootComp->RegisterComponent();
		const FVector DecoyLocation(750.0f, 250.0f, 0.0f);
		Decoy->SetActorLocation(DecoyLocation);
		Decoy->Tags.Add(FName(TEXT("AudioDecoy")));
		Hound->Tick(0.1f);

		TestEqual(TEXT("Audio decoy attracts Hound to its exact position"),
			Hound->GetLastKnownSoundLocation(), DecoyLocation);
		TestTrue(TEXT("Loud decoy triggers pursuit/charge state"),
			Hound->GetHoundState() == EHoundAcousticState::Poursuite);

		Decoy->Tags.Empty();
		Decoy->Destroy();
	}

	// 4. Eye contact rule (Regle du regard): direct stare within 4m intimidates Hound and pauses charge
	// Position Player 1 facing directly at Hound
	Player1->SetActorLocation(FVector(250.0f, 0.0f, 0.0f));
	Player1->SetActorRotation(FRotator(0.0f, 180.0f, 0.0f)); // looking straight at origin (Hound)
	if (AController* C = Player1->GetController())
	{
		C->SetControlRotation(FRotator(0.0f, 180.0f, 0.0f));
	}
	Hound->RegisterAcousticStimulus(Player1->GetActorLocation(), 1.6f, Player1, FName(TEXT("PlayerSpotted")));
	Hound->Tick(0.1f);

	TestTrue(TEXT("Direct eye contact holds Hound at bay"), Hound->IsTargetIntimidating());

	// 5. Target stability / hysteresis: two players do not oscillate target every frame
	Hound->RegisterAcousticStimulus(Player1->GetActorLocation(), 1.5f, Player1, FName(TEXT("P1Sound")));
	Hound->Tick(0.1f);
	TestEqual(TEXT("Hound locked on Player 1"), Hound->GetCurrentAcousticTarget(), (AActor*)Player1);

	// Player 2 emits a comparable sound, but below the 1.25x hysteresis threshold
	Hound->RegisterAcousticStimulus(Player2->GetActorLocation(), 1.52f, Player2, FName(TEXT("P2Sound")));
	Hound->Tick(0.1f);
	TestEqual(TEXT("Target stability: Hound does not flip target to Player 2 without significant margin"),
		Hound->GetCurrentAcousticTarget(), (AActor*)Player1);

	World->DestroyWorld(false);
	return true;
}

#endif
