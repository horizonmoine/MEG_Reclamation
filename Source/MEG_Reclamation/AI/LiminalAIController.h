#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "LiminalAIController.generated.h"

class UBehaviorTree;
struct FAIStimulus;

/**
 * Controleur IA generique des entites liminales.
 *
 * Modele : une cible unique "PriorityTarget" ecrite en blackboard a chaque tick.
 * Priorite = bruit frais (NoiseMemorySeconds) sinon point de patrouille cyclique.
 * Le BT associe se reduit donc a une tache MoveTo sur cette cle unique.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalAIController : public AAIController
{
	GENERATED_BODY()

public:
	static const FName PriorityTargetKey;

	ALiminalAIController();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	void RefreshPatrolPoint();
	void CommitPriorityTarget();

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> DefaultBehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Patrol", meta = (ClampMin = "0.0"))
	float PatrolRadius = 1500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Patrol", meta = (ClampMin = "0.0"))
	float PatrolReachTolerance = 120.0f;

	UPROPERTY(EditDefaultsOnly, Category = "AI|Hearing", meta = (ClampMin = "0.0"))
	float NoiseMemorySeconds = 8.0f;

private:
	FVector CurrentNoiseLocation = FVector::ZeroVector;
	double LastNoiseHeardTime = -1.0;

	FVector CurrentPatrolLocation = FVector::ZeroVector;
	bool bHasValidPatrol = false;

	FVector CommittedTarget = FVector::ZeroVector;
	bool bHasCommittedTarget = false;
};
