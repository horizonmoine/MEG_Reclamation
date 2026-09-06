#include "Objects/LiminalValvePuzzleActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Objects/LiminalDoorActor.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ALiminalValvePuzzleActor::ALiminalValvePuzzleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	PipeBoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PipeBoardMesh"));
	PipeBoardMesh->SetupAttachment(RootScene);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PipeFinder(
		TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe"));
	if (PipeFinder.Succeeded())
	{
		PipeBoardMesh->SetStaticMesh(PipeFinder.Object);
		PipeBoardMesh->SetRelativeScale3D(FVector(0.35f, 0.35f, 0.35f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (CubeFinder.Succeeded())
		{
			PipeBoardMesh->SetStaticMesh(CubeFinder.Object);
			PipeBoardMesh->SetRelativeScale3D(FVector(0.2f, 1.2f, 0.3f));
		}
	}

	Valve0Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Valve0Mesh"));
	Valve0Mesh->SetupAttachment(PipeBoardMesh);
	Valve0Mesh->SetRelativeLocation(FVector(0.0f, -40.0f, 0.0f));

	Valve1Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Valve1Mesh"));
	Valve1Mesh->SetupAttachment(PipeBoardMesh);
	Valve1Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));

	Valve2Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Valve2Mesh"));
	Valve2Mesh->SetupAttachment(PipeBoardMesh);
	Valve2Mesh->SetRelativeLocation(FVector(0.0f, 40.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ValveFinder(
		TEXT("/Game/Meshes/Puzzles/SM_SteamValve.SM_SteamValve"));
	if (ValveFinder.Succeeded())
	{
		Valve0Mesh->SetStaticMesh(ValveFinder.Object);
		Valve1Mesh->SetStaticMesh(ValveFinder.Object);
		Valve2Mesh->SetStaticMesh(ValveFinder.Object);
		Valve0Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		Valve1Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		Valve2Mesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

void ALiminalValvePuzzleActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalValvePuzzleActor, CurrentPressurePSI);
	DOREPLIFETIME(ALiminalValvePuzzleActor, bIsSolved);
}

void ALiminalValvePuzzleActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && LinkedDoor)
	{
		LinkedDoor->Lock();
	}
}

void ALiminalValvePuzzleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority() || bIsSolved)
	{
		return;
	}

	if (IsPressureInSafeZone())
	{
		CurrentStabilizedDuration += DeltaTime;
		if (CurrentStabilizedDuration >= RequiredStabilizationTime)
		{
			bIsSolved = true;
			OnPuzzleSolved.Broadcast(true);

			if (LinkedDoor)
			{
				LinkedDoor->Unlock();
			}
		}
	}
	else
	{
		CurrentStabilizedDuration = 0.0f;
	}
}

bool ALiminalValvePuzzleActor::IsPressureInSafeZone() const
{
	return CurrentPressurePSI >= SafePressureMin && CurrentPressurePSI <= SafePressureMax;
}

void ALiminalValvePuzzleActor::InteractWithValve(int32 ValveIndex, AActor* InInstigator)
{
	if (!HasAuthority() || bIsSolved)
	{
		return;
	}

	if (ValveIndex >= 0 && ValveIndex < 3)
	{
		CurrentPressurePSI = FMath::Clamp(CurrentPressurePSI + ValveDeltaPSI[ValveIndex], 0.0f, 300.0f);
		OnPressureChanged.Broadcast(CurrentPressurePSI);

		// Si surpression critique : souffle de vapeur brulante
		if (CurrentPressurePSI >= OverpressureThreshold && InInstigator)
		{
			UGameplayStatics::ApplyDamage(InInstigator, 15.0f, nullptr, this, nullptr);
		}
	}
}

void ALiminalValvePuzzleActor::SetLinkedDoor(ALiminalDoorActor* InDoor)
{
	LinkedDoor = InDoor;
	if (HasAuthority() && LinkedDoor && !bIsSolved)
	{
		LinkedDoor->Lock();
	}
}

void ALiminalValvePuzzleActor::OnRep_CurrentPressure()
{
	OnPressureChanged.Broadcast(CurrentPressurePSI);
}

void ALiminalValvePuzzleActor::OnRep_IsSolved()
{
	OnPuzzleSolved.Broadcast(bIsSolved);
}
