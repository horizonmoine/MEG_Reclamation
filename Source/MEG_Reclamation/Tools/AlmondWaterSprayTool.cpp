#include "Tools/AlmondWaterSprayTool.h"

#include "AI/LiminalEntity.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Player/ScavengerCharacter.h"
#include "UObject/ConstructorHelpers.h"

AAlmondWaterSprayTool::AAlmondWaterSprayTool()
{
	ActiveBatteryDrainPerSecond = 15.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SprayFinder(
		TEXT("/Game/Meshes/Tools/SM_AlmondWaterSpray.SM_AlmondWaterSpray"));
	if (SprayFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(SprayFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> BottleFinder(
			TEXT("/Game/Meshes/Props/SM_Almond_Water_Bottle.SM_Almond_Water_Bottle"));
		if (BottleFinder.Succeeded() && ToolMesh)
		{
			ToolMesh->SetStaticMesh(BottleFinder.Object);
			ToolMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		}
	}
}

void AAlmondWaterSprayTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!IsActive())
	{
		return;
	}

	if (HasAuthority())
	{
		if (AActor* Target = TraceSprayTarget())
		{
			ApplyCalmingEffect(Target, DeltaSeconds);
		}
		else if (AScavengerCharacter* OwnerChar = Cast<AScavengerCharacter>(GetOwner()))
		{
			OwnerChar->ServerRestoreSanity(12.0f * DeltaSeconds);
		}
	}
}

AActor* AAlmondWaterSprayTool::TraceSprayTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * SprayRange;

	FCollisionQueryParams Params(TEXT("AlmondWaterSpray"), false);
	Params.AddIgnoredActor(this);
	if (GetOwner())
	{
		Params.AddIgnoredActor(GetOwner());
	}

	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(25.0f), Params);

	return bHit ? HitResult.GetActor() : nullptr;
}

void AAlmondWaterSprayTool::ApplyCalmingEffect(AActor* Target, float DeltaSeconds)
{
	if (!Target || !HasAuthority())
	{
		return;
	}

	if (AScavengerCharacter* Ally = Cast<AScavengerCharacter>(Target))
	{
		Ally->ServerRestoreSanity(25.0f * DeltaSeconds);
	}
	else if (ALiminalEntity* Entity = Cast<ALiminalEntity>(Target))
	{
		Entity->ApplyCalm(4.0f);
	}
}
