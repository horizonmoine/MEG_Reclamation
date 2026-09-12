#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalAirlockActor.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class AScavengerCharacter;
class ALiminalSafeZoneVolume;

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Airlock")
	void Interact(AScavengerCharacter* Operator);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Airlock")
	void ServerActivateAirlock(AScavengerCharacter* Operator);

	UFUNCTION(BlueprintPure, Category = "Airlock")
	bool IsCycleActive() const { return bIsCycleActive; }

	UFUNCTION(BlueprintPure, Category = "Airlock")
	bool IsSealed() const { return bIsSealed; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Airlock")
	void SetSealed(bool bInSealed);

	UFUNCTION(BlueprintPure, Category = "Airlock")
	bool IsActorInsideAirlock(const AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Airlock")
	UBoxComponent* GetChamberVolume() const { return ChamberVolume; }

	UFUNCTION(BlueprintCallable, Category = "Airlock")
	ALiminalSafeZoneVolume* GetSafeZoneVolume();

	void EnsureSafeZoneVolume();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	TObjectPtr<ALiminalSafeZoneVolume> AirlockSafeZoneVolume;

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

	UPROPERTY(ReplicatedUsing = OnRep_IsSealed, VisibleAnywhere, BlueprintReadOnly, Category = "Airlock")
	bool bIsSealed = false;

	UFUNCTION()
	void OnRep_CycleActive();

	UFUNCTION()
	void OnRep_IsSealed();

	float CycleTimer = 0.0f;
};
