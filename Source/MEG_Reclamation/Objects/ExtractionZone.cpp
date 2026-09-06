#include "Objects/ExtractionZone.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameModes/LiminalGameMode.h"
#include "Player/ScavengerCharacter.h"
#include "Objects/LiminalBreakerActor.h"
#include "Objects/LiminalKeypadActor.h"
#include "UObject/ConstructorHelpers.h"

AExtractionZone::AExtractionZone()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	SetRootComponent(ZoneBox);
	ZoneBox->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
	ZoneBox->SetCollisionProfileName(TEXT("Trigger"));

	FrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FrameMesh"));
	FrameMesh->SetupAttachment(ZoneBox);
	FrameMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -150.0f));
	FrameMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FrameMeshFinder(
		TEXT("/Game/Meshes/Modular/SM_Door_Frame.SM_Door_Frame"));
	if (FrameMeshFinder.Succeeded())
	{
		FrameMesh->SetStaticMesh(FrameMeshFinder.Object);
	}

	ExitSignMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExitSignMesh"));
	ExitSignMesh->SetupAttachment(ZoneBox);
	ExitSignMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 130.0f));
	ExitSignMesh->SetCollisionProfileName(TEXT("NoCollision"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ExitSignFinder(
		TEXT("/Game/Meshes/Props/SM_Exit_Sign.SM_Exit_Sign"));
	if (ExitSignFinder.Succeeded())
	{
		ExitSignMesh->SetStaticMesh(ExitSignFinder.Object);
	}

	MarkerLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("MarkerLight"));
	MarkerLight->SetupAttachment(ZoneBox);
	MarkerLight->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));
	MarkerLight->SetLightColor(FLinearColor(0.2f, 1.0f, 0.3f));
	MarkerLight->SetIntensity(8000.0f);
	MarkerLight->SetAttenuationRadius(600.0f);
}

void AExtractionZone::BeginPlay()
{
	Super::BeginPlay();

	if (ZoneBox)
	{
		ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &AExtractionZone::OnZoneBeginOverlap);
	}
}

void AExtractionZone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const bool bUnlocked = IsExtractionUnlocked();

	if (MarkerLight)
	{
		if (bUnlocked)
		{
			MarkerLight->SetLightColor(FLinearColor(0.2f, 1.0f, 0.3f));
			MarkerLight->SetIntensity(8000.0f);
		}
		else
		{
			// Pulsation d'alerte rouge sang quand le sas est verrouille
			const float Pulse = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 5.0f);
			MarkerLight->SetLightColor(FLinearColor(1.0f, 0.1f, 0.1f));
			MarkerLight->SetIntensity(4000.0f * Pulse);
		}
	}

	if (ExitSignMesh)
	{
		// Custom depth stencil / visual feedback for extraction status
		ExitSignMesh->SetRenderCustomDepth(true);
		ExitSignMesh->SetCustomDepthStencilValue(bUnlocked ? 1 : 2);
	}
}

bool AExtractionZone::IsExtractionUnlocked() const
{
	if (bRequiresPower)
	{
		if (!RequiredBreaker || !RequiredBreaker->IsPowerRestored())
		{
			return false;
		}
	}

	if (bRequiresKeypad)
	{
		if (!RequiredKeypad || !RequiredKeypad->IsUnlocked())
		{
			return false;
		}
	}

	return true;
}

void AExtractionZone::SetRequiresPower(bool bReq, ALiminalBreakerActor* InBreaker)
{
	bRequiresPower = bReq;
	if (InBreaker)
	{
		RequiredBreaker = InBreaker;
	}
}

void AExtractionZone::SetRequiresKeypad(bool bReq, ALiminalKeypadActor* InKeypad)
{
	bRequiresKeypad = bReq;
	if (InKeypad)
	{
		RequiredKeypad = InKeypad;
	}
}

void AExtractionZone::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority() || !OtherActor)
	{
		return;
	}

	AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(OtherActor);
	if (!Scavenger)
	{
		return;
	}

	// Si l'extraction necessite de resoudre une enigme (disjoncteur ou digicode)
	if (!IsExtractionUnlocked())
	{
		UE_LOG(LogTemp, Warning, TEXT("ExtractionZone : Acces refuse a %s — Condition de deverrouillage non remplie (Courant ou Digicode)"), *Scavenger->GetName());
		return;
	}

	const double Now = GetWorld()->GetTimeSeconds();
	if (Now - LastDeliveryTime < CooldownSeconds)
	{
		return;
	}

	if (Scavenger->GetCarriedCredits() > 0 || Scavenger->GetWeightRatio() > 0.0f)
	{
		DeliverLoot(Scavenger);
	}
}

void AExtractionZone::DeliverLoot(AScavengerCharacter* Scavenger)
{
	const int32 Delivered = Scavenger->GetCarriedCredits();

	LastDeliveryTime = GetWorld()->GetTimeSeconds();
	Scavenger->DeliverCarriedLoot();

	if (ALiminalGameMode* GameMode = Cast<ALiminalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->TriggerExtraction(Scavenger);
	}

	UE_LOG(LogTemp, Log, TEXT("ExtractionZone : %d credits livres par %s"), Delivered, *Scavenger->GetName());
}
