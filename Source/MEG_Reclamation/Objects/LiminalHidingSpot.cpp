#include "Objects/LiminalHidingSpot.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"

ALiminalHidingSpot::ALiminalHidingSpot()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	SpotMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpotMesh"));
	SpotMesh->SetupAttachment(Root);
	SpotMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> LockerFinder(
		TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"));
	if (LockerFinder.Succeeded())
	{
		SpotMesh->SetStaticMesh(LockerFinder.Object);
		SpotMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		SpotMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeFinder.Succeeded())
		{
			SpotMesh->SetStaticMesh(CubeFinder.Object);
			SpotMesh->SetRelativeScale3D(FVector(1.2f, 0.8f, 2.2f));
			SpotMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 110.0f));
		}
	}

	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
	InteractionZone->SetupAttachment(Root);
	InteractionZone->SetBoxExtent(FVector(100.0f, 100.0f, 100.0f));
	InteractionZone->SetRelativeLocation(FVector(80.0f, 0.0f, 80.0f));
	InteractionZone->SetCollisionProfileName(TEXT("OverlapAll"));
	InteractionZone->SetGenerateOverlapEvents(true);
}

void ALiminalHidingSpot::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalHidingSpot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalHidingSpot, OccupantNetId);
}

void ALiminalHidingSpot::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Animation d'entree/sortie par interpolation
	if (bIsTransitioning && OccupantCharacter.IsValid())
	{
		TransitionAlpha += DeltaSeconds / EnterExitDuration;
		TransitionAlpha = FMath::Clamp(TransitionAlpha, 0.0f, 1.0f);

		AScavengerCharacter* Scav = OccupantCharacter.Get();
		const float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, TransitionAlpha, 2.0f);

		if (bEntering)
		{
			const FVector TargetLoc = GetActorLocation() + HiddenCameraOffset;
			const FVector NewLoc = FMath::Lerp(SavedPlayerLocation, TargetLoc, SmoothAlpha);
			Scav->SetActorLocation(NewLoc);
		}
		else
		{
			const FVector CurrentLoc = GetActorLocation() + HiddenCameraOffset;
			const FVector NewLoc = FMath::Lerp(CurrentLoc, SavedPlayerLocation, SmoothAlpha);
			Scav->SetActorLocation(NewLoc);
		}

		if (TransitionAlpha >= 1.0f)
		{
			bIsTransitioning = false;

			if (!bEntering)
			{
				ExitHidingSpot();
			}
		}
	}

	// Rythme cardiaque quand cache
	if (OccupantCharacter.IsValid() && !bIsTransitioning)
	{
		UpdateHeartbeatAudio(DeltaSeconds);
	}
}

void ALiminalHidingSpot::TryEnter(AScavengerCharacter* Scavenger)
{
	if (!HasAuthority() || !Scavenger || bIsTransitioning)
	{
		return;
	}

	if (OccupantCharacter.IsValid())
	{
		// Deja occupe — le joueur actuel sort
		if (OccupantCharacter.Get() == Scavenger)
		{
			// Debut de l'animation de sortie
			bIsTransitioning = true;
			TransitionAlpha = 0.0f;
			bEntering = false;
		}
		// Sinon, quelqu'un d'autre est dedans — refuse
		return;
	}

	// Verification de l'etat du joueur
	if (Scavenger->IsDead() || Scavenger->IsDowned())
	{
		return;
	}

	// Larguer le loot porte avant d'entrer
	if (Scavenger->GetHeldLoot())
	{
		Scavenger->InputRelease();
	}

	EnterHidingSpot(Scavenger);
}

void ALiminalHidingSpot::ForceExit()
{
	if (OccupantCharacter.IsValid() && !bIsTransitioning)
	{
		bIsTransitioning = true;
		TransitionAlpha = 0.0f;
		bEntering = false;
	}
}

FTransform ALiminalHidingSpot::GetHiddenViewpoint() const
{
	return FTransform(HiddenCameraRotation, GetActorLocation() + HiddenCameraOffset);
}

void ALiminalHidingSpot::EnterHidingSpot(AScavengerCharacter* Scavenger)
{
	OccupantCharacter = Scavenger;
	OccupantNetId = Scavenger->GetUniqueID();

	// Sauvegarder la position/rotation du joueur pour la restauration a la sortie
	SavedPlayerLocation = Scavenger->GetActorLocation();
	SavedPlayerRotation = Scavenger->GetActorRotation();

	// Desactiver le mouvement et la collision du joueur
	if (UCharacterMovementComponent* Movement = Scavenger->GetCharacterMovement())
	{
		Movement->DisableMovement();
	}
	if (UCapsuleComponent* Capsule = Scavenger->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// Cacher le mesh 3eme personne (le joueur est invisible dans la cachette)
	Scavenger->SetActorHiddenInGame(true);

	// Demarrer l'animation d'entree
	bIsTransitioning = true;
	TransitionAlpha = 0.0f;
	bEntering = true;

	UE_LOG(LogTemp, Log, TEXT("HidingSpot: %s entered %s"), *Scavenger->GetName(), *GetName());
}

void ALiminalHidingSpot::ExitHidingSpot()
{
	if (!OccupantCharacter.IsValid())
	{
		return;
	}

	AScavengerCharacter* Scav = OccupantCharacter.Get();

	// Restaurer la position du joueur
	Scav->SetActorLocation(SavedPlayerLocation);
	Scav->SetActorRotation(SavedPlayerRotation);

	// Reactiver le mouvement et la collision
	if (UCharacterMovementComponent* Movement = Scav->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
	if (UCapsuleComponent* Capsule = Scav->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	// Rendre le joueur visible a nouveau
	Scav->SetActorHiddenInGame(false);

	UE_LOG(LogTemp, Log, TEXT("HidingSpot: %s exited %s"), *Scav->GetName(), *GetName());

	OccupantCharacter.Reset();
	OccupantNetId = 0;
}

void ALiminalHidingSpot::UpdateHeartbeatAudio(float DeltaSeconds)
{
	AScavengerCharacter* Scav = OccupantCharacter.Get();
	if (!Scav)
	{
		return;
	}

	// Le rythme cardiaque accelere avec :
	// - La basse sanite (peur)
	// - La proximite d'une entite hostile
	const float SanityPercent = Scav->GetSanityPercent();
	const float FearFactor = 1.0f - SanityPercent; // 0 = calme, 1 = terreur

	// Rythme cardiaque : 60 BPM au calme, 180 BPM en terreur
	HeartbeatRate = FMath::Lerp(1.0f, 3.0f, FearFactor);
	const float HeartbeatInterval = 1.0f / HeartbeatRate;

	HeartbeatTimer += DeltaSeconds;
	if (HeartbeatTimer >= HeartbeatInterval)
	{
		HeartbeatTimer -= HeartbeatInterval;

		// Volume proportionnel a la peur
		const float Volume = HeartbeatBaseVolume + (FearFactor * 0.7f);

		// Jouer le son de battement de coeur localement
		// Le bruit cardiaque est audible par l'IA a haute peur (> 0.7)
		if (FearFactor > 0.7f)
		{
			MakeNoise(FearFactor * 0.3f, Scav, GetActorLocation());
		}
	}
}

void ALiminalHidingSpot::OnRep_OccupantId()
{
	// Sur le client, trouver le joueur par son ID et mettre a jour la visibilite
	if (OccupantNetId == 0)
	{
		// Le joueur est sorti — le rendre visible
		// (Le serveur gerera la logique complete, le client ajuste juste le visuel)
	}
}

void ALiminalHidingSpot::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Pas d'interaction automatique — le joueur doit appuyer sur la touche d'interaction
}
