#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalTetrisInventory.generated.h"

class ALootActor;

USTRUCT(BlueprintType)
struct FTetrisItemInstance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris", meta = (IgnoreForMemberInitializationTest))
	FGuid ItemId = FGuid();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	FName ItemIdentifier = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	FIntPoint GridPosition = FIntPoint::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	FIntPoint Dimensions = FIntPoint(1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	bool bIsRotated = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	float WeightKg = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	int32 ValueCredits = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tetris")
	TSoftClassPtr<ALootActor> LootClass;

	FIntPoint GetEffectiveDimensions() const
	{
		return bIsRotated ? FIntPoint(Dimensions.Y, Dimensions.X) : Dimensions;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTetrisInventoryChanged);

/**
 * Inventaire spatial a grille diegetique type Resident Evil / Deus Ex.
 * Server-authoritative : gestion des collisions de cellules, rotations 90 deg,
 * transfert de poids physique vers le ScavengerCharacter.
 */
UCLASS(ClassGroup = (Inventory), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalTetrisInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalTetrisInventoryComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Verifie si l'item peut etre place a la coordonnee specifiee sans collision */
	UFUNCTION(BlueprintPure, Category = "Liminal|Inventory")
	bool CanPlaceItemAt(const FTetrisItemInstance& Item, const FIntPoint& TargetSlot, const FGuid& IgnoredItemId = FGuid()) const;

	/** Ajoute un item en trouvant automatiquement la premiere cellule libre */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Inventory")
	bool AutoAddItem(FTetrisItemInstance NewItem, FIntPoint& OutPlacedSlot);

	/** Deplace un item existant vers un nouveau slot */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Inventory")
	bool MoveItem(const FGuid& ItemId, const FIntPoint& NewSlot);

	/** Pivote un item de 90 degres */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Inventory")
	bool RotateItem(const FGuid& ItemId);

	/** Retire un item de l'inventaire */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Inventory")
	bool RemoveItem(const FGuid& ItemId, FTetrisItemInstance& OutRemovedItem);

	UFUNCTION(BlueprintPure, Category = "Liminal|Inventory")
	int32 GetGridWidth() const { return GridColumns; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Inventory")
	int32 GetGridHeight() const { return GridRows; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Inventory")
	const TArray<FTetrisItemInstance>& GetItems() const { return StoredItems; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Inventory")
	float CalculateTotalWeight() const;

	UFUNCTION(BlueprintPure, Category = "Liminal|Inventory")
	int32 CalculateTotalValue() const;

	// Server RPCs
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Liminal|Inventory")
	void ServerMoveItem(const FGuid& ItemId, const FIntPoint& NewSlot);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Liminal|Inventory")
	void ServerRotateItem(const FGuid& ItemId);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Liminal|Inventory")
	void ServerRemoveItem(const FGuid& ItemId);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Inventory")
	FOnTetrisInventoryChanged OnInventoryChanged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Inventory|Grid", meta = (ClampMin = "2", ClampMax = "16"))
	int32 GridColumns = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Inventory|Grid", meta = (ClampMin = "2", ClampMax = "16"))
	int32 GridRows = 6;

	UPROPERTY(ReplicatedUsing = OnRep_StoredItems)
	TArray<FTetrisItemInstance> StoredItems;

	UFUNCTION()
	void OnRep_StoredItems();

	void SyncWeightToCharacter();
};
