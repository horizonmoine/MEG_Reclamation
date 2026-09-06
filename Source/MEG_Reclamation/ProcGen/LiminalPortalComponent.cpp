#include "ProcGen/LiminalPortalComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ULiminalPortalComponent::ULiminalPortalComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}

void ULiminalPortalComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bEnableSceneCapture && !PortalRenderTarget)
	{
		PortalRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("PortalRenderTarget"));
		if (PortalRenderTarget)
		{
			PortalRenderTarget->InitAutoFormat(RenderTargetWidth, RenderTargetHeight);
			PortalRenderTarget->UpdateResourceImmediate(true);
		}

		PortalCapture = NewObject<USceneCaptureComponent2D>(GetOwner(), TEXT("PortalCapture"));
		if (PortalCapture)
		{
			PortalCapture->RegisterComponent();
			PortalCapture->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
			PortalCapture->TextureTarget = PortalRenderTarget;
			PortalCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorHDR;
			PortalCapture->bCaptureEveryFrame = true;
			PortalCapture->bCaptureOnMovement = false;
			PortalCapture->FOVAngle = 90.0f;
		}
	}
}

void ULiminalPortalComponent::LinkTargetPortal(ULiminalPortalComponent* InTargetPortal)
{
	TargetPortal = InTargetPortal;
	if (TargetPortal && !TargetPortal->TargetPortal)
	{
		TargetPortal->LinkTargetPortal(this);
	}
}

void ULiminalPortalComponent::SetDisplayMesh(UStaticMeshComponent* InMesh)
{
	DisplayMeshComponent = InMesh;
	if (DisplayMeshComponent && PortalRenderTarget)
	{
		UMaterialInterface* BaseMat = DisplayMeshComponent->GetMaterial(0);
		if (BaseMat)
		{
			PortalMaterialInstance = DisplayMeshComponent->CreateDynamicMaterialInstance(0, BaseMat);
			if (PortalMaterialInstance)
			{
				PortalMaterialInstance->SetTextureParameterValue(FName(TEXT("PortalTexture")), PortalRenderTarget);
			}
		}
	}
}

void ULiminalPortalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!TargetPortal)
	{
		return;
	}

	UpdateCaptureCamera();

	// Verification du passage des acteurs a portee
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	APlayerController* PC = World->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		APawn* PlayerPawn = PC->GetPawn();
		const FVector CurrentLoc = PlayerPawn->GetActorLocation();

		if (TrackedActorPositions.Contains(PlayerPawn))
		{
			const FVector PreviousLoc = TrackedActorPositions[PlayerPawn];
			if (CheckActorCrossing(PlayerPawn, PreviousLoc, CurrentLoc))
			{
				TeleportActor(PlayerPawn);
			}
		}

		TrackedActorPositions.Add(PlayerPawn, PlayerPawn->GetActorLocation());
	}
}

void ULiminalPortalComponent::UpdateCaptureCamera()
{
	if (!PortalCapture || !TargetPortal)
	{
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}

	const FVector CamLocation = PC->PlayerCameraManager->GetCameraLocation();
	const FRotator CamRotation = PC->PlayerCameraManager->GetCameraRotation();

	// Calcul de la transformation de la camera relative a ce portail
	const FTransform SourcePortalTransform = GetComponentTransform();
	const FTransform TargetPortalTransform = TargetPortal->GetComponentTransform();

	const FTransform RelativeCamTransform = FTransform(CamRotation, CamLocation).GetRelativeTransform(SourcePortalTransform);

	// Rotation de 180 degres sur Yaw pour compenser l'inversion du plan de sortie
	const FQuat RotFlip(FVector::UpVector, FMath::DegreesToRadians(180.0f));
	const FTransform FlippedRelativeTransform = RelativeCamTransform * FTransform(RotFlip);

	const FTransform DestCaptureTransform = FlippedRelativeTransform * TargetPortalTransform;

	PortalCapture->SetWorldLocationAndRotation(DestCaptureTransform.GetLocation(), DestCaptureTransform.GetRotation());
	PortalCapture->FOVAngle = PC->PlayerCameraManager->GetFOVAngle();
}

bool ULiminalPortalComponent::CheckActorCrossing(AActor* InActor, const FVector& PreviousLocation, const FVector& CurrentLocation)
{
	if (!InActor)
	{
		return false;
	}

	const FVector PortalLocation = GetComponentLocation();
	const FVector PortalForward = GetForwardVector();

	// Verifier la proximite
	if (FVector::DistSquared(CurrentLocation, PortalLocation) > FMath::Square(TeleportThresholdDistance))
	{
		return false;
	}

	const FVector PrevOffset = PreviousLocation - PortalLocation;
	const FVector CurrOffset = CurrentLocation - PortalLocation;

	const float PrevDot = FVector::DotProduct(PrevOffset, PortalForward);
	const float CurrDot = FVector::DotProduct(CurrOffset, PortalForward);

	// L'acteur traverse le plan d'avant en arriere
	return (PrevDot > 0.0f && CurrDot <= 0.0f);
}

bool ULiminalPortalComponent::TeleportActor(AActor* InActor)
{
	if (!InActor || !TargetPortal)
	{
		return false;
	}

	const FTransform SourceTransform = GetComponentTransform();
	const FTransform TargetTransform = TargetPortal->GetComponentTransform();

	const FTransform ActorWorldTransform = InActor->GetActorTransform();
	const FTransform ActorLocalTransform = ActorWorldTransform.GetRelativeTransform(SourceTransform);

	const FQuat RotFlip(FVector::UpVector, FMath::DegreesToRadians(180.0f));
	const FTransform DestActorTransform = (ActorLocalTransform * FTransform(RotFlip)) * TargetTransform;

	const FVector NewLocation = DestActorTransform.GetLocation();
	const FRotator NewRotation = DestActorTransform.Rotator();

	// Mise a jour de la velocite du mouvement
	if (ACharacter* Char = Cast<ACharacter>(InActor))
	{
		if (UCharacterMovementComponent* MoveComp = Char->GetCharacterMovement())
		{
			const FVector LocalVel = SourceTransform.InverseTransformVector(MoveComp->Velocity);
			const FVector RotatedVel = RotFlip.RotateVector(LocalVel);
			MoveComp->Velocity = TargetTransform.TransformVector(RotatedVel);
		}

		if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
		{
			const FRotator ControlRot = PC->GetControlRotation();
			const FTransform RotTransform(ControlRot, FVector::ZeroVector);
			const FTransform LocalRot = RotTransform.GetRelativeTransform(SourceTransform);
			const FTransform DestRot = (LocalRot * FTransform(RotFlip)) * TargetTransform;
			PC->SetControlRotation(DestRot.Rotator());
		}
	}

	InActor->TeleportTo(NewLocation, NewRotation, false, true);

	// Mettre a jour la position traquee pour eviter un aller-retour immediat
	TrackedActorPositions.Add(InActor, NewLocation);

	OnPortalTraversed.Broadcast(InActor, this);
	return true;
}

ALiminalPortalActor::ALiminalPortalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PortalComponent = CreateDefaultSubobject<ULiminalPortalComponent>(TEXT("PortalComponent"));
	SetRootComponent(PortalComponent);

	DisplayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DisplayMesh"));
	DisplayMesh->SetupAttachment(PortalComponent);
	DisplayMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	DisplayMesh->SetRelativeRotation(FRotator(0.0f, 90.0f, 0.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneFinder(
		TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (PlaneFinder.Succeeded())
	{
		DisplayMesh->SetStaticMesh(PlaneFinder.Object);
		DisplayMesh->SetRelativeScale3D(FVector(1.0f, 1.5f, 2.5f));
	}

	PortalComponent->SetDisplayMesh(DisplayMesh);
}

