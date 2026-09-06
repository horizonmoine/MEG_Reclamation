#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalFuseBoxActor.generated.h"

class UStaticMeshComponent;
class ALiminalDoorActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPowerStateChanged, bool, bIsPowered);

/**
 * Boitier de fusibles electrique pour le Niveau 1 (Habitable Zone) et Niveau 3 (Electrical Station).
 * Exige de trouver et d'enficher les fusibles du bon amperrage (ex: 15A, 30A, 50A) pour retablir le courant.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalFuseBoxActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalFuseBoxActor();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Insere un fusible d'un certain amperage dans un slot specifique (Slot 0, 1, ou 2) */
	UFUNCTION(BlueprintCallable, Category = "Liminal|FuseBox")
	bool InsertFuse(int32 SlotIndex, int32 FuseAmperage, AActor* InInstigator);

	/** Interaction générique pour insérer ou inspecter le fusible */
	UFUNCTION(BlueprintCallable, Category = "Liminal|FuseBox")
	void Interact(AActor* InInstigator);

	/** Retire le fusible du slot specifie */
	UFUNCTION(BlueprintCallable, Category = "Liminal|FuseBox")
	int32 RemoveFuse(int32 SlotIndex);

	UFUNCTION(BlueprintPure, Category = "Liminal|FuseBox")
	bool IsPowerRestored() const { return bPowerRestored; }

	UFUNCTION(BlueprintPure, Category = "Liminal|FuseBox")
	int32 GetSlottedFuse(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable, Category = "Liminal|FuseBox")
	void SetLinkedDoor(ALiminalDoorActor* InDoor);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|FuseBox")
	FOnPowerStateChanged OnPowerStateChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_SlottedFuses();

	UFUNCTION()
	void OnRep_PowerRestored();

private:
	void CheckPowerStatus();

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> BoxMesh;

	UPROPERTY(ReplicatedUsing = OnRep_SlottedFuses, BlueprintReadOnly, Category = "Liminal|FuseBox", meta = (AllowPrivateAccess = "true"))
	TArray<int32> SlottedFuses;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|FuseBox", meta = (AllowPrivateAccess = "true"))
	TArray<int32> RequiredFuses;

	UPROPERTY(ReplicatedUsing = OnRep_PowerRestored, BlueprintReadOnly, Category = "Liminal|FuseBox", meta = (AllowPrivateAccess = "true"))
	bool bPowerRestored = false;

	UPROPERTY(EditInstanceOnly, Category = "Liminal|FuseBox")
	TObjectPtr<ALiminalDoorActor> LinkedDoor;
};
