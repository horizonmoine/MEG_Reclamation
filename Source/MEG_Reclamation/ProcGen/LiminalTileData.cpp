#include "ProcGen/LiminalTileData.h"

ULiminalTileData::ULiminalTileData()
{
	TileId = FName("Tile_Default");
	DisplayName = FText::FromString(TEXT("Default Modular Tile"));
	UnitCellSize = 400.0f;
	CeilingHeight = 300.0f;
}

FPrimaryAssetId ULiminalTileData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FName("LiminalTileData"), GetFName());
}

bool ULiminalTileData::HasSocket(ELiminalSocketDirection Direction, ELiminalSocketType& OutType) const
{
	for (const FLiminalTileSocket& Sock : Sockets)
	{
		if (Sock.Direction == Direction)
		{
			OutType = Sock.SocketType;
			return true;
		}
	}
	OutType = ELiminalSocketType::ClosedWall;
	return false;
}
