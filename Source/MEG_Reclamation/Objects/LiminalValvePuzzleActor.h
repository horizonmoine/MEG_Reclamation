#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalValvePuzzleActor.generated.h"

class UStaticMeshComponent;
class UAudioComponent;
class ALiminalDoorActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValvePressureChanged, float, NewPressure);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValvePuzzleSolved, bool, bSuccess);

/**
 * Puzzle de regulation de pression de vapeur pour le Niveau 2 (Pipe Dreams)
 * et les complexes industriels liminaux.
 * 
 * Totalement replique (Server-Authoritative) et synchronise.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalValvePuzzleActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalValvePuzzleActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Interagit avec une des 3 vannes (Index 0, 1 ou 2) */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Puzzle")
	void InteractWithValve(int32 ValveIndex, AActor* InInstigator);

	/** Interaction générique avec la vanne principale */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Puzzle")
	void Interact(AActor* InInstigator) { InteractWithValve(0, InInstigator); }

	UFUNCTION(BlueprintPure, Category = "Liminal|Puzzle")
	float GetCurrentPressure() const { return CurrentPressurePSI; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Puzzle")
	bool IsPressureInSafeZone() const;

	UFUNCTION(BlueprintPure, Category = "Liminal|Puzzle")
	bool IsPuzzleSolved() const { return bIsSolved; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|Puzzle")
	void SetLinkedDoor(ALiminalDoorActor* InDoor);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Puzzle")
	FOnValvePressureChanged OnPressureChanged;

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Puzzle")
	FOnValvePuzzleSolved OnPuzzleSolved;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnRep_CurrentPressure();

	UFUNCTION()
	void OnRep_IsSolved();

private:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PipeBoardMesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Valve0Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Valve1Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> Valve2Mesh;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentPressure, BlueprintReadOnly, Category = "Liminal|Puzzle", meta = (AllowPrivateAccess = "true"))
	float CurrentPressurePSI = 180.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsSolved, BlueprintReadOnly, Category = "Liminal|Puzzle", meta = (AllowPrivateAccess = "true"))
	bool bIsSolved = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Puzzle", meta = (AllowPrivateAccess = "true"))
	float SafePressureMin = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Puzzle", meta = (AllowPrivateAccess = "true"))
	float SafePressureMax = 110.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Puzzle", meta = (AllowPrivateAccess = "true"))
	float OverpressureThreshold = 220.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Puzzle", meta = (AllowPrivateAccess = "true"))
	float RequiredStabilizationTime = 3.0f;

	UPROPERTY(EditInstanceOnly, Category = "Liminal|Puzzle")
	TObjectPtr<ALiminalDoorActor> LinkedDoor;

	float CurrentStabilizedDuration = 0.0f;

	// Delays/Coefficients de pression par vanne
	float ValveDeltaPSI[3] = { -25.0f, 15.0f, -40.0f };
};
