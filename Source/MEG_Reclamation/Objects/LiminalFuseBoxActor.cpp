#include "Objects/LiminalFuseBoxActor.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Objects/LiminalDoorActor.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ALiminalFuseBoxActor::ALiminalFuseBoxActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	BoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoxMesh"));
	BoxMesh->SetupAttachment(RootScene);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FuseBoxFinder(
		TEXT("/Game/Meshes/Puzzles/SM_FuseBox.SM_FuseBox"));
	if (FuseBoxFinder.Succeeded())
	{
		BoxMesh->SetStaticMesh(FuseBoxFinder.Object);
		BoxMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (CubeFinder.Succeeded())
		{
			BoxMesh->SetStaticMesh(CubeFinder.Object);
			BoxMesh->SetRelativeScale3D(FVector(0.2f, 0.4f, 0.6f));
		}
	}

	// Par defaut : 3 emplacements de fusibles (ex: 15A, 30A, 50A)
	RequiredFuses = { 15, 30, 50 };
	SlottedFuses = { 0, 0, 0 };
}

void ALiminalFuseBoxActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalFuseBoxActor, SlottedFuses);
	DOREPLIFETIME(ALiminalFuseBoxActor, bPowerRestored);
}

void ALiminalFuseBoxActor::BeginPlay()
{
	Super::BeginPlay();

	if (SlottedFuses.Num() != RequiredFuses.Num())
	{
		SlottedFuses.Init(0, RequiredFuses.Num());
	}

	if (HasAuthority() && LinkedDoor)
	{
		LinkedDoor->Lock();
	}
}

bool ALiminalFuseBoxActor::InsertFuse(int32 SlotIndex, int32 FuseAmperage, AActor* InInstigator)
{
	if (!HasAuthority() || bPowerRestored || !SlottedFuses.IsValidIndex(SlotIndex))
	{
		return false;
	}

	// Verifier si le fusible est de sur-amperage dangereux (> 2x la valeur requise) : arc electrique
	if (RequiredFuses.IsValidIndex(SlotIndex) && FuseAmperage > RequiredFuses[SlotIndex] * 2 && InInstigator)
	{
		UGameplayStatics::ApplyDamage(InInstigator, 20.0f, nullptr, this, nullptr);
	}

	SlottedFuses[SlotIndex] = FuseAmperage;
	CheckPowerStatus();
	return true;
}

void ALiminalFuseBoxActor::Interact(AActor* InInstigator)
{
	if (!HasAuthority() || bPowerRestored)
	{
		return;
	}

	for (int32 i = 0; i < SlottedFuses.Num(); ++i)
	{
		if (SlottedFuses[i] == 0)
		{
			const int32 NeededAmp = RequiredFuses.IsValidIndex(i) ? RequiredFuses[i] : 15;
			InsertFuse(i, NeededAmp, InInstigator);
			return;
		}
	}
}

int32 ALiminalFuseBoxActor::RemoveFuse(int32 SlotIndex)
{
	if (!HasAuthority() || !SlottedFuses.IsValidIndex(SlotIndex))
	{
		return 0;
	}

	const int32 Removed = SlottedFuses[SlotIndex];
	SlottedFuses[SlotIndex] = 0;
	CheckPowerStatus();
	return Removed;
}

int32 ALiminalFuseBoxActor::GetSlottedFuse(int32 SlotIndex) const
{
	return SlottedFuses.IsValidIndex(SlotIndex) ? SlottedFuses[SlotIndex] : 0;
}

void ALiminalFuseBoxActor::SetLinkedDoor(ALiminalDoorActor* InDoor)
{
	LinkedDoor = InDoor;
	if (HasAuthority() && LinkedDoor && !bPowerRestored)
	{
		LinkedDoor->Lock();
	}
}

void ALiminalFuseBoxActor::CheckPowerStatus()
{
	if (!HasAuthority())
	{
		return;
	}

	bool bAllMatch = true;
	for (int32 i = 0; i < RequiredFuses.Num(); ++i)
	{
		if (!SlottedFuses.IsValidIndex(i) || SlottedFuses[i] != RequiredFuses[i])
		{
			bAllMatch = false;
			break;
		}
	}

	if (bAllMatch != bPowerRestored)
	{
		bPowerRestored = bAllMatch;
		OnPowerStateChanged.Broadcast(bPowerRestored);

		if (LinkedDoor)
		{
			if (bPowerRestored)
			{
				LinkedDoor->Unlock();
			}
			else
			{
				LinkedDoor->Lock();
			}
		}
	}
}

void ALiminalFuseBoxActor::OnRep_SlottedFuses()
{
	// Visuels de fusibles mis a jour cote client
}

void ALiminalFuseBoxActor::OnRep_PowerRestored()
{
	OnPowerStateChanged.Broadcast(bPowerRestored);
}
