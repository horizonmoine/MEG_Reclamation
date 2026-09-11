#pragma once

#include "CoreMinimal.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "LiminalInventoryTypes.generated.h"

struct FLiminalInventoryList;

/**
 * Entree d'inventaire Tetris. Positions et tailles en cellules de grille.
 * Repliquee par delta (seule l'entree modifiee transite sur le reseau).
 */
USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FLiminalInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	/** Identifiant stable attribue par le serveur. INDEX_NONE tant que non place. */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 EntryId = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FName ItemId;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FIntPoint GridPos = FIntPoint::ZeroValue;

	/** Taille non tournee (largeur, hauteur). */
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FIntPoint Size = FIntPoint(1, 1);

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	bool bRotated = false;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	float WeightKg = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 Value = 0;

	FIntPoint GetFootprint() const { return bRotated ? FIntPoint(Size.Y, Size.X) : Size; }
	bool Overlaps(const FLiminalInventoryEntry& Other) const;
	bool FitsInGrid(const FIntPoint& GridSize) const;

	// Callbacks FFastArraySerializer (cote client).
	void PreReplicatedRemove(const FLiminalInventoryList& InArraySerializer);
	void PostReplicatedAdd(const FLiminalInventoryList& InArraySerializer);
	void PostReplicatedChange(const FLiminalInventoryList& InArraySerializer);
};

/**
 * Liste d'inventaire repliquee par delta. Toute mutation passe par AddEntry / RemoveEntry
 * / MoveEntry (serveur), qui marquent les items dirty.
 */
USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FLiminalInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	TArray<FLiminalInventoryEntry> Items;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FIntPoint GridSize = FIntPoint(6, 4);

	/** Compteur serveur, non replique. */
	int32 NextEntryId = 0;

	/** Notification locale (client) apres application d'un delta reseau. */
	TFunction<void()> OnReplicatedChange;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FLiminalInventoryEntry, FLiminalInventoryList>(Items, DeltaParms, *this);
	}

	/** Verifie bornes et chevauchement. IgnoreEntryId permet de tester un deplacement. */
	bool CanPlace(const FLiminalInventoryEntry& Candidate, int32 IgnoreEntryId = INDEX_NONE) const;

	/** Attribue un EntryId et ajoute. Retourne INDEX_NONE si placement invalide. */
	int32 AddEntry(FLiminalInventoryEntry Entry);

	bool RemoveEntry(int32 EntryId);

	bool MoveEntry(int32 EntryId, const FIntPoint& NewPos, bool bNewRotated);

	const FLiminalInventoryEntry* FindEntry(int32 EntryId) const;

	bool FindFreeSlot(const FIntPoint& Footprint, FIntPoint& OutPos) const;

	float GetTotalWeightKg() const;
	int32 GetTotalValue() const;
};

template<>
struct TStructOpsTypeTraits<FLiminalInventoryList> : public TStructOpsTypeTraitsBase2<FLiminalInventoryList>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
