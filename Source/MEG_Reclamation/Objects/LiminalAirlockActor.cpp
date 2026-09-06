#include "Objects/LiminalAirlockActor.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/LiminalGameInstance.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"
#include "Perception/AISense_Hearing.h"

ALiminalAirlockActor::ALiminalAirlockActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	ChamberVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("ChamberVolume"));
	SetRootComponent(ChamberVolume);
	ChamberVolume->SetBoxExtent(FVector(200.0f, 200.0f, 150.0f));
	ChamberVolume->SetCollisionProfileName(TEXT("Trigger"));

	InteractionTrigger = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionTrigger"));
	InteractionTrigger->SetupAttachment(ChamberVolume);
	InteractionTrigger->SetRelativeLocation(FVector(180.0f, 0.0f, 0.0f));
	InteractionTrigger->SetBoxExtent(FVector(60.0f, 60.0f, 80.0f));
	InteractionTrigger->SetCollisionProfileName(TEXT("Trigger"));

	AirlockFrame = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AirlockFrame"));
	AirlockFrame->SetupAttachment(ChamberVolume);
	AirlockFrame->SetCollisionProfileName(TEXT("BlockAll"));

	LeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeverMesh"));
	LeverMesh->SetupAttachment(InteractionTrigger);
	LeverMesh->SetRelativeScale3D(FVector(0.2f, 0.2f, 0.6f));
	LeverMesh->SetCollisionProfileName(TEXT("BlockAll"));

	StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
	StatusLight->SetupAttachment(ChamberVolume);
	StatusLight->SetRelativeLocation(FVector(0.0f, 0.0f, 120.0f));
	StatusLight->SetLightColor(FLinearColor(0.1f, 1.0f, 0.3f)); // Vert = Pret
	StatusLight->SetIntensity(5000.0f);
	StatusLight->SetAttenuationRadius(800.0f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FrameFinder(
		TEXT("/Game/Meshes/Modular/SM_Door_Frame.SM_Door_Frame"));
	if (FrameFinder.Succeeded())
	{
		AirlockFrame->SetStaticMesh(FrameFinder.Object);
		AirlockFrame->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		AirlockFrame->SetRelativeLocation(FVector(0.0f, 0.0f, -140.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (CubeFinder.Succeeded())
		{
			AirlockFrame->SetStaticMesh(CubeFinder.Object);
			AirlockFrame->SetRelativeScale3D(FVector(4.2f, 4.2f, 0.2f));
			AirlockFrame->SetRelativeLocation(FVector(0.0f, 0.0f, -140.0f));
		}
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BreakerFinder(
		TEXT("/Game/Meshes/Puzzles/SM_Breaker.SM_Breaker"));
	if (BreakerFinder.Succeeded())
	{
		LeverMesh->SetStaticMesh(BreakerFinder.Object);
		LeverMesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 0.4f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (CubeFinder.Succeeded())
		{
			LeverMesh->SetStaticMesh(CubeFinder.Object);
		}
	}
}

void ALiminalAirlockActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalAirlockActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalAirlockActor, bIsCycleActive);
}

void ALiminalAirlockActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsCycleActive)
	{
		// Clignotement stroboscopique ambre/rouge d'alerte de decontamination
		if (StatusLight)
		{
			const float StrobeAlpha = 0.5f + 0.5f * FMath::Sin(GetWorld()->GetTimeSeconds() * 20.0f);
			const FLinearColor StrobeColor = FMath::Lerp(FLinearColor(1.0f, 0.45f, 0.05f), FLinearColor(1.0f, 0.02f, 0.02f), StrobeAlpha);
			StatusLight->SetLightColor(StrobeColor);
			StatusLight->SetIntensity(3000.0f + 7000.0f * StrobeAlpha);
		}

		if (HasAuthority())
		{
			CycleTimer -= DeltaSeconds;
			if (CycleTimer <= 0.0f)
			{
				bIsCycleActive = false;
				OnRep_CycleActive();
				if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
				{
					GI->TravelToMission();
				}
			}
		}
	}
}

void ALiminalAirlockActor::Interact(AScavengerCharacter* Operator)
{
	if (!Operator || bIsCycleActive)
	{
		return;
	}

	ServerActivateAirlock(Operator);
}

void ALiminalAirlockActor::ServerActivateAirlock_Implementation(AScavengerCharacter* Operator)
{
	if (!HasAuthority() || bIsCycleActive)
	{
		return;
	}

	bIsCycleActive = true;
	CycleTimer = 2.5f;
	OnRep_CycleActive();

	// Signal acoustique de purge sous pression (alerte le Hub et les capteurs)
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, Operator);
	MakeNoise(1.0f, Operator, GetActorLocation());

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow,
			TEXT("[SAS M.E.G.] Decontamination et purge sous pression... Scellage des portes !"));
	}
}

void ALiminalAirlockActor::OnRep_CycleActive()
{
	if (StatusLight)
	{
		if (bIsCycleActive)
		{
			StatusLight->SetLightColor(FLinearColor(1.0f, 0.1f, 0.1f)); // Rouge alerte
			StatusLight->SetIntensity(9000.0f);
		}
		else
		{
			StatusLight->SetLightColor(FLinearColor(0.1f, 1.0f, 0.3f)); // Vert pret
			StatusLight->SetIntensity(5000.0f);
		}
	}
}
