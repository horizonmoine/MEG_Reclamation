#pragma once

#include "CoreMinimal.h"
#include "ProcGen/LiminalRoomTemplate.h"

enum class EProcCellType : uint8
{
	Wall = 0,
	Floor = 1,
	Corridor = 2,
	Doorway = 3,
	Pillar = 4,
	StairsUp = 5,
	StairsDown = 6,
	ElevatorShaft = 7,
	WaterHazard = 8,
	Ventilation = 9,
	HeroPuzzle = 10
};

struct FProcRoom
{
	int32 OriginX = 0;
	int32 OriginY = 0;
	int32 SizeX = 0;
	int32 SizeY = 0;
	int32 CenterX = 0;
	int32 CenterY = 0;
	bool bIsHeroRoom = false;
	bool bHasStairs = false;
	bool bHasWaterHazard = false;
	ERoomType RoomType = ERoomType::MediumRoom;
	float ElevationZ = 0.0f;
	bool bIsPowerSealed = false;
};

struct FGeneratedLayout
{
	int32 Width = 0;
	int32 Height = 0;
	TArray<EProcCellType> Cells;
	TArray<FProcRoom> Rooms;
	uint32 Hash = 0;

	EProcCellType GetCell(int32 X, int32 Y) const;
	bool IsFloor(int32 X, int32 Y) const;
	bool IsFloorIndex(int32 CellIndex) const;
	bool IsWall(int32 X, int32 Y) const;
	bool IsCorridor(int32 X, int32 Y) const;
	bool IsDoorway(int32 X, int32 Y) const;
	void SetFloor(int32 X, int32 Y);
	void SetCell(int32 X, int32 Y, EProcCellType CellType);
	uint32 ComputeHash() const;
	bool IsEveryRoomConnected() const;
};

/**
 * Generation deterministe d'un etage liminal : salles reliees par couloirs en L.
 * Meme Seed + memes parametres => layout bit-a-bit identique (hash stable).
 */
class FLiminalLayoutBuilder
{
public:
	static FGeneratedLayout Generate(int32 Seed, int32 Width, int32 Height,
		int32 RoomCount, int32 MinRoomSize, int32 MaxRoomSize, float ExtraLoopChance);

private:
	static FGeneratedLayout BuildAttempt(FRandomStream& Stream, int32 Width, int32 Height,
		int32 RoomCount, int32 MinRoomSize, int32 MaxRoomSize, float ExtraLoopChance);

	static void CarveRect(FGeneratedLayout& Layout, const FProcRoom& Room);
	static void CarveCorridor(FGeneratedLayout& Layout, const FProcRoom& From,
		const FProcRoom& To, FRandomStream& Stream);
	static bool RoomsOverlap(const FProcRoom& A, const FProcRoom& B, int32 Padding);
};
