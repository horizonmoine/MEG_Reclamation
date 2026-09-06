#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalBreakerActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBreakerSwitchedSignature, ALiminalBreakerActor*, Breaker, bool, bPowerState);

/**
 * Boitier de disjoncteur mural diegetique (style Escape the Backrooms).
 * Permet de retablir l'electricite apres une panne "Lights Out" ou d'alimenter des equipements M.E.G.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalBreakerActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalBreakerActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Breaker")
	void ToggleBreaker();

	UFUNCTION(BlueprintCallable, Category = "Breaker")
	void SetBreakerState(bool bNewState);

	UFUNCTION(BlueprintPure, Category = "Breaker")
	bool IsPowerRestored() const { return bIsPowerRestored; }

	UPROPERTY(BlueprintAssignable, Category = "Breaker|Events")
	FOnBreakerSwitchedSignature OnBreakerSwitched;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BreakerBoxMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> SwitchLeverMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> IndicatorLight;

	UPROPERTY(ReplicatedUsing = OnRep_IsPowerRestored, EditAnywhere, BlueprintReadWrite, Category = "Breaker")
	bool bIsPowerRestored = false;

	UFUNCTION()
	void OnRep_IsPowerRestored();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void UpdateVisualState();
};
