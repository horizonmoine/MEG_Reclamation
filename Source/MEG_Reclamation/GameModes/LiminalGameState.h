#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LiminalGameState.generated.h"

UENUM(BlueprintType)
enum class EStabilityPhase : uint8
{
	Normal UMETA(DisplayName = "100-60% Stable"),
	Destabilized UMETA(DisplayName = "60-20% Déstabilisé (Lumières & Portes)"),
	Collapse UMETA(DisplayName = "20-0% Effondrement (Hordes & Dégâts)")
};

UCLASS()
class MEG_RECLAMATION_API ALiminalGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALiminalGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_BlackoutActive, BlueprintReadOnly, Category = "Mission|Events")
	bool bIsBlackoutActive;

	UFUNCTION()
	void OnRep_BlackoutActive();

	/** Stabilite globale du niveau liminal (100% -> 0%) */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStability, BlueprintReadOnly, Category = "Mission|Stability")
	float CurrentStability;

	UFUNCTION()
	void OnRep_CurrentStability();

	UFUNCTION(BlueprintPure, Category = "Mission|Stability")
	EStabilityPhase GetStabilityPhase() const;

	UFUNCTION(BlueprintCallable, Category = "Mission|Stability")
	void SetStability(float NewStability);

	UFUNCTION(BlueprintCallable, Category = "Mission|Stability")
	void DrainStability(float Amount);
};

