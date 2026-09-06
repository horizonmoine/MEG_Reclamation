#include "Tools/RealityAnchorTool.h"
#include "Player/ScavengerCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ARealityAnchorTool::ARealityAnchorTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	ActiveBatteryDrainPerSecond = 0.0f;
	RemainingFieldDuration = MaxFieldDurationSeconds;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> AnchorFinder(
		TEXT("/Game/Meshes/Tools/SM_Reality_Anchor.SM_Reality_Anchor"));
	if (AnchorFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(AnchorFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f));
	}
}

bool ARealityAnchorTool::CanActivate() const
{
	return Super::CanActivate() && RemainingFieldDuration > 0.0f;
}

bool ARealityAnchorTool::Activate()
{
	if (!CanActivate())
	{
		return false;
	}

	if (!bIsDeployed)
	{
		DeployAnchor();
	}
	else
	{
		PackUpAnchor();
	}
	return true;
}

void ARealityAnchorTool::Deactivate()
{
	Super::Deactivate();
	PackUpAnchor();
}

void ARealityAnchorTool::DeployAnchor()
{
	bIsDeployed = true;
	bIsActive = true;
	OnAnchorStateChanged.Broadcast(true);

	// Detach from parent and fix onto the ground
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

	UE_LOG(LogTemp, Log, TEXT("[RealityAnchor] Deployed at %s (Radius: %.0f, Duration: %.1fs)"),
		*GetActorLocation().ToString(), StabilizationRadius, RemainingFieldDuration);
}

void ARealityAnchorTool::PackUpAnchor()
{
	if (bIsDeployed)
	{
		bIsDeployed = false;
		bIsActive = false;

		// Clear anchor effect on all scavengers
		if (UWorld* World = GetWorld())
		{
			for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
			{
				if (AScavengerCharacter* Scavenger = *It)
				{
					Scavenger->SetInsideRealityAnchor(false);
				}
			}
		}

		OnAnchorStateChanged.Broadcast(false);
		UE_LOG(LogTemp, Log, TEXT("[RealityAnchor] Packed up."));
	}
}

void ARealityAnchorTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsDeployed)
	{
		UpdateStabilizationField(DeltaSeconds);
	}
}

void ARealityAnchorTool::UpdateStabilizationField(float DeltaSeconds)
{
	RemainingFieldDuration -= DeltaSeconds;
	if (RemainingFieldDuration <= 0.0f)
	{
		PackUpAnchor();
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector AnchorLocation = GetActorLocation();

	// Check all scavengers within radius
	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Scavenger = *It;
		if (Scavenger && !Scavenger->IsDead())
		{
			const float Dist = FVector::Dist(AnchorLocation, Scavenger->GetActorLocation());
			const bool bInside = (Dist <= StabilizationRadius);
			Scavenger->SetInsideRealityAnchor(bInside);
		}
	}
}
