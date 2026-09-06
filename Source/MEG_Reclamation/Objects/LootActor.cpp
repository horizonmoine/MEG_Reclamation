#include "Objects/LootActor.h"

#include "Components/StaticMeshComponent.h"
#include "Data/ItemData.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "Sound/SoundBase.h"

ALootActor::ALootActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetCollisionProfileName(TEXT("PhysicsActor"));

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultImpactSound(
		TEXT("/Game/Audio/S_Loot_Impact_Metal.S_Loot_Impact_Metal"));
	if (DefaultImpactSound.Succeeded())
	{
		ImpactSound = DefaultImpactSound.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SupplyCrateFinder(
		TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"));
	if (SupplyCrateFinder.Succeeded())
	{
		MeshComp->SetStaticMesh(SupplyCrateFinder.Object);
		MeshComp->SetWorldScale3D(FVector(0.75f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMeshFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (DefaultMeshFinder.Succeeded())
		{
			MeshComp->SetStaticMesh(DefaultMeshFinder.Object);
			MeshComp->SetWorldScale3D(FVector(0.5f));
		}
	}
}

void ALootActor::BeginPlay()
{
	Super::BeginPlay();

	if (MeshComp)
	{
		MeshComp->OnComponentHit.AddDynamic(this, &ALootActor::OnLootHit);
	}
}

void ALootActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALootActor, ClaimedBy);
	DOREPLIFETIME(ALootActor, ItemData);
	DOREPLIFETIME(ALootActor, CustomMesh);
	DOREPLIFETIME(ALootActor, LootItemId);
	DOREPLIFETIME(ALootActor, RandomizedWeightKg);
	DOREPLIFETIME(ALootActor, RandomizedCreditsValue);
}

void ALootActor::OnRep_ClaimedBy()
{
	if (MeshComp)
	{
		if (ClaimedBy)
		{
			MeshComp->SetSimulatePhysics(false);
			MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		else
		{
			MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			MeshComp->SetSimulatePhysics(true);
		}
	}
}

void ALootActor::OnRep_ItemData()
{
	if (ItemData && ItemData->ItemMesh && MeshComp)
	{
		MeshComp->SetStaticMesh(ItemData->ItemMesh);
	}
}

void ALootActor::OnRep_CustomMesh()
{
	if (CustomMesh && MeshComp)
	{
		MeshComp->SetStaticMesh(CustomMesh);
	}
}

void ALootActor::SetCustomLoot(UStaticMesh* InMesh, USoundBase* InSound, float InWeight, int32 InCredits, FName InItemId)
{
	CustomMesh = InMesh;
	LootItemId = InItemId;
	if (InMesh && MeshComp)
	{
		MeshComp->SetStaticMesh(InMesh);
	}
	if (InSound)
	{
		ImpactSound = InSound;
	}
	SetRandomizedStats(InWeight, InCredits);
}

float ALootActor::GetWeightKg() const
{
	if (RandomizedWeightKg > 0.0f)
	{
		return RandomizedWeightKg;
	}
	return ItemData ? ItemData->WeightKg : FallbackWeightKg;
}

int32 ALootActor::GetCreditsValue() const
{
	if (RandomizedCreditsValue > 0)
	{
		return RandomizedCreditsValue;
	}
	return ItemData ? ItemData->CreditsValue : 10;
}

void ALootActor::SetRandomizedStats(float InWeight, int32 InCredits)
{
	RandomizedWeightKg = FMath::Max(0.5f, InWeight);
	RandomizedCreditsValue = FMath::Max(1, InCredits);
}

void ALootActor::HighlightLoot(bool bEnable)
{
	if (MeshComp)
	{
		MeshComp->SetRenderCustomDepth(bEnable);
		MeshComp->SetCustomDepthStencilValue(bEnable ? 2 : 0);
	}
}

void ALootActor::SetItemData(UItemData* InItemData)
{
	ItemData = InItemData;
	if (ItemData && ItemData->ItemMesh && MeshComp)
	{
		MeshComp->SetStaticMesh(ItemData->ItemMesh);
	}
}

bool ALootActor::IsClaimed() const
{
	return ClaimedBy != nullptr;
}

void ALootActor::SetClaimed(AScavengerCharacter* Claimant)
{
	ClaimedBy = Claimant;
	OnRep_ClaimedBy();
}

UPrimitiveComponent* ALootActor::GetLootPrimitive() const
{
	return MeshComp;
}

void ALootActor::OnLootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	const float ImpulseStrength = NormalImpulse.Size();
	if (ImpulseStrength < MinImpactNoiseThreshold || !GetWorld())
	{
		return;
	}

	const float Loudness = FMath::Min(ImpulseStrength / MinImpactNoiseThreshold, MaxImpactNoiseLoudness);
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), Loudness, this);

	USoundBase* SoundToPlay = (ItemData && ItemData->CollisionSound) ? ItemData->CollisionSound.Get() : ImpactSound.Get();
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), SoundToPlay, GetActorLocation(),
			FMath::Clamp(Loudness, 0.2f, 1.0f), FMath::FRandRange(0.85f, 1.15f));
	}
}
