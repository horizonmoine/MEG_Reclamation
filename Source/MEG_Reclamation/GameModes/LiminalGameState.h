#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LiminalGameState.generated.h"

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
};
