#include "Tools/SonicMicrowaveTool.h"

#include "AI/LiminalEntity.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Perception/AISense_Hearing.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASonicMicrowaveTool::ASonicMicrowaveTool()
{
	ActiveBatteryDrainPerSecond = 80.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SonicFinder(
		TEXT("/Game/Meshes/Tools/SM_Sonic_Microwave.SM_Sonic_Microwave"));
	if (SonicFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(SonicFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f));
	}
}

bool ASonicMicrowaveTool::CanActivate() const
{
	return !bIsOverheated && Super::CanActivate();
}

bool ASonicMicrowaveTool::Activate()
{
	if (!Super::Activate())
	{
		return false;
	}

	FirePulse();

	GetWorldTimerManager().SetTimer(ActiveWindowTimerHandle, this, &ASonicMicrowaveTool::EndActiveWindow, ActiveWindowSeconds, false);

	bIsOverheated = true;
	GetWorldTimerManager().SetTimer(OverheatTimerHandle, this, &ASonicMicrowaveTool::ClearOverheat, OverheatCooldownSeconds, false);

	return true;
}

void ASonicMicrowaveTool::FirePulse()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const FVector ToolForward = GetActorForwardVector();

	UAISense_Hearing::ReportNoiseEvent(World, Origin, 1.2f, GetOwner());

	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(TEXT("SonicMicrowavePulse"), false);
	Params.AddIgnoredActor(this);
	if (GetOwner())
	{
		Params.AddIgnoredActor(GetOwner());
	}

	World->OverlapMultiByChannel(Overlaps, Origin, FQuat::Identity, ECC_Visibility, FCollisionShape::MakeSphere(Range), Params);

	TSet<AActor*> AffectedActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* OverlappedActor = Overlap.GetActor();
		if (!OverlappedActor || OverlappedActor == GetOwner() || AffectedActors.Contains(OverlappedActor))
		{
			continue;
		}

		const FVector TargetLocation = OverlappedActor->GetActorLocation();
		FVector Direction = TargetLocation - Origin;
		const float Distance = Direction.Size();

		if (Distance < KINDA_SMALL_NUMBER || FVector::DotProduct(Direction / Distance, ToolForward) < 0.1f)
		{
			continue;
		}

		AffectedActors.Add(OverlappedActor);
		const float Falloff = 1.0f - FMath::Clamp(Distance / Range, 0.0f, 1.0f);

		if (ACharacter* Char = Cast<ACharacter>(OverlappedActor))
		{
			const FVector LaunchVel = (Direction.GetSafeNormal() + ToolForward * 0.35f + FVector(0.0f, 0.0f, 0.25f)).GetSafeNormal() * 1200.0f * Falloff;
			Char->LaunchCharacter(LaunchVel, true, true);

			if (ALiminalEntity* Entity = Cast<ALiminalEntity>(Char))
			{
				Entity->ApplyStun(2.5f);
			}
			continue;
		}

		UPrimitiveComponent* Primitive = Overlap.Component.Get();
		if (Primitive && Primitive->IsSimulatingPhysics())
		{
			const FVector ImpulseDirection = (Direction.GetSafeNormal() + ToolForward * 0.35f).GetSafeNormal();
			Primitive->AddImpulse(ImpulseDirection * KnockbackStrength * Falloff, NAME_None, true);
		}
	}
}

void ASonicMicrowaveTool::EndActiveWindow()
{
	Deactivate();
}

void ASonicMicrowaveTool::ClearOverheat()
{
	bIsOverheated = false;
}
