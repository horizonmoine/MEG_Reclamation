#include "Objects/LiminalVentActor.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "Player/ScavengerCharacter.h"

ALiminalVentActor::ALiminalVentActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	VentRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VentRoot"));
	SetRootComponent(VentRoot);

	VentMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VentMesh"));
	VentMesh->SetupAttachment(VentRoot);
	VentMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> VentFinder(
		TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"));
	if (VentFinder.Succeeded())
	{
		VentMesh->SetStaticMesh(VentFinder.Object);
		VentMesh->SetRelativeScale3D(FVector(2.0f, 1.0f, 1.0f));
		VentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (FallbackFinder.Succeeded())
		{
			VentMesh->SetStaticMesh(FallbackFinder.Object);
			VentMesh->SetRelativeScale3D(FVector(4.0f, 0.6f, 0.6f));
			VentMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));
		}
	}

	GrilleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrilleMesh"));
	GrilleMesh->SetupAttachment(VentRoot);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GrilleCubeFinder(
		TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
	if (GrilleCubeFinder.Succeeded())
	{
		GrilleMesh->SetStaticMesh(GrilleCubeFinder.Object);
		GrilleMesh->SetRelativeScale3D(FVector(0.02f, 0.55f, 0.55f));
		GrilleMesh->SetRelativeLocation(FVector(-200.0f, 0.0f, 30.0f));
	}
	GrilleMesh->SetCollisionProfileName(TEXT("BlockAll"));

	CrawlPath = CreateDefaultSubobject<USplineComponent>(TEXT("CrawlPath"));
	CrawlPath->SetupAttachment(VentRoot);
	CrawlPath->SetRelativeLocation(FVector(0.0f, 0.0f, 30.0f));

	EntryZoneA = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryZoneA"));
	EntryZoneA->SetupAttachment(VentRoot);
	EntryZoneA->SetBoxExtent(FVector(60.0f, 60.0f, 80.0f));
	EntryZoneA->SetRelativeLocation(FVector(-250.0f, 0.0f, 50.0f));
	EntryZoneA->SetCollisionProfileName(TEXT("OverlapAll"));
	EntryZoneA->SetGenerateOverlapEvents(true);

	EntryZoneB = CreateDefaultSubobject<UBoxComponent>(TEXT("EntryZoneB"));
	EntryZoneB->SetupAttachment(VentRoot);
	EntryZoneB->SetBoxExtent(FVector(60.0f, 60.0f, 80.0f));
	EntryZoneB->SetRelativeLocation(FVector(250.0f, 0.0f, 50.0f));
	EntryZoneB->SetCollisionProfileName(TEXT("OverlapAll"));
	EntryZoneB->SetGenerateOverlapEvents(true);
}

void ALiminalVentActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalVentActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALiminalVentActor, CurrentState);
}

void ALiminalVentActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentState == EVentState::Occupied && OccupantCharacter.IsValid())
	{
		UpdateCrawlProgress(DeltaSeconds);
	}
}

void ALiminalVentActor::TryEnter(AScavengerCharacter* Scavenger)
{
	if (!HasAuthority() || !Scavenger || CurrentState == EVentState::Occupied)
	{
		return;
	}

	if (Scavenger->IsDead() || Scavenger->IsDowned())
	{
		return;
	}

	// Larguer le loot porte
	if (Scavenger->GetHeldLoot())
	{
		Scavenger->InputRelease();
	}

	// Ouvrir la grille si scellee
	if (CurrentState == EVentState::Sealed)
	{
		CurrentState = EVentState::Open;

		// La grille tombe — physics
		if (GrilleMesh)
		{
			GrilleMesh->SetSimulatePhysics(true);
			GrilleMesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
			GrilleMesh->AddImpulse(FVector(0.0f, 0.0f, -500.0f));
		}

		// Bruit de grille arrachee
		MakeNoise(1.0f, Scavenger, GetActorLocation());
	}

	// Determiner le point d'entree le plus proche
	const float DistToA = FVector::Dist(Scavenger->GetActorLocation(), EntryZoneA->GetComponentLocation());
	const float DistToB = FVector::Dist(Scavenger->GetActorLocation(), EntryZoneB->GetComponentLocation());
	bCrawlForward = (DistToA <= DistToB);
	CrawlProgress = bCrawlForward ? 0.0f : 1.0f;

	// Sauvegarder la position du joueur
	SavedPlayerLocation = Scavenger->GetActorLocation();
	SavedPlayerRotation = Scavenger->GetActorRotation();

	// Desactiver le mouvement normal
	if (UCharacterMovementComponent* Movement = Scavenger->GetCharacterMovement())
	{
		Movement->DisableMovement();
	}
	if (UCapsuleComponent* Capsule = Scavenger->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	OccupantCharacter = Scavenger;
	CurrentState = EVentState::Occupied;
	CrawlNoiseTimer = 0.0f;

	UE_LOG(LogTemp, Log, TEXT("Vent: %s entered (direction: %s)"),
		*Scavenger->GetName(), bCrawlForward ? TEXT("A->B") : TEXT("B->A"));
}

void ALiminalVentActor::ExitVent()
{
	if (!OccupantCharacter.IsValid())
	{
		return;
	}

	AScavengerCharacter* Scav = OccupantCharacter.Get();

	// Placer le joueur a la sortie du conduit
	const FVector ExitPoint = bCrawlForward
		? EntryZoneB->GetComponentLocation() + FVector(80.0f, 0.0f, 0.0f)
		: EntryZoneA->GetComponentLocation() + FVector(-80.0f, 0.0f, 0.0f);

	Scav->SetActorLocation(ExitPoint);

	// Reactiver le mouvement et la collision
	if (UCharacterMovementComponent* Movement = Scav->GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}
	if (UCapsuleComponent* Capsule = Scav->GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}

	OccupantCharacter.Reset();
	CurrentState = EVentState::Open;

	UE_LOG(LogTemp, Log, TEXT("Vent: Player exited."));
}

void ALiminalVentActor::UpdateCrawlProgress(float DeltaSeconds)
{
	if (!CrawlPath)
	{
		return;
	}

	AScavengerCharacter* Scav = OccupantCharacter.Get();
	if (!Scav)
	{
		return;
	}

	const float SplineLength = CrawlPath->GetSplineLength();
	if (SplineLength <= 0.0f)
	{
		return;
	}

	// Avancer le long du spline
	const float ProgressDelta = (CrawlSpeed * DeltaSeconds) / SplineLength;
	if (bCrawlForward)
	{
		CrawlProgress += ProgressDelta;
	}
	else
	{
		CrawlProgress -= ProgressDelta;
	}

	CrawlProgress = FMath::Clamp(CrawlProgress, 0.0f, 1.0f);

	// Positionner le joueur sur le spline
	const float DistAlongSpline = CrawlProgress * SplineLength;
	const FVector SplineLocation = CrawlPath->GetLocationAtDistanceAlongSpline(DistAlongSpline, ESplineCoordinateSpace::World);
	const FRotator SplineRotation = CrawlPath->GetRotationAtDistanceAlongSpline(DistAlongSpline, ESplineCoordinateSpace::World);

	Scav->SetActorLocation(SplineLocation);
	Scav->SetActorRotation(SplineRotation);

	// Bruits metalliques periodiques
	CrawlNoiseTimer += DeltaSeconds;
	if (CrawlNoiseTimer >= CrawlNoiseInterval)
	{
		CrawlNoiseTimer -= CrawlNoiseInterval;
		EmitCrawlNoise();
	}

	// Verifier si le joueur est arrive a la sortie
	const bool bReachedEnd = bCrawlForward ? (CrawlProgress >= 1.0f) : (CrawlProgress <= 0.0f);
	if (bReachedEnd)
	{
		ExitVent();
	}
}

void ALiminalVentActor::EmitCrawlNoise()
{
	if (OccupantCharacter.IsValid())
	{
		MakeNoise(CrawlNoiseLoudness, OccupantCharacter.Get(), GetActorLocation());
	}
}

void ALiminalVentActor::OnRep_VentState()
{
	// Ajustement visuel cote client
	if (CurrentState != EVentState::Sealed && GrilleMesh)
	{
		GrilleMesh->SetVisibility(false);
	}
}
