#include "Objects/LiminalBreakerActor.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameModes/LiminalGameMode.h"
#include "Perception/AISense_Hearing.h"
#include "UObject/ConstructorHelpers.h"

ALiminalBreakerActor::ALiminalBreakerActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	BreakerBoxMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BreakerBoxMesh"));
	BreakerBoxMesh->SetupAttachment(RootScene);
	BreakerBoxMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BreakerFinder(
		TEXT("/Game/Meshes/Puzzles/SM_Breaker.SM_Breaker"));
	if (BreakerFinder.Succeeded())
	{
		BreakerBoxMesh->SetStaticMesh(BreakerFinder.Object);
		BreakerBoxMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> BoxFinder(
			TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (BoxFinder.Succeeded())
		{
			BreakerBoxMesh->SetStaticMesh(BoxFinder.Object);
			BreakerBoxMesh->SetRelativeScale3D(FVector(0.15f, 0.45f, 0.65f));
		}
	}

	SwitchLeverMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SwitchLeverMesh"));
	SwitchLeverMesh->SetupAttachment(RootScene);
	SwitchLeverMesh->SetRelativeLocation(FVector(8.0f, 0.0f, 0.0f));
	SwitchLeverMesh->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.25f));
	SwitchLeverMesh->SetCollisionProfileName(TEXT("NoCollision"));

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootScene);
	InteractionBox->SetBoxExtent(FVector(70.0f, 70.0f, 70.0f));
	InteractionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	IndicatorLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("IndicatorLight"));
	IndicatorLight->SetupAttachment(RootScene);
	IndicatorLight->SetRelativeLocation(FVector(15.0f, 0.0f, 25.0f));
	IndicatorLight->SetLightColor(FLinearColor(1.0f, 0.1f, 0.1f));
	IndicatorLight->SetIntensity(200.0f);
	IndicatorLight->SetAttenuationRadius(250.0f);
	IndicatorLight->SetCastShadows(false);
}

void ALiminalBreakerActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateVisualState();
}

void ALiminalBreakerActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalBreakerActor, bIsPowerRestored);
}

void ALiminalBreakerActor::ToggleBreaker()
{
	SetBreakerState(!bIsPowerRestored);
}

void ALiminalBreakerActor::SetBreakerState(bool bNewState)
{
	if (bIsPowerRestored == bNewState)
	{
		return;
	}

	bIsPowerRestored = bNewState;
	UpdateVisualState();

	// Clac mecanique lourd audible
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.75f, this);

	// Si active et qu'une panne de courant etait en cours, retablit l'alimentation
	if (bIsPowerRestored)
	{
		if (ALiminalGameMode* GM = Cast<ALiminalGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr))
		{
			GM->RestorePower();
		}
	}

	OnBreakerSwitched.Broadcast(this, bIsPowerRestored);
}

void ALiminalBreakerActor::OnRep_IsPowerRestored()
{
	UpdateVisualState();
}

void ALiminalBreakerActor::UpdateVisualState()
{
	if (IndicatorLight)
	{
		if (bIsPowerRestored)
		{
			IndicatorLight->SetLightColor(FLinearColor(0.1f, 1.0f, 0.2f));
			IndicatorLight->SetIntensity(350.0f);
		}
		else
		{
			IndicatorLight->SetLightColor(FLinearColor(1.0f, 0.1f, 0.05f));
			IndicatorLight->SetIntensity(150.0f);
		}
	}

	if (SwitchLeverMesh)
	{
		// Rotation du levier (vers le haut quand ON, vers le bas quand OFF)
		const FRotator LeverRot = bIsPowerRestored ? FRotator(35.0f, 0.0f, 0.0f) : FRotator(-35.0f, 0.0f, 0.0f);
		SwitchLeverMesh->SetRelativeRotation(LeverRot);
	}
}
