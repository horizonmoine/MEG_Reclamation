#include "Inventory/LiminalInventoryTypes.h"

// ---------------------------------------------------------------------------
// FLiminalInventoryEntry
// ---------------------------------------------------------------------------

bool FLiminalInventoryEntry::Overlaps(const FLiminalInventoryEntry& Other) const
{
	const FIntPoint A = GetFootprint();
	const FIntPoint B = Other.GetFootprint();

	const bool bSeparatedX = GridPos.X + A.X <= Other.GridPos.X || Other.GridPos.X + B.X <= GridPos.X;
	const bool bSeparatedY = GridPos.Y + A.Y <= Other.GridPos.Y || Other.GridPos.Y + B.Y <= GridPos.Y;
	return !(bSeparatedX || bSeparatedY);
}

bool FLiminalInventoryEntry::FitsInGrid(const FIntPoint& GridSize) const
{
	const FIntPoint Footprint = GetFootprint();
	if (Footprint.X <= 0 || Footprint.Y <= 0 || GridPos.X < 0 || GridPos.Y < 0)
	{
		return false;
	}
	return GridPos.X + Footprint.X <= GridSize.X && GridPos.Y + Footprint.Y <= GridSize.Y;
}

void FLiminalInventoryEntry::PreReplicatedRemove(const FLiminalInventoryList& InArraySerializer)
{
	if (InArraySerializer.OnReplicatedChange)
	{
		InArraySerializer.OnReplicatedChange();
	}
}

void FLiminalInventoryEntry::PostReplicatedAdd(const FLiminalInventoryList& InArraySerializer)
{
	if (InArraySerializer.OnReplicatedChange)
	{
		InArraySerializer.OnReplicatedChange();
	}
}

void FLiminalInventoryEntry::PostReplicatedChange(const FLiminalInventoryList& InArraySerializer)
{
	if (InArraySerializer.OnReplicatedChange)
	{
		InArraySerializer.OnReplicatedChange();
	}
}

// ---------------------------------------------------------------------------
// FLiminalInventoryList
// ---------------------------------------------------------------------------

bool FLiminalInventoryList::CanPlace(const FLiminalInventoryEntry& Candidate, int32 IgnoreEntryId) const
{
	if (!Candidate.FitsInGrid(GridSize))
	{
		return false;
	}

	for (const FLiminalInventoryEntry& Existing : Items)
	{
		if (Existing.EntryId == IgnoreEntryId)
		{
			continue;
		}
		if (Candidate.Overlaps(Existing))
		{
			return false;
		}
	}

	return true;
}

int32 FLiminalInventoryList::AddEntry(FLiminalInventoryEntry Entry)
{
	if (!CanPlace(Entry))
	{
		return INDEX_NONE;
	}

	Entry.EntryId = NextEntryId++;
	FLiminalInventoryEntry& Added = Items.Add_GetRef(MoveTemp(Entry));
	MarkItemDirty(Added);
	return Added.EntryId;
}

bool FLiminalInventoryList::RemoveEntry(int32 EntryId)
{
	const int32 Index = Items.IndexOfByPredicate([EntryId](const FLiminalInventoryEntry& Entry)
	{
		return Entry.EntryId == EntryId;
	});

	if (Index == INDEX_NONE)
	{
		return false;
	}

	Items.RemoveAt(Index);
	MarkArrayDirty();
	return true;
}

bool FLiminalInventoryList::MoveEntry(int32 EntryId, const FIntPoint& NewPos, bool bNewRotated)
{
	FLiminalInventoryEntry* Entry = Items.FindByPredicate([EntryId](const FLiminalInventoryEntry& Candidate)
	{
		return Candidate.EntryId == EntryId;
	});

	if (!Entry)
	{
		return false;
	}

	FLiminalInventoryEntry Moved = *Entry;
	Moved.GridPos = NewPos;
	Moved.bRotated = bNewRotated;

	if (!CanPlace(Moved, EntryId))
	{
		return false;
	}

	Entry->GridPos = NewPos;
	Entry->bRotated = bNewRotated;
	MarkItemDirty(*Entry);
	return true;
}

const FLiminalInventoryEntry* FLiminalInventoryList::FindEntry(int32 EntryId) const
{
	return Items.FindByPredicate([EntryId](const FLiminalInventoryEntry& Entry)
	{
		return Entry.EntryId == EntryId;
	});
}

bool FLiminalInventoryList::FindFreeSlot(const FIntPoint& Footprint, FIntPoint& OutPos) const
{
	FLiminalInventoryEntry Probe;
	Probe.Size = Footprint;

	for (int32 Y = 0; Y + Footprint.Y <= GridSize.Y; ++Y)
	{
		for (int32 X = 0; X + Footprint.X <= GridSize.X; ++X)
		{
			Probe.GridPos = FIntPoint(X, Y);
			if (CanPlace(Probe))
			{
				OutPos = Probe.GridPos;
				return true;
			}
		}
	}

	return false;
}

float FLiminalInventoryList::GetTotalWeightKg() const
{
	float Total = 0.0f;
	for (const FLiminalInventoryEntry& Entry : Items)
	{
		Total += Entry.WeightKg;
	}
	return Total;
}

int32 FLiminalInventoryList::GetTotalValue() const
{
	int32 Total = 0;
	for (const FLiminalInventoryEntry& Entry : Items)
	{
		Total += Entry.Value;
	}
	return Total;
}
