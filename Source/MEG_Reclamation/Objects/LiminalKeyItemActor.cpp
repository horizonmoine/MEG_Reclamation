#include "Objects/LiminalKeyItemActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"
#include "UObject/ConstructorHelpers.h"

ALiminalKeyItemActor::ALiminalKeyItemActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	SetRootComponent(CollisionSphere);
	CollisionSphere->SetSphereRadius(60.0f);
	CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CollisionSphere->SetGenerateOverlapEvents(true);

	KeyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeyMesh"));
	KeyMesh->SetupAttachment(CollisionSphere);
	KeyMesh->SetCollisionProfileName(TEXT("NoCollision"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> KeycardFinder(
		TEXT("/Game/Meshes/Puzzles/SM_Keycard.SM_Keycard"));
	if (KeycardFinder.Succeeded())
	{
		KeyMesh->SetStaticMesh(KeycardFinder.Object);
		KeyMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (CubeFinder.Succeeded())
		{
			KeyMesh->SetStaticMesh(CubeFinder.Object);
			KeyMesh->SetRelativeScale3D(FVector(0.1f, 0.15f, 0.02f));
		}
	}

	DisplayName = FText::FromString(TEXT("Pass d'acces M.E.G."));
}

void ALiminalKeyItemActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalKeyItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalKeyItemActor, KeyId);
	DOREPLIFETIME(ALiminalKeyItemActor, KeycardLevel);
	DOREPLIFETIME(ALiminalKeyItemActor, DisplayName);
	DOREPLIFETIME(ALiminalKeyItemActor, bIsCollected);
}

void ALiminalKeyItemActor::SetKeyData(const FName& InKeyId, EKeycardLevel InLevel, const FText& InDisplayName)
{
	if (!HasAuthority())
	{
		return;
	}

	KeyId = InKeyId;
	KeycardLevel = InLevel;
	DisplayName = InDisplayName;
}

bool ALiminalKeyItemActor::TryCollectKey(AScavengerCharacter* InScavenger)
{
	if (!HasAuthority() || bIsCollected || !InScavenger)
	{
		return false;
	}

	bIsCollected = true;
	InScavenger->Tags.AddUnique(KeyId);

	OnKeyCollected.Broadcast(InScavenger, KeyId);
	OnRep_IsCollected();

	SetLifeSpan(0.1f);
	return true;
}

void ALiminalKeyItemActor::OnRep_IsCollected()
{
	if (bIsCollected)
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}
}
