#include "ProcGen/LiminalTileSet.h"

ULiminalTileSet::ULiminalTileSet()
{
	TargetBiome = ELevelBiome::Level0_YellowLobby;
}

FPrimaryAssetId ULiminalTileSet::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FName("LiminalTileSet"), GetFName());
}

ULiminalTileData* ULiminalTileSet::FindTileByType(ELiminalTileType DesiredType, FRandomStream& InRandomStream) const
{
	TArray<ULiminalTileData*> Pool;

	auto FilterPool = [&](const TArray<TObjectPtr<ULiminalTileData>>& InTiles)
	{
		for (const TObjectPtr<ULiminalTileData>& Tile : InTiles)
		{
			if (Tile && Tile->TileType == DesiredType)
			{
				Pool.Add(Tile.Get());
			}
		}
	};

	FilterPool(CorridorTiles);
	FilterPool(StandardRooms);
	FilterPool(HeroRooms);
	FilterPool(VerticalTiles);

	if (AirlockExtractionTile && AirlockExtractionTile->TileType == DesiredType)
	{
		Pool.Add(AirlockExtractionTile.Get());
	}

	if (Pool.Num() == 0)
	{
		return nullptr;
	}

	// Tirage pondéré
	float TotalWeight = 0.0f;
	for (ULiminalTileData* T : Pool)
	{
		TotalWeight += FMath::Max(0.01f, T->SelectionWeight);
	}

	float PickVal = InRandomStream.FRandRange(0.0f, TotalWeight);
	float Acc = 0.0f;
	for (ULiminalTileData* T : Pool)
	{
		Acc += FMath::Max(0.01f, T->SelectionWeight);
		if (PickVal <= Acc)
		{
			return T;
		}
	}

	return Pool[0];
}

ULiminalTileData* ULiminalTileSet::FindTileWithMatchingSockets(const TArray<FLiminalTileSocket>& RequiredSockets, FRandomStream& InRandomStream, float& OutRotationYaw) const
{
	OutRotationYaw = 0.0f;

	TArray<ULiminalTileData*> AllCandidates;
	for (const auto& T : CorridorTiles) { if (T) AllCandidates.Add(T.Get()); }
	for (const auto& T : StandardRooms) { if (T) AllCandidates.Add(T.Get()); }
	for (const auto& T : HeroRooms) { if (T) AllCandidates.Add(T.Get()); }

	if (AllCandidates.Num() == 0)
	{
		return nullptr;
	}

	const float Angles[4] = { 0.0f, 90.0f, 180.0f, 270.0f };

	TArray<TPair<ULiminalTileData*, float>> ValidMatches;

	for (ULiminalTileData* Tile : AllCandidates)
	{
		const int32 NumRotations = Tile->bAllowRotation ? 4 : 1;
		for (int32 r = 0; r < NumRotations; ++r)
		{
			const float Rot = Angles[r];
			const int32 DirShift = r; // 90 deg clockwise shift in directions (N->E->S->W)

			bool bAllMatch = true;
			for (const FLiminalTileSocket& Req : RequiredSockets)
			{
				if (!Req.bIsRequired)
				{
					continue;
				}

				// Trouver la direction locale équivalente
				int32 LocalDirInt = (static_cast<int32>(Req.Direction) - DirShift) % 4;
				if (LocalDirInt < 0) LocalDirInt += 4;
				const ELiminalSocketDirection LocalDir = static_cast<ELiminalSocketDirection>(LocalDirInt);

				ELiminalSocketType ActualType = ELiminalSocketType::ClosedWall;
				Tile->HasSocket(LocalDir, ActualType);

				if (ActualType != Req.SocketType)
				{
					bAllMatch = false;
					break;
				}
			}

			if (bAllMatch)
			{
				ValidMatches.Add(TPair<ULiminalTileData*, float>(Tile, Rot));
			}
		}
	}

	if (ValidMatches.Num() > 0)
	{
		const int32 PickIndex = InRandomStream.RandRange(0, ValidMatches.Num() - 1);
		OutRotationYaw = ValidMatches[PickIndex].Value;
		return ValidMatches[PickIndex].Key;
	}

	return nullptr;
}
