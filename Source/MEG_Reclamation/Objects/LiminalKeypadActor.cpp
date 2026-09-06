#include "Objects/LiminalKeypadActor.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "UObject/ConstructorHelpers.h"

ALiminalKeypadActor::ALiminalKeypadActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	SetRootComponent(RootScene);

	KeypadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("KeypadMesh"));
	KeypadMesh->SetupAttachment(RootScene);
	KeypadMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> KeypadFinder(
		TEXT("/Game/Meshes/Puzzles/SM_Keypad.SM_Keypad"));
	if (KeypadFinder.Succeeded())
	{
		KeypadMesh->SetStaticMesh(KeypadFinder.Object);
		KeypadMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(
			TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeMeshFinder.Succeeded())
		{
			KeypadMesh->SetStaticMesh(CubeMeshFinder.Object);
			KeypadMesh->SetRelativeScale3D(FVector(0.1f, 0.35f, 0.45f));
		}
	}

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	InteractionBox->SetupAttachment(RootScene);
	InteractionBox->SetBoxExtent(FVector(60.0f, 60.0f, 60.0f));
	InteractionBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	StatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StatusLight"));
	StatusLight->SetupAttachment(RootScene);
	StatusLight->SetRelativeLocation(FVector(15.0f, 0.0f, 20.0f));
	StatusLight->SetLightColor(FLinearColor(1.0f, 0.05f, 0.05f));
	StatusLight->SetIntensity(250.0f);
	StatusLight->SetAttenuationRadius(200.0f);
	StatusLight->SetCastShadows(false);

	DisplayText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DisplayText"));
	DisplayText->SetupAttachment(RootScene);
	DisplayText->SetRelativeLocation(FVector(8.0f, 0.0f, 10.0f));
	DisplayText->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	DisplayText->SetText(FText::FromString(TEXT("----")));
	DisplayText->SetTextRenderColor(FColor(255, 60, 60));
	DisplayText->SetWorldSize(12.0f);
	DisplayText->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	DisplayText->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
}

void ALiminalKeypadActor::BeginPlay()
{
	Super::BeginPlay();
	UpdateVisualState();
}

void ALiminalKeypadActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (ErrorFlashTimer > 0.0f)
	{
		ErrorFlashTimer -= DeltaSeconds;
		if (StatusLight)
		{
			const bool bBlink = FMath::Fmod(ErrorFlashTimer * 8.0f, 1.0f) < 0.5f;
			StatusLight->SetIntensity(bBlink ? 500.0f : 20.0f);
		}
		if (ErrorFlashTimer <= 0.0f)
		{
			UpdateVisualState();
		}
	}
}

void ALiminalKeypadActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalKeypadActor, CurrentInput);
	DOREPLIFETIME(ALiminalKeypadActor, bIsUnlocked);
}

bool ALiminalKeypadActor::EnterDigit(int32 Digit)
{
	if (bIsUnlocked || Digit < 0 || Digit > 9)
	{
		return false;
	}

	if (CurrentInput.Len() >= MaxDigits)
	{
		return false;
	}

	CurrentInput.Append(FString::FromInt(Digit));
	UpdateVisualState();

	// Bip diegetique
	UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.2f, this);

	// Verification automatique lorsque le nombre maximal de chiffres est saisi
	if (CurrentInput.Len() >= MaxDigits)
	{
		return SubmitCode();
	}

	return true;
}

void ALiminalKeypadActor::ClearInput()
{
	if (bIsUnlocked)
	{
		return;
	}

	CurrentInput.Empty();
	UpdateVisualState();
}

bool ALiminalKeypadActor::SubmitCode()
{
	if (bIsUnlocked)
	{
		return true;
	}

	if (CurrentInput.Equals(TargetCode, ESearchCase::IgnoreCase))
	{
		bIsUnlocked = true;
		UpdateVisualState();
		OnKeypadUnlocked.Broadcast(this);

		// Signal sonore de deverrouillage
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.5f, this);

		if (LinkedTargetActor.IsValid())
		{
			LinkedTargetActor->SetActorEnableCollision(false);
			LinkedTargetActor->SetActorHiddenInGame(true);
		}

		return true;
	}
	else
	{
		ErrorFlashTimer = 1.2f;
		CurrentInput.Empty();
		UpdateVisualState();
		OnKeypadCodeFailed.Broadcast(this);

		// Bip d'erreur audible
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 0.8f, this);
		return false;
	}
}

FString ALiminalKeypadActor::GetDisplayString() const
{
	if (bIsUnlocked)
	{
		return TEXT("OPEN");
	}

	if (ErrorFlashTimer > 0.0f)
	{
		return TEXT("ERR!");
	}

	FString Display;
	for (int32 i = 0; i < MaxDigits; ++i)
	{
		if (i < CurrentInput.Len())
		{
			Display.AppendChar('*');
		}
		else
		{
			Display.AppendChar('-');
		}
	}
	return Display;
}

void ALiminalKeypadActor::OnRep_CurrentInput()
{
	UpdateVisualState();
}

void ALiminalKeypadActor::OnRep_IsUnlocked()
{
	UpdateVisualState();
}

void ALiminalKeypadActor::UpdateVisualState()
{
	const FString Disp = GetDisplayString();

	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(Disp));
		DisplayText->SetTextRenderColor(bIsUnlocked ? FColor(40, 255, 60) : FColor(255, 60, 60));
	}

	if (StatusLight)
	{
		if (bIsUnlocked)
		{
			StatusLight->SetLightColor(FLinearColor(0.1f, 1.0f, 0.2f));
			StatusLight->SetIntensity(300.0f);
		}
		else if (ErrorFlashTimer > 0.0f)
		{
			StatusLight->SetLightColor(FLinearColor(1.0f, 0.0f, 0.0f));
			StatusLight->SetIntensity(500.0f);
		}
		else
		{
			StatusLight->SetLightColor(FLinearColor(1.0f, 0.1f, 0.1f));
			StatusLight->SetIntensity(150.0f);
		}
	}
}
