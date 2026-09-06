#include "Tools/HarmonicResonatorTool.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AHarmonicResonatorTool::AHarmonicResonatorTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	bIsExpended = false;
	SuccessChance = 0.70f;
	MaxWallThickness = 120.0f;
}

bool AHarmonicResonatorTool::CanActivate() const
{
	return !bIsExpended && Super::CanActivate();
}

bool AHarmonicResonatorTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	AActor* ToolOwner = GetOwner();
	if (!ToolOwner)
	{
		return false;
	}

	APawn* PawnOwner = Cast<APawn>(ToolOwner);
	if (!PawnOwner)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	bIsExpended = true;
	bIsActive = true;

	const FVector EyeLoc = PawnOwner->GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);
	const FVector ForwardDir = PawnOwner->GetActorForwardVector();
	const FVector TraceEnd = EyeLoc + ForwardDir * 200.0f;

	FHitResult WallHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(ToolOwner);

	const bool bHit = World->LineTraceSingleByChannel(WallHit, EyeLoc, TraceEnd, ECC_Visibility, Params);

	const float Roll = FMath::FRand();
	if (Roll <= SuccessChance)
	{
		// 70% Succes : No-Clip a travers la cloison
		FVector Destination = bHit ? (WallHit.ImpactPoint + ForwardDir * (MaxWallThickness + 80.0f)) : (EyeLoc + ForwardDir * 350.0f);
		Destination.Z = PawnOwner->GetActorLocation().Z;
		PawnOwner->TeleportTo(Destination, PawnOwner->GetActorRotation());
	}
	else
	{
		// 30% Echec critique : teleportation instable ou degats de desynchronisation
		FVector ChaoticOffset = FMath::VRand() * 150.0f;
		ChaoticOffset.Z = 0.0f;
		PawnOwner->TeleportTo(PawnOwner->GetActorLocation() + ChaoticOffset, PawnOwner->GetActorRotation());
		UGameplayStatics::ApplyDamage(PawnOwner, 25.0f, PawnOwner->GetController(), this, UDamageType::StaticClass());
	}

	BatteryCharge = 0.0f;
	bIsActive = false;
	return true;
}
