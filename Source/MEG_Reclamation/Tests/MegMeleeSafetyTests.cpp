#if WITH_DEV_AUTOMATION_TESTS

#include "AI/LiminalEntity.h"
#include "AI/LiminalEntity_Smiler.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/AutomationTest.h"
#include "Player/ScavengerCharacter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegMeleeSafetyTest, "Project.Functional Tests.MEG.MeleeSafety.PawnDamage",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegMeleeSafetyTest::RunTest(const FString& Parameters)
{
	// A transient physics world; no map loading, BeginPlay, asset import or package saves.
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Transient world"), World))
	{
		return false;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALiminalEntity* Entity = World->SpawnActor<ALiminalEntity>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AScavengerCharacter* Target = World->SpawnActor<AScavengerCharacter>(FVector(80.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	if (!TestNotNull(TEXT("Attacker"), Entity) || !TestNotNull(TEXT("Target"), Target))
	{
		World->DestroyWorld(false);
		return false;
	}

	const float Before = Target->GetHealthPercent();
	TestTrue(TEXT("Fallback swing accepted"), Entity->PerformMeleeAttack(Target));
	TestTrue(TEXT("One 35 damage impact, not two"), FMath::IsNearlyEqual(Target->GetHealthPercent(), Before - 0.35f));
	const float After = Target->GetHealthPercent();
	TestEqual(TEXT("Repeated notify ignored"), Entity->OnAttackNotify(), 0);
	TestEqual(TEXT("Repeated notify does not damage"), Target->GetHealthPercent(), After);
	TestFalse(TEXT("Cooldown prevents a second swing"), Entity->PerformMeleeAttack(Target));

	Entity->Tick(2.0f);
	Entity->ApplyCalm(1.0f);
	TestFalse(TEXT("Calmed attacker cannot swing"), Entity->PerformMeleeAttack(Target));
	Entity->Tick(2.0f);

	AActor* Wall = World->SpawnActor<AActor>();
	if (TestNotNull(TEXT("Wall"), Wall))
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Wall->SetRootComponent(Box);
		Box->SetBoxExtent(FVector(10.0f, 150.0f, 150.0f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->RegisterComponent();
		Wall->SetActorLocation(FVector(40.0f, 0.0f, 0.0f));
		TestFalse(TEXT("Opaque wall rejects attack"), Entity->PerformMeleeAttack(Target));
		TestEqual(TEXT("Wall protects target"), Target->GetHealthPercent(), After);
	}

	World->DestroyWorld(false);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FMegSmilerVisibilityTest, "Project.Functional Tests.MEG.MeleeSafety.SmilerVisibility",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FMegSmilerVisibilityTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	if (!TestNotNull(TEXT("Transient world"), World))
	{
		return false;
	}
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ALiminalEntity_Smiler* Smiler = World->SpawnActor<ALiminalEntity_Smiler>(FVector(300.0f, 0.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
	AScavengerCharacter* Player = World->SpawnActor<AScavengerCharacter>(FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	AActor* Wall = World->SpawnActor<AActor>();
	if (TestNotNull(TEXT("Smiler"), Smiler) && TestNotNull(TEXT("Player"), Player) && TestNotNull(TEXT("Wall"), Wall))
	{
		UBoxComponent* Box = NewObject<UBoxComponent>(Wall);
		Wall->SetRootComponent(Box);
		Box->SetBoxExtent(FVector(10.0f, 200.0f, 200.0f));
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Box->SetCollisionResponseToAllChannels(ECR_Block);
		Box->RegisterComponent();
		Wall->SetActorLocation(FVector(150.0f, 0.0f, 0.0f));
		const float SanityBefore = Player->GetSanityPercent();
		Smiler->Tick(0.1f);
		TestFalse(TEXT("Wall blocks gaze reaction"), Smiler->IsCharging());
		TestEqual(TEXT("Wall blocks gaze sanity drain"), Player->GetSanityPercent(), SanityBefore);
		Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Smiler->Tick(0.1f);
		TestTrue(TEXT("Visible direct gaze provokes charge"), Smiler->IsCharging());
		TestFalse(TEXT("Direct gaze is not a paralysis exploit"), Smiler->IsParalyzedByStare());
	}
	World->DestroyWorld(false);
	return true;
}

#endif

