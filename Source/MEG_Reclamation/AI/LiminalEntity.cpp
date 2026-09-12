#include "AI/LiminalEntity.h"

#include "AI/LiminalAIController.h"
#include "Animation/AnimMontage.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISense_Hearing.h"
#include "Player/ScavengerCharacter.h"
#include "Objects/LootActor.h"
#include "GameModes/LiminalZoneRulesSubsystem.h"
#include "UObject/ConstructorHelpers.h"

ALiminalEntity::ALiminalEntity()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	AIControllerClass = ALiminalAIController::StaticClass();

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));

	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 2500.0f;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*HearingConfig);
	AIPerception->SetDominantSense(UAISense_Hearing::StaticClass());

	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DefaultBodyMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Hound/SM_Hound.SM_Hound")));
	DefaultSkeletalMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT("/Game/Characters/Bestiary/Hound/SK_Hound.SK_Hound")));
	BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -15.0f));
	BodyMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	BodyMesh->SetRelativeScale3D(FVector(0.9f));

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetCollisionProfileName(TEXT("CharacterMesh"));
		SkelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SkelMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
		SkelMesh->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f));
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> BiteFinder(
		TEXT("/Game/Audio/S_Hound_Bite.S_Hound_Bite"));
	if (BiteFinder.Succeeded())
	{
		AttackSound = BiteFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<USoundBase> SnarlFinder(
		TEXT("/Game/Audio/S_Hound_Snarl.S_Hound_Snarl"));
	if (SnarlFinder.Succeeded())
	{
		AggroSound = SnarlFinder.Object;
	}
}

EMonsterType ALiminalEntity::GetMonsterType() const
{
	return MonsterType;
}

UAIPerceptionComponent* ALiminalEntity::GetPerceptionComponent() const
{
	return AIPerception;
}

UBehaviorTree* ALiminalEntity::GetInitialBehaviorTree() const
{
	return InitialBehaviorTree;
}

void ALiminalEntity::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (UWorld* World = GetWorld())
		{
			if (ULiminalZoneRulesSubsystem* Rules = World->GetSubsystem<ULiminalZoneRulesSubsystem>())
			{
				if (!Rules->IsEntitySpawnAllowedAt(GetActorLocation()))
				{
					UE_LOG(LogTemp, Warning, TEXT("[LiminalEntity] Spawn refuse en zone protegee / non hostile : %s a %s. Destruction immediate."),
						*GetName(), *GetActorLocation().ToString());
					Destroy();
					return;
				}
			}
		}
	}
	
	if (DefaultBodyMesh.IsPending())
	{
		DefaultBodyMesh.LoadSynchronous();
	}
	if (BodyMesh && DefaultBodyMesh.IsValid())
	{
		BodyMesh->SetStaticMesh(DefaultBodyMesh.Get());
	}

	if (DefaultSkeletalMesh.IsPending())
	{
		DefaultSkeletalMesh.LoadSynchronous();
	}

	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		if (DefaultSkeletalMesh.IsValid())
		{
			SkelMesh->SetSkeletalMesh(DefaultSkeletalMesh.Get());
			if (DefaultAnimClass)
			{
				SkelMesh->SetAnimInstanceClass(DefaultAnimClass);
			}
			SkelMesh->SetVisibility(true);

			if (BodyMesh)
			{
				BodyMesh->SetVisibility(false);
			}
		}
	}
}

void ALiminalEntity::SetEntityVisualScale(const FVector& Scale3D)
{
	if (BodyMesh)
	{
		BodyMesh->SetRelativeScale3D(Scale3D);
	}
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		SkelMesh->SetRelativeScale3D(Scale3D);
	}
}

void ALiminalEntity::SetEntityVisualVisibility(bool bVisible)
{
	if (USkeletalMeshComponent* SkelMesh = GetMesh())
	{
		if (DefaultSkeletalMesh.IsValid() || SkelMesh->GetSkeletalMeshAsset())
		{
			SkelMesh->SetVisibility(bVisible, true);
			if (BodyMesh)
			{
				BodyMesh->SetVisibility(false);
			}
			return;
		}
	}

	if (BodyMesh)
	{
		BodyMesh->SetVisibility(bVisible, true);
	}
}

void ALiminalEntity::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority())
	{
		return;
	}

	if (StunTimer > 0.0f)
	{
		StunTimer -= DeltaSeconds;
		if (StunTimer <= 0.0f)
		{
			StunTimer = 0.0f;
			if (UCharacterMovementComponent* Movement = GetCharacterMovement())
			{
				Movement->MaxWalkSpeed = OriginalWalkSpeed;
			}
		}
	}

	if (CalmTimer > 0.0f)
	{
		CalmTimer = FMath::Max(CalmTimer - DeltaSeconds, 0.0f);
	}

	if (AttackCooldownTimer > 0.0f)
	{
		AttackCooldownTimer = FMath::Max(AttackCooldownTimer - DeltaSeconds, 0.0f);
		if (AttackCooldownTimer <= 0.0f)
		{
			bAttackImpactPending = false;
		}
	}
}

void ALiminalEntity::ApplyStun(float DurationSeconds)
{
	if (DurationSeconds <= 0.0f)
	{
		return;
	}

	if (StunTimer <= 0.0f)
	{
		if (UCharacterMovementComponent* Movement = GetCharacterMovement())
		{
			OriginalWalkSpeed = Movement->MaxWalkSpeed;
			Movement->StopMovementImmediately();
			Movement->MaxWalkSpeed = 0.0f;
		}
	}

	StunTimer = FMath::Max(StunTimer, DurationSeconds);
	bAttackImpactPending = false;
}

void ALiminalEntity::ApplyCalm(float DurationSeconds)
{
	if (DurationSeconds <= 0.0f)
	{
		return;
	}

	CalmTimer = FMath::Max(CalmTimer, DurationSeconds);
	bAttackImpactPending = false;
}

bool ALiminalEntity::IsStunned() const
{
	return StunTimer > 0.0f;
}

bool ALiminalEntity::IsCalmed() const
{
	return CalmTimer > 0.0f;
}

bool ALiminalEntity::CanAttack() const
{
	return !IsStunned() && !IsCalmed() && AttackCooldownTimer <= 0.0f && CurrentHealth > 0.0f;
}

bool ALiminalEntity::PerformMeleeAttack(AActor* Target)
{
	const AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(Target);
	UWorld* World = GetWorld();
	if (!CanAttack() || !IsValid(Scavenger) || !HasAuthority() || !World ||
		Scavenger->IsDead() || Scavenger->IsDowned() || Scavenger->IsHiddenInSpot())
	{
		return false;
	}

	const float DistSq = FVector::DistSquared(GetActorLocation(), Target->GetActorLocation());
	if (DistSq > FMath::Square(AttackRange + 50.0f))
	{
		return false;
	}

	FCollisionQueryParams VisibilityParams(SCENE_QUERY_STAT(LiminalMeleeVisibility), false, this);
	FHitResult VisibilityHit;
	if (World->LineTraceSingleByChannel(VisibilityHit, GetActorLocation(), Target->GetActorLocation(),
		ECC_Visibility, VisibilityParams) && VisibilityHit.GetActor() != Target)
	{
		return false;
	}

	AttackCooldownTimer = AttackCooldownSeconds;
	bAttackImpactPending = true;

	bool bMontagePlayed = false;
	if (AttackMontage)
	{
		const float MontageDuration = PlayAnimMontage(AttackMontage);
		if (MontageDuration > 0.0f)
		{
			bMontagePlayed = true;
			// A late notify from this montage must not consume a subsequent attack.
			AttackCooldownTimer = FMath::Max(AttackCooldownTimer, MontageDuration);
		}
	}

	// In headless (-nullrhi) tests or when no montage is configured, trigger attack notify trace immediately as fallback
	if (!bMontagePlayed)
	{
		OnAttackNotify(DefaultAttackSocket, DefaultAttackTraceRadius, AttackRange);

		UAISense_Hearing::ReportNoiseEvent(GetWorld(), GetActorLocation(), 1.0f, this);

		if (AttackSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), AttackSound, GetActorLocation(),
				1.0f, FMath::FRandRange(0.85f, 1.15f));
		}
	}

	return true;
}

int32 ALiminalEntity::OnAttackNotify(
	FName SocketName,
	float Radius,
	float ForwardDistance,
	float DamageOverride,
	TSubclassOf<UDamageType> DamageType,
	bool bDrawDebug)
{
	if (!HasAuthority() || CurrentHealth <= 0.0f || IsStunned() || IsCalmed() || !bAttackImpactPending)
	{
		return 0;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return 0;
	}
	// Consume even on a miss: repeated notifies cannot turn one swing into multiple hits.
	bAttackImpactPending = false;

	const FName TargetSocket = (SocketName != NAME_None) ? SocketName : DefaultAttackSocket;
	const float ActualRadius = (Radius > 0.0f) ? Radius : DefaultAttackTraceRadius;
	const float ActualDistance = (ForwardDistance > 0.0f) ? ForwardDistance : DefaultAttackTraceDistance;
	const float EffectiveDamage = (DamageOverride >= 0.0f) ? DamageOverride : AttackDamage;

	FVector StartPoint;
	FVector EndPoint;

	USkeletalMeshComponent* SkelMesh = GetMesh();
	if (SkelMesh && SkelMesh->DoesSocketExist(TargetSocket))
	{
		StartPoint = SkelMesh->GetSocketLocation(TargetSocket);
		const FRotator SocketRot = SkelMesh->GetSocketRotation(TargetSocket);
		EndPoint = StartPoint + SocketRot.Vector() * ActualDistance;
	}
	else
	{
		StartPoint = GetActorLocation() + FVector(0.0f, 0.0f, 40.0f);
		EndPoint = StartPoint + GetActorForwardVector() * ActualDistance;
	}

	FCollisionShape Sphere = FCollisionShape::MakeSphere(ActualRadius);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LiminalAttackSweep), false, this);
	QueryParams.AddIgnoredActor(this);
	QueryParams.bReturnPhysicalMaterial = true;

	TArray<FHitResult> OutHits;
	const bool bHitAny = World->SweepMultiByChannel(
		OutHits, StartPoint, EndPoint, FQuat::Identity, ECC_Pawn, Sphere, QueryParams);

#if !UE_BUILD_SHIPPING
	if (bDrawDebug)
	{
		DrawDebugCapsule(World, (StartPoint + EndPoint) * 0.5f,
			ActualDistance * 0.5f + ActualRadius, ActualRadius,
			FRotationMatrix::MakeFromZ(EndPoint - StartPoint).ToQuat(),
			bHitAny ? FColor::Red : FColor::Green, false, 1.5f, 0, 1.5f);
	}
#endif

	if (!bHitAny)
	{
		return 0;
	}

	TSet<AActor*> DamagedActors;
	int32 HitCount = 0;
	const FVector HitDir = (EndPoint - StartPoint).GetSafeNormal();

	for (const FHitResult& Hit : OutHits)
	{
		AActor* HitActor = Hit.GetActor();
		if (!HitActor || HitActor == this || DamagedActors.Contains(HitActor))
		{
			continue;
		}

		DamagedActors.Add(HitActor);

		if (AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(HitActor))
		{
			if (Scavenger->IsDead() || Scavenger->IsDowned() || Scavenger->IsHiddenInSpot())
			{
				continue;
			}

			FHitResult OcclusionHit;
			FCollisionQueryParams OcclusionParams = QueryParams;
			TArray<AActor*> AttachedActors;
			Scavenger->GetAttachedActors(AttachedActors);
			OcclusionParams.AddIgnoredActors(AttachedActors);
			if (World->LineTraceSingleByChannel(OcclusionHit, StartPoint, Scavenger->GetActorLocation(),
				ECC_Visibility, OcclusionParams) && OcclusionHit.GetActor() != Scavenger)
			{
				continue;
			}

			UGameplayStatics::ApplyPointDamage(
				Scavenger,
				EffectiveDamage,
				HitDir,
				Hit,
				GetController(),
				this,
				DamageType ? DamageType : TSubclassOf<UDamageType>(UDamageType::StaticClass())
			);

			USoundBase* ImpactSoundToPlay = AttackImpactSound ? AttackImpactSound.Get() : AttackSound.Get();
			if (ImpactSoundToPlay)
			{
				UGameplayStatics::PlaySoundAtLocation(World, ImpactSoundToPlay, Hit.ImpactPoint,
					1.0f, FMath::FRandRange(0.9f, 1.1f));
			}

			UAISense_Hearing::ReportNoiseEvent(World, Hit.ImpactPoint, 1.0f, this);
			HitCount++;
		}
	}

	return HitCount;
}

float ALiminalEntity::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority() || Damage <= 0.0f || CurrentHealth <= 0.0f)
	{
		return 0.0f;
	}

	const float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
	CurrentHealth = FMath::Max(CurrentHealth - ActualDamage, 0.0f);

	if (CurrentHealth <= 0.0f)
	{
		SetActorEnableCollision(false);

		// Recompense d'expedition : drop de butin / residu anomalique sur le monstre neutralise
		if (UWorld* World = GetWorld())
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			const FVector DropLocation = GetActorLocation() + FVector(0.0f, 0.0f, 25.0f);
			if (ALootActor* Loot = World->SpawnActor<ALootActor>(ALootActor::StaticClass(), DropLocation, GetActorRotation(), SpawnParams))
			{
				Loot->SetRandomizedStats(FMath::FRandRange(1.8f, 3.8f), FMath::RandRange(85, 180));
			}
		}

		SetLifeSpan(3.0f);
	}

	return ActualDamage;
}
