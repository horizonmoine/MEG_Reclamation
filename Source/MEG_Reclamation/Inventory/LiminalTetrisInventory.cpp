#include "Inventory/LiminalTetrisInventory.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"

ULiminalTetrisInventoryComponent::ULiminalTetrisInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULiminalTetrisInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
}

void ULiminalTetrisInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULiminalTetrisInventoryComponent, StoredItems);
}

bool ULiminalTetrisInventoryComponent::CanPlaceItemAt(const FTetrisItemInstance& Item, const FIntPoint& TargetSlot, const FGuid& IgnoredItemId) const
{
	const FIntPoint EffectiveSize = Item.GetEffectiveDimensions();

	if (TargetSlot.X < 0 || TargetSlot.Y < 0)
	{
		return false;
	}

	if (TargetSlot.X + EffectiveSize.X > GridColumns || TargetSlot.Y + EffectiveSize.Y > GridRows)
	{
		return false;
	}

	const FIntRect NewItemRect(TargetSlot.X, TargetSlot.Y, TargetSlot.X + EffectiveSize.X, TargetSlot.Y + EffectiveSize.Y);

	for (const FTetrisItemInstance& Existing : StoredItems)
	{
		if (Existing.ItemId == IgnoredItemId)
		{
			continue;
		}

		const FIntPoint ExistingSize = Existing.GetEffectiveDimensions();
		const FIntRect ExistingRect(
			Existing.GridPosition.X,
			Existing.GridPosition.Y,
			Existing.GridPosition.X + ExistingSize.X,
			Existing.GridPosition.Y + ExistingSize.Y
		);

		// Verifier intersection de rectangles
		if (NewItemRect.Min.X < ExistingRect.Max.X && NewItemRect.Max.X > ExistingRect.Min.X &&
			NewItemRect.Min.Y < ExistingRect.Max.Y && NewItemRect.Max.Y > ExistingRect.Min.Y)
		{
			return false;
		}
	}

	return true;
}

bool ULiminalTetrisInventoryComponent::AutoAddItem(FTetrisItemInstance NewItem, FIntPoint& OutPlacedSlot)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	// Recherche d'un emplacement sans rotation
	for (int32 Y = 0; Y < GridRows; ++Y)
	{
		for (int32 X = 0; X < GridColumns; ++X)
		{
			const FIntPoint Slot(X, Y);
			NewItem.bIsRotated = false;
			if (CanPlaceItemAt(NewItem, Slot))
			{
				NewItem.GridPosition = Slot;
				StoredItems.Add(NewItem);
				OutPlacedSlot = Slot;
				SyncWeightToCharacter();
				OnInventoryChanged.Broadcast();
				return true;
			}

			// Tentative avec rotation
			NewItem.bIsRotated = true;
			if (CanPlaceItemAt(NewItem, Slot))
			{
				NewItem.GridPosition = Slot;
				StoredItems.Add(NewItem);
				OutPlacedSlot = Slot;
				SyncWeightToCharacter();
				OnInventoryChanged.Broadcast();
				return true;
			}
		}
	}

	return false;
}

bool ULiminalTetrisInventoryComponent::MoveItem(const FGuid& ItemId, const FIntPoint& NewSlot)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerMoveItem(ItemId, NewSlot);
		return true;
	}

	for (FTetrisItemInstance& Item : StoredItems)
	{
		if (Item.ItemId == ItemId)
		{
			if (CanPlaceItemAt(Item, NewSlot, ItemId))
			{
				Item.GridPosition = NewSlot;
				OnInventoryChanged.Broadcast();
				return true;
			}
			return false;
		}
	}

	return false;
}

void ULiminalTetrisInventoryComponent::ServerMoveItem_Implementation(const FGuid& ItemId, const FIntPoint& NewSlot)
{
	MoveItem(ItemId, NewSlot);
}

bool ULiminalTetrisInventoryComponent::RotateItem(const FGuid& ItemId)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerRotateItem(ItemId);
		return true;
	}

	for (FTetrisItemInstance& Item : StoredItems)
	{
		if (Item.ItemId == ItemId)
		{
			FTetrisItemInstance TempItem = Item;
			TempItem.bIsRotated = !TempItem.bIsRotated;

			if (CanPlaceItemAt(TempItem, Item.GridPosition, ItemId))
			{
				Item.bIsRotated = TempItem.bIsRotated;
				OnInventoryChanged.Broadcast();
				return true;
			}
			return false;
		}
	}

	return false;
}

void ULiminalTetrisInventoryComponent::ServerRotateItem_Implementation(const FGuid& ItemId)
{
	RotateItem(ItemId);
}

bool ULiminalTetrisInventoryComponent::RemoveItem(const FGuid& ItemId, FTetrisItemInstance& OutRemovedItem)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerRemoveItem(ItemId);
		return true;
	}

	for (int32 Index = 0; Index < StoredItems.Num(); ++Index)
	{
		if (StoredItems[Index].ItemId == ItemId)
		{
			OutRemovedItem = StoredItems[Index];
			StoredItems.RemoveAt(Index);
			SyncWeightToCharacter();
			OnInventoryChanged.Broadcast();
			return true;
		}
	}

	return false;
}

void ULiminalTetrisInventoryComponent::ServerRemoveItem_Implementation(const FGuid& ItemId)
{
	FTetrisItemInstance Dummy;
	RemoveItem(ItemId, Dummy);
}

float ULiminalTetrisInventoryComponent::CalculateTotalWeight() const
{
	float Total = 0.0f;
	for (const FTetrisItemInstance& Item : StoredItems)
	{
		Total += Item.WeightKg;
	}
	return Total;
}

int32 ULiminalTetrisInventoryComponent::CalculateTotalValue() const
{
	int32 Total = 0;
	for (const FTetrisItemInstance& Item : StoredItems)
	{
		Total += Item.ValueCredits;
	}
	return Total;
}

void ULiminalTetrisInventoryComponent::SyncWeightToCharacter()
{
	if (AScavengerCharacter* Scavenger = Cast<AScavengerCharacter>(GetOwner()))
	{
		// Met a jour le poids direct si autorite serveur
		if (Scavenger->HasAuthority())
		{
			// Reajustement du poids porte
		}
	}
}

void ULiminalTetrisInventoryComponent::OnRep_StoredItems()
{
	OnInventoryChanged.Broadcast();
}
