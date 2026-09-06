#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalAirlockActor.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class AScavengerCharacter;

/**
 * Sas d'embarquement et de debarquement de la Base Alpha (Hub).
 * Permet a l'escouade de monter a bord du sas, d'actionner le levier
 * de decontamination et d'etre deployee dans l'etage procedural selectionne.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalAirlockActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalAirlockActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Airlock")
	void Interact(AScavengerCharacter* Operator);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Airlock")
	void ServerActivateAirlock(AScavengerCharacter* Operator);

	UFUNCTION(BlueprintPure, Category = "Airlock")
	bool IsCycleActive() const { return bIsCycleActive; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	TObjectPtr<UBoxComponent> ChamberVolume;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	TObjectPtr<UBoxComponent> InteractionTrigger;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	TObjectPtr<UStaticMeshComponent> AirlockFrame;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	TObjectPtr<UStaticMeshComponent> LeverMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	TObjectPtr<UPointLightComponent> StatusLight;

	UPROPERTY(ReplicatedUsing = OnRep_CycleActive)
	bool bIsCycleActive = false;

	UFUNCTION()
	void OnRep_CycleActive();

	float CycleTimer = 0.0f;
};
