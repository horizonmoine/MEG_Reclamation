#include "AI/LiminalAIController.h"

#include "AI/LiminalEntity.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Sound/SoundBase.h"

const FName ALiminalAIController::PriorityTargetKey(TEXT("PriorityTarget"));

ALiminalAIController::ALiminalAIController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALiminalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const ALiminalEntity* Entity = Cast<ALiminalEntity>(InPawn);
	if (!Entity)
	{
		return;
	}

	if (UAIPerceptionComponent* Perception = Entity->GetPerceptionComponent())
	{
		Perception->OnTargetPerceptionUpdated.AddDynamic(this, &ALiminalAIController::OnTargetPerceptionUpdated);
	}

	UBehaviorTree* BehaviorToRun = Entity->GetInitialBehaviorTree();
	if (!BehaviorToRun)
	{
		BehaviorToRun = DefaultBehaviorTree;
	}

	if (BehaviorToRun)
	{
		RunBehaviorTree(BehaviorToRun);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ALiminalAIController::OnPossess - No BehaviorTree set on %s"), *GetName());
	}
}

void ALiminalAIController::OnUnPossess()
{
	if (const ALiminalEntity* Entity = Cast<ALiminalEntity>(GetPawn()))
	{
		if (UAIPerceptionComponent* Perception = Entity->GetPerceptionComponent())
		{
			Perception->OnTargetPerceptionUpdated.RemoveDynamic(this, &ALiminalAIController::OnTargetPerceptionUpdated);
		}
	}

	Super::OnUnPossess();
}

void ALiminalAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!Blackboard || !GetPawn())
	{
		return;
	}

	ALiminalEntity* Entity = Cast<ALiminalEntity>(GetPawn());
	if (Entity)
	{
		if (Entity->IsStunned())
		{
			StopMovement();
			return;
		}

		if (Entity->IsCalmed())
		{
			CurrentNoiseLocation = FVector::ZeroVector;
			LastNoiseHeardTime = -1.0;
		}

		if (Entity->CanAttack())
		{
			const float AttackRange = Entity->GetAttackRange();
			const FVector EntityLoc = Entity->GetActorLocation();

			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				if (APlayerController* PC = It->Get())
				{
					if (APawn* PlayerPawn = PC->GetPawn())
					{
						if (FVector::DistSquared(EntityLoc, PlayerPawn->GetActorLocation()) <= FMath::Square(AttackRange + 30.0f))
						{
							Entity->PerformMeleeAttack(PlayerPawn);
							break;
						}
					}
				}
			}
		}
	}

	if (LastNoiseHeardTime >= 0.0 &&
		GetWorld()->GetTimeSeconds() - LastNoiseHeardTime > NoiseMemorySeconds)
	{
		CurrentNoiseLocation = FVector::ZeroVector;
		LastNoiseHeardTime = -1.0;
	}

	RefreshPatrolPoint();
	CommitPriorityTarget();
}

void ALiminalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !Stimulus.WasSuccessfullySensed())
	{
		return;
	}

	const bool bFirstAlert = (LastNoiseHeardTime < 0.0);
	CurrentNoiseLocation = Stimulus.StimulusLocation;
	LastNoiseHeardTime = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;

	if (bFirstAlert && GetWorld())
	{
		if (const ALiminalEntity* Entity = Cast<ALiminalEntity>(GetPawn()))
		{
			if (USoundBase* Aggro = Entity->GetAggroSound())
			{
				UGameplayStatics::PlaySoundAtLocation(GetWorld(), Aggro, Entity->GetActorLocation(),
					0.95f, FMath::FRandRange(0.9f, 1.1f));
			}
		}
	}
}

void ALiminalAIController::RefreshPatrolPoint()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	const APawn* ControlledPawn = GetPawn();
	if (!NavSys || !ControlledPawn)
	{
		return;
	}

	const bool bReached = bHasValidPatrol &&
		FVector::DistSquared2D(ControlledPawn->GetActorLocation(), CurrentPatrolLocation) <
			FMath::Square(PatrolReachTolerance);

	if (bHasValidPatrol && !bReached)
	{
		return;
	}

	FNavLocation NewPatrol(FVector::ZeroVector);
	if (NavSys->GetRandomReachablePointInRadius(ControlledPawn->GetActorLocation(), PatrolRadius, NewPatrol))
	{
		CurrentPatrolLocation = NewPatrol.Location;
		bHasValidPatrol = true;
	}
	else
	{
		bHasValidPatrol = false;
	}
}

void ALiminalAIController::CommitPriorityTarget()
{
	const bool bNoiseFresh = LastNoiseHeardTime >= 0.0;
	const FVector Target = bNoiseFresh ? CurrentNoiseLocation : CurrentPatrolLocation;

	if (!bNoiseFresh && !bHasValidPatrol)
	{
		return;
	}

	if (!bHasCommittedTarget || !Target.Equals(CommittedTarget, 1.0f))
	{
		Blackboard->SetValueAsVector(PriorityTargetKey, Target);
		CommittedTarget = Target;
		bHasCommittedTarget = true;
	}
}
