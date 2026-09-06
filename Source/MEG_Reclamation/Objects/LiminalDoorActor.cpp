#include "Objects/LiminalDoorActor.h"

#include "AI/LiminalEntity.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "Player/ScavengerCharacter.h"
#include "Sound/SoundBase.h"

ALiminalDoorActor::ALiminalDoorActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DoorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorRoot"));
	SetRootComponent(DoorRoot);

	DoorFrameMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorFrameMesh"));
	DoorFrameMesh->SetupAttachment(DoorRoot);
	DoorFrameMesh->SetCollisionProfileName(TEXT("BlockAll"));

	// Le pivot est decentre sur le bord de la porte (charniere)
	DoorPivot = CreateDefaultSubobject<USceneComponent>(TEXT("DoorPivot"));
	DoorPivot->SetupAttachment(DoorRoot);
	DoorPivot->SetRelativeLocation(FVector(-50.0f, 0.0f, 0.0f));

	DoorPanelMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DoorPanelMesh"));
	DoorPanelMesh->SetupAttachment(DoorPivot);
	DoorPanelMesh->SetRelativeLocation(FVector(50.0f, 0.0f, 0.0f));
	DoorPanelMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FrameFinder(
		TEXT("/Game/Meshes/Modular/SM_Door_Frame.SM_Door_Frame"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> LeafFinder(
		TEXT("/Game/Meshes/Modular/SM_Door_Leaf.SM_Door_Leaf"));

	if (FrameFinder.Succeeded() && LeafFinder.Succeeded())
	{
		DoorFrameMesh->SetStaticMesh(FrameFinder.Object);
		DoorFrameMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		DoorFrameMesh->SetRelativeLocation(FVector::ZeroVector);

		DoorPanelMesh->SetStaticMesh(LeafFinder.Object);
		DoorPanelMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		DoorPanelMesh->SetRelativeLocation(FVector::ZeroVector);
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Engine/BasicShapes/Cube.Cube"));
		if (CubeFinder.Succeeded())
		{
			DoorPanelMesh->SetStaticMesh(CubeFinder.Object);
			DoorPanelMesh->SetRelativeScale3D(FVector(1.0f, 0.06f, 2.4f));
			DoorFrameMesh->SetStaticMesh(CubeFinder.Object);
			DoorFrameMesh->SetRelativeScale3D(FVector(1.1f, 0.08f, 2.5f));
			DoorFrameMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 125.0f));
		}
	}

	// Zone d'interaction pour le joueur (rayon de detection)
	InteractionZone = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionZone"));
	InteractionZone->SetupAttachment(DoorRoot);
	InteractionZone->SetBoxExtent(FVector(120.0f, 120.0f, 150.0f));
	InteractionZone->SetCollisionProfileName(TEXT("OverlapAll"));
	InteractionZone->SetGenerateOverlapEvents(true);
}

void ALiminalDoorActor::BeginPlay()
{
	Super::BeginPlay();

	if (InteractionZone)
	{
		InteractionZone->OnComponentBeginOverlap.AddDynamic(this, &ALiminalDoorActor::OnInteractionOverlapBegin);
	}
}

void ALiminalDoorActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalDoorActor, CurrentState);
	DOREPLIFETIME(ALiminalDoorActor, CurrentAngle);
}

void ALiminalDoorActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateDoorSwing(DeltaSeconds);

	// Timer d'auto-fermeture
	if (HasAuthority() && CurrentState == EDoorState::Open && AutoCloseDelaySeconds > 0.0f)
	{
		AutoCloseTimer += DeltaSeconds;
		if (AutoCloseTimer >= AutoCloseDelaySeconds)
		{
			SetState(EDoorState::Closing, nullptr);
			AutoCloseTimer = 0.0f;
		}
	}

	// Cooldown AI break
	if (AIBreakCooldownTimer > 0.0f)
	{
		AIBreakCooldownTimer -= DeltaSeconds;
	}
}

void ALiminalDoorActor::Interact(AScavengerCharacter* InInstigator)
{
	if (!HasAuthority() || !InInstigator)
	{
		return;
	}

	switch (CurrentState)
	{
	case EDoorState::Closed:
	{
		// Determiner la direction d'ouverture basee sur la position du joueur
		const FVector ToPlayer = InInstigator->GetActorLocation() - GetActorLocation();
		const float Dot = FVector::DotProduct(GetActorForwardVector(), ToPlayer);
		OpenDirection = (Dot >= 0.0f) ? 1.0f : -1.0f;
		TargetAngle = MaxOpenAngleDegrees * OpenDirection;
		SetState(EDoorState::Opening, InInstigator);

		if (USoundBase* Sound = OpenSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
		EmitDoorNoise(CreakLoudness);
		break;
	}

	case EDoorState::Open:
	case EDoorState::Opening:
		TargetAngle = 0.0f;
		SetState(EDoorState::Closing, InInstigator);

		if (USoundBase* Sound = CloseSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
		EmitDoorNoise(SlamLoudness);
		break;

	case EDoorState::LockedClosed:
		if (!RequiredKeyTag.IsNone() && InInstigator->ActorHasTag(RequiredKeyTag))
		{
			Unlock();
			const FVector ToPlayer = InInstigator->GetActorLocation() - GetActorLocation();
			const float Dot = FVector::DotProduct(GetActorForwardVector(), ToPlayer);
			OpenDirection = (Dot >= 0.0f) ? 1.0f : -1.0f;
			TargetAngle = MaxOpenAngleDegrees * OpenDirection;
			SetState(EDoorState::Opening, InInstigator);

			if (USoundBase* Sound = OpenSound.LoadSynchronous())
			{
				UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
			}
			EmitDoorNoise(CreakLoudness);
			break;
		}

		if (USoundBase* Sound = LockedSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}
		// Le joueur secoue la poignee — petit bruit qui attire
		EmitDoorNoise(CreakLoudness * 0.5f);
		break;

	case EDoorState::Closing:
		// Arret et reouverture
		OpenDirection = (CurrentAngle >= 0.0f) ? 1.0f : -1.0f;
		TargetAngle = MaxOpenAngleDegrees * OpenDirection;
		SetState(EDoorState::Opening, InInstigator);
		break;

	default:
		break;
	}
}

void ALiminalDoorActor::AIBreakDoor(ALiminalEntity* Entity)
{
	if (!HasAuthority() || !Entity || CurrentState == EDoorState::Broken || CurrentState == EDoorState::Open)
	{
		return;
	}

	if (AIBreakCooldownTimer > 0.0f)
	{
		return;
	}

	AIBreakProgress += 1.0f;
	AIBreakCooldownTimer = AIBreakCooldown;

	// Impact sonore — le monstre frappe la porte
	EmitDoorNoise(BreakLoudness);

	// Vibration visuelle — petite secousse de la porte a chaque coup
	if (DoorPanelMesh)
	{
		const float ShakeAmount = 3.0f;
		DoorPanelMesh->AddLocalOffset(FVector(
			FMath::RandRange(-ShakeAmount, ShakeAmount),
			FMath::RandRange(-ShakeAmount, ShakeAmount),
			0.0f));
	}

	if (AIBreakProgress >= AIBreakHitsRequired)
	{
		// La porte vole en eclats
		SetState(EDoorState::Broken, Entity);

		if (USoundBase* Sound = BreakSound.LoadSynchronous())
		{
			UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation());
		}

		// Desactiver la collision du panneau pour que les entites/joueurs passent
		if (DoorPanelMesh)
		{
			DoorPanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			// Propulser le panneau
			DoorPanelMesh->SetSimulatePhysics(true);
			DoorPanelMesh->AddImpulse(Entity->GetActorForwardVector() * 50000.0f);
		}
	}
}

void ALiminalDoorActor::AIOpenDoor(ALiminalEntity* Entity)
{
	if (!HasAuthority() || !Entity)
	{
		return;
	}

	if (CurrentState == EDoorState::Closed)
	{
		const FVector ToEntity = Entity->GetActorLocation() - GetActorLocation();
		const float Dot = FVector::DotProduct(GetActorForwardVector(), ToEntity);
		OpenDirection = (Dot >= 0.0f) ? 1.0f : -1.0f;
		TargetAngle = MaxOpenAngleDegrees * OpenDirection;
		SetState(EDoorState::Opening, Entity);
		EmitDoorNoise(CreakLoudness);
	}
	else if (CurrentState == EDoorState::LockedClosed)
	{
		// L'entite ne peut pas ouvrir une porte verrouillee normalement — elle doit la casser
		AIBreakDoor(Entity);
	}
}

void ALiminalDoorActor::Lock()
{
	if (HasAuthority() && CurrentState == EDoorState::Closed)
	{
		SetState(EDoorState::LockedClosed, nullptr);
	}
}

void ALiminalDoorActor::Unlock()
{
	if (HasAuthority() && CurrentState == EDoorState::LockedClosed)
	{
		SetState(EDoorState::Closed, nullptr);
	}
}

void ALiminalDoorActor::SetState(EDoorState NewState, AActor* InInstigator)
{
	if (CurrentState == NewState)
	{
		return;
	}

	CurrentState = NewState;
	OnDoorStateChanged.Broadcast(NewState, InInstigator);

	if (NewState == EDoorState::Opening)
	{
		AutoCloseTimer = 0.0f;
	}
}

void ALiminalDoorActor::UpdateDoorSwing(float DeltaSeconds)
{
	if (CurrentState == EDoorState::Broken)
	{
		return;
	}

	const float PrevAngle = CurrentAngle;
	bool bReachedTarget = false;

	if (CurrentState == EDoorState::Opening)
	{
		const float Direction = FMath::Sign(TargetAngle - CurrentAngle);
		CurrentAngle += Direction * OpenSpeed * DeltaSeconds;

		if ((Direction > 0.0f && CurrentAngle >= TargetAngle) ||
			(Direction < 0.0f && CurrentAngle <= TargetAngle))
		{
			CurrentAngle = TargetAngle;
			bReachedTarget = true;
		}
	}
	else if (CurrentState == EDoorState::Closing)
	{
		const float Direction = FMath::Sign(0.0f - CurrentAngle);
		CurrentAngle += Direction * CloseSpeed * DeltaSeconds;

		if (FMath::Abs(CurrentAngle) < 1.0f)
		{
			CurrentAngle = 0.0f;
			bReachedTarget = true;
		}
	}

	// Appliquer la rotation au pivot de la porte
	if (DoorPivot && FMath::Abs(CurrentAngle - PrevAngle) > 0.01f)
	{
		DoorPivot->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));

		// Grincement continu pendant le mouvement
		if (CreakLoudness > 0.0f && FMath::Abs(CurrentAngle - PrevAngle) > 2.0f)
		{
			const float SwingSpeed = FMath::Abs(CurrentAngle - PrevAngle) / DeltaSeconds;
			const float NormalizedSpeed = FMath::Clamp(SwingSpeed / OpenSpeed, 0.0f, 1.0f);
			
			if (NormalizedSpeed > 0.3f)
			{
				if (USoundBase* Sound = CreakSound.LoadSynchronous())
				{
					UGameplayStatics::PlaySoundAtLocation(this, Sound, GetActorLocation(),
						NormalizedSpeed * CreakLoudness);
				}
			}
		}
	}

	// Transition d'etat quand la cible est atteinte
	if (bReachedTarget && HasAuthority())
	{
		if (CurrentState == EDoorState::Opening)
		{
			SetState(EDoorState::Open, nullptr);
		}
		else if (CurrentState == EDoorState::Closing)
		{
			SetState(EDoorState::Closed, nullptr);

			// Claquement a la fermeture
			EmitDoorNoise(SlamLoudness * 0.5f);
		}
	}
}

void ALiminalDoorActor::EmitDoorNoise(float Loudness)
{
	if (Loudness > 0.0f)
	{
		MakeNoise(Loudness, nullptr, GetActorLocation());
	}
}

void ALiminalDoorActor::OnRep_DoorState()
{
	// Sur le client, ajuster visuellement selon l'etat
	if (CurrentState == EDoorState::Broken && DoorPanelMesh)
	{
		DoorPanelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void ALiminalDoorActor::OnRep_DoorAngle()
{
	if (DoorPivot)
	{
		DoorPivot->SetRelativeRotation(FRotator(0.0f, CurrentAngle, 0.0f));
	}
}

void ALiminalDoorActor::OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Les entites IA qui entrent dans la zone d'interaction tentent d'ouvrir la porte
	if (HasAuthority())
	{
		if (ALiminalEntity* Entity = Cast<ALiminalEntity>(OtherActor))
		{
			if (CurrentState == EDoorState::Closed)
			{
				AIOpenDoor(Entity);
			}
			else if (CurrentState == EDoorState::LockedClosed)
			{
				AIBreakDoor(Entity);
			}
		}
	}
}
