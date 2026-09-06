#include "Sanity/LiminalHallucinationActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"

ALiminalHallucinationActor::ALiminalHallucinationActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false; // Asymmetric client-side hallucination only

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	RootComponent = VisualMesh;

	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	VisualMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshFinder.Succeeded())
	{
		VisualMesh->SetStaticMesh(CubeMeshFinder.Object);
	}
}

void ALiminalHallucinationActor::BeginPlay()
{
	Super::BeginPlay();
	InitialLocation = GetActorLocation();
}

void ALiminalHallucinationActor::ConfigureHallucination(EHallucinationType InType, float InLifespan)
{
	HallucinationType = InType;
	MaxLifespan = InLifespan;

	switch (HallucinationType)
	{
	case EHallucinationType::ShadowSilhouette:
		SetActorScale3D(FVector(0.4f, 0.4f, 1.8f)); // Silhouette humanoide menacante
		break;
	case EHallucinationType::FakeDoor:
		if (VisualMesh)
		{
			if (UStaticMesh* DoorMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/Meshes/Modular/SM_Door_Leaf.SM_Door_Leaf"))))
			{
				VisualMesh->SetStaticMesh(DoorMesh);
			}
		}
		SetActorScale3D(FVector(1.0f, 1.0f, 1.0f)); // Fausse porte de secours trompeuse
		break;
	case EHallucinationType::FakeLoot:
		if (VisualMesh)
		{
			if (UStaticMesh* CrateMesh = Cast<UStaticMesh>(StaticLoadObject(UStaticMesh::StaticClass(), nullptr, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"))))
			{
				VisualMesh->SetStaticMesh(CrateMesh);
			}
		}
		SetActorScale3D(FVector(0.85f, 0.85f, 0.85f)); // Fausse caisse de ravitaillement
		break;
	case EHallucinationType::PhantomAuditory:
		SetActorScale3D(FVector(0.1f, 0.1f, 0.1f));
		break;
	}

	if (VisualMesh)
	{
		if (UMaterialInstanceDynamic* DynMat = VisualMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			switch (HallucinationType)
			{
			case EHallucinationType::ShadowSilhouette:
				DynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.01f, 0.01f, 0.01f, 1.0f));
				break;
			case EHallucinationType::FakeDoor:
				DynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.18f, 0.12f, 0.06f, 1.0f));
				break;
			case EHallucinationType::FakeLoot:
				DynMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.85f, 0.75f, 0.15f, 1.0f));
				break;
			default:
				break;
			}
		}
	}
}

void ALiminalHallucinationActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	LifeTimer += DeltaSeconds;
	if (LifeTimer >= MaxLifespan && !bIsVanishing)
	{
		Vanish();
	}

	if (bIsVanishing)
	{
		VanishTimer += DeltaSeconds;
		const float Alpha = 1.0f - FMath::Clamp(VanishTimer / VanishFadeDuration, 0.0f, 1.0f);
		SetActorScale3D(GetActorScale3D() * Alpha);

		if (VanishTimer >= VanishFadeDuration)
		{
			Destroy();
		}
		return;
	}

	// Jitter effect
	if (JitterIntensity > 0.0f)
	{
		const FVector JitterOffset(
			FMath::FRandRange(-JitterIntensity, JitterIntensity),
			FMath::FRandRange(-JitterIntensity, JitterIntensity),
			FMath::FRandRange(-JitterIntensity * 0.5f, JitterIntensity * 0.5f)
		);
		SetActorLocation(InitialLocation + JitterOffset);
	}

	// Check distance to local player pawn
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		if (APawn* PlayerPawn = PC->GetPawn())
		{
			const float DistSq = FVector::DistSquared(PlayerPawn->GetActorLocation(), GetActorLocation());
			if (DistSq <= FMath::Square(VanishDistance))
			{
				Vanish();
			}
		}
	}
}

void ALiminalHallucinationActor::Vanish()
{
	if (bIsVanishing)
	{
		return;
	}

	bIsVanishing = true;
	VanishTimer = 0.0f;
	UE_LOG(LogTemp, Verbose, TEXT("[LiminalHallucination] Hallucination vanished near player."));
}
