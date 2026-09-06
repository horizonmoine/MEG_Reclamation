#include "Tools/ChalkMarkerTool.h"
#include "Tools/ChalkTraceActor.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

AChalkMarkerTool::AChalkMarkerTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	ActiveBatteryDrainPerSecond = 0.0f;
	RemainingMarks = MaxMarks;
	PlacementTraceDistance = 300.0f;
}

bool AChalkMarkerTool::CanActivate() const
{
	return Super::CanActivate() && RemainingMarks > 0;
}

bool AChalkMarkerTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	return PlaceMarkOnSurface();
}

bool AChalkMarkerTool::PlaceMarkOnSurface()
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor || RemainingMarks <= 0)
	{
		return false;
	}

	FVector Start = OwnerActor->GetActorLocation();
	FRotator ViewRot = OwnerActor->GetActorRotation();
	OwnerActor->GetActorEyesViewPoint(Start, ViewRot);
	const FVector End = Start + (ViewRot.Vector() * PlacementTraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(OwnerActor);

	// Tente d'abord de marquer le mur ou l'objet directement vise
	bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params);
	if (!bHit)
	{
		// En repli, trace vers le bas pour marquer le sol sous les pieds
		const FVector DownVector = -FVector::UpVector * PlacementTraceDistance;
		bHit = World->LineTraceSingleByChannel(Hit, Start, Start + DownVector, ECC_WorldStatic, Params);
	}

	if (bHit)
	{
		RemainingMarks--;
		OnChalkMarkPlaced.Broadcast(Hit.ImpactPoint, RemainingMarks);

		if (HasAuthority())
		{
			const FVector SpawnLoc = Hit.ImpactPoint + (Hit.ImpactNormal * 1.5f);
			const FRotator SpawnRot = Hit.ImpactNormal.Rotation();

			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<AChalkTraceActor>(AChalkTraceActor::StaticClass(), SpawnLoc, SpawnRot, SpawnParams);
		}

		UE_LOG(LogTemp, Log, TEXT("[ChalkMarker] Mark placed at %s (%d marks remaining)"),
			*Hit.ImpactPoint.ToString(), RemainingMarks);
		return true;
	}

	return false;
}
