#include "Tools/FlashStrobeTool.h"

#include "AI/LiminalEntity.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AFlashStrobeTool::AFlashStrobeTool()
{
	ActiveBatteryDrainPerSecond = 25.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> StrobeFinder(
		TEXT("/Game/Meshes/Tools/SM_FlashStrobe.SM_FlashStrobe"));
	if (StrobeFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(StrobeFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}

	StrobeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StrobeLight"));
	StrobeLight->SetupAttachment(ToolMesh);
	StrobeLight->SetIntensity(0.0f);
}

bool AFlashStrobeTool::Activate()
{
	if (!Super::Activate())
	{
		return false;
	}

	StrobePhaseSeconds = 0.0f;
	return true;
}

void AFlashStrobeTool::Deactivate()
{
	Super::Deactivate();

	if (StrobeLight)
	{
		StrobeLight->SetIntensity(0.0f);
	}
}

void AFlashStrobeTool::OnRep_IsActive()
{
	Super::OnRep_IsActive();

	if (!IsActive() && StrobeLight)
	{
		StrobeLight->SetIntensity(0.0f);
	}
}

void AFlashStrobeTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsActive())
	{
		return;
	}

	StrobePhaseSeconds += DeltaSeconds;

	const float CyclePosition = FMath::Frac(StrobePhaseSeconds * StrobeFrequencyHz);
	const bool bFlashOn = CyclePosition < 0.5f;

	if (StrobeLight)
	{
		StrobeLight->SetIntensity(bFlashOn ? MaxIntensity : 0.0f);
	}

	if (bFlashOn && HasAuthority())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			return;
		}

		const FVector FlashOrigin = GetActorLocation();
		const FVector FlashForward = GetActorForwardVector();

		for (TActorIterator<ALiminalEntity> It(World); It; ++It)
		{
			ALiminalEntity* Entity = *It;
			if (!Entity || Entity->IsStunned())
			{
				continue;
			}

			const FVector ToEntity = Entity->GetActorLocation() - FlashOrigin;
			const float DistSq = ToEntity.SizeSquared();
			if (DistSq <= FMath::Square(StrobeRange))
			{
				const FVector Dir = ToEntity.GetSafeNormal();
				if (FVector::DotProduct(Dir, FlashForward) > 0.35f)
				{
					Entity->ApplyStun(StunDuration);
				}
			}
		}
	}
}
