#include "ProcGen/LiminalLayoutLibrary.h"

EProcCellType FGeneratedLayout::GetCell(int32 X, int32 Y) const
{
	if (X < 0 || Y < 0 || X >= Width || Y >= Height) return EProcCellType::Wall;
	const int32 Index = Y * Width + X;
	return Cells.IsValidIndex(Index) ? Cells[Index] : EProcCellType::Wall;
}

bool FGeneratedLayout::IsFloor(int32 X, int32 Y) const
{
	return X >= 0 && Y >= 0 && X < Width && Y < Height && IsFloorIndex(Y * Width + X);
}

bool FGeneratedLayout::IsFloorIndex(int32 CellIndex) const
{
	if (!Cells.IsValidIndex(CellIndex))
	{
		return false;
	}
	const EProcCellType Cell = Cells[CellIndex];
	return Cell == EProcCellType::Floor ||
	       Cell == EProcCellType::Corridor ||
	       Cell == EProcCellType::Doorway ||
	       Cell == EProcCellType::StairsUp ||
	       Cell == EProcCellType::StairsDown ||
	       Cell == EProcCellType::WaterHazard ||
	       Cell == EProcCellType::HeroPuzzle;
}

bool FGeneratedLayout::IsWall(int32 X, int32 Y) const
{
	const EProcCellType Cell = GetCell(X, Y);
	return Cell == EProcCellType::Wall || Cell == EProcCellType::Pillar;
}

bool FGeneratedLayout::IsCorridor(int32 X, int32 Y) const
{
	return GetCell(X, Y) == EProcCellType::Corridor;
}

bool FGeneratedLayout::IsDoorway(int32 X, int32 Y) const
{
	return GetCell(X, Y) == EProcCellType::Doorway;
}

void FGeneratedLayout::SetFloor(int32 X, int32 Y)
{
	SetCell(X, Y, EProcCellType::Floor);
}

void FGeneratedLayout::SetCell(int32 X, int32 Y, EProcCellType CellType)
{
	if (X >= 1 && X < Width - 1 && Y >= 1 && Y < Height - 1)
	{
		Cells[Y * Width + X] = CellType;
	}
}

uint32 FGeneratedLayout::ComputeHash() const
{
	uint32 Running = 2166136261u;
	auto Mix = [&Running](uint8 Byte)
	{
		Running ^= Byte;
		Running *= 16777619u;
	};

	Mix(static_cast<uint8>(Width & 0xFF));
	Mix(static_cast<uint8>((Width >> 8) & 0xFF));
	Mix(static_cast<uint8>(Height & 0xFF));
	Mix(static_cast<uint8>((Height >> 8) & 0xFF));

	for (EProcCellType Cell : Cells)
	{
		Mix(static_cast<uint8>(Cell));
	}

	Mix(static_cast<uint8>(Rooms.Num()));
	return Running;
}

TArray<int32> FGeneratedLayout::GetRoomPathLengths() const
{
	TArray<int32> RoomDistances;
	RoomDistances.Init(INDEX_NONE, Rooms.Num());
	if (Width <= 0 || Height <= 0 || static_cast<int64>(Width) * Height != Cells.Num() || Rooms.IsEmpty()) return RoomDistances;
	const FProcRoom& Start = Rooms[0];
	if (!IsFloor(Start.CenterX, Start.CenterY)) return RoomDistances;
	TArray<int32> Distances;
	Distances.Init(INDEX_NONE, Cells.Num());
	TArray<int32> Queue;
	const int32 StartIndex = Start.CenterY * Width + Start.CenterX;
	Queue.Add(StartIndex);
	Distances[StartIndex] = 0;
	for (int32 Head = 0; Head < Queue.Num(); ++Head)
	{
		const int32 Current = Queue[Head];
		const int32 X = Current % Width;
		const int32 Y = Current / Width;
		const FIntPoint Neighbors[] = {{X + 1, Y}, {X - 1, Y}, {X, Y + 1}, {X, Y - 1}};
		for (const FIntPoint& Neighbor : Neighbors)
		{
			if (!IsFloor(Neighbor.X, Neighbor.Y)) continue;
			const int32 Index = Neighbor.Y * Width + Neighbor.X;
			if (Distances[Index] != INDEX_NONE) continue;
			Distances[Index] = Distances[Current] + 1;
			Queue.Add(Index);
		}
	}
	for (int32 Index = 0; Index < Rooms.Num(); ++Index)
	{
		const FProcRoom& Room = Rooms[Index];
		if (IsFloor(Room.CenterX, Room.CenterY)) RoomDistances[Index] = Distances[Room.CenterY * Width + Room.CenterX];
	}
	return RoomDistances;
}

bool FGeneratedLayout::IsEveryRoomConnected() const
{
	const TArray<int32> Distances = GetRoomPathLengths();
	return !Distances.IsEmpty() && !Distances.Contains(INDEX_NONE);
}

bool FLiminalLayoutBuilder::TryGenerateExpedition(int32 Seed, int32 Width, int32 Height,
	int32 RoomCount, int32 MinRoomSize, int32 MaxRoomSize, float ExtraLoopChance,
	float CellSize, FGeneratedLayout& OutLayout, float MinimumDistanceCm)
{
	if (!FMath::IsFinite(CellSize) || CellSize <= 0.0f || !FMath::IsFinite(MinimumDistanceCm) || MinimumDistanceCm < 0.0f) return false;
	for (uint32 Attempt = 0; Attempt <= 20; ++Attempt)
	{
		FGeneratedLayout Candidate = Generate(static_cast<int32>(static_cast<uint32>(Seed) + Attempt * 1337u),
			Width, Height, RoomCount, MinRoomSize, MaxRoomSize, ExtraLoopChance);
		if (Attempt == 20 && Width >= 8 && Height >= 8 && Width <= 512 && Height <= 512)
		{
			// Last resort: a reproducible connected corridor, never the last rejected random layout.
			Candidate = FGeneratedLayout();
			Candidate.Width = Width;
			Candidate.Height = Height;
			Candidate.Cells.Init(EProcCellType::Wall, Width * Height);
			FProcRoom Spawn;
			Spawn.OriginX = Spawn.OriginY = 1;
			Spawn.SizeX = Spawn.SizeY = 3;
			Spawn.CenterX = Spawn.CenterY = 2;
			FProcRoom Exit = Spawn;
			Exit.OriginX = Width - 4;
			Exit.OriginY = Height - 4;
			Exit.CenterX = Width - 3;
			Exit.CenterY = Height - 3;
			Exit.bIsHeroRoom = true;
			Candidate.Rooms = {Spawn, Exit};
			CarveRect(Candidate, Spawn);
			CarveRect(Candidate, Exit);
			FRandomStream FallbackStream(Seed);
			CarveCorridor(Candidate, Spawn, Exit, FallbackStream);
			Candidate.Hash = Candidate.ComputeHash();
		}
		const TArray<int32> Distances = Candidate.GetRoomPathLengths();
		if (Distances.Num() < 2 || Distances.Contains(INDEX_NONE)) continue;
		int32 BestDistance = INDEX_NONE;
		for (int32 Index = 1; Index < Candidate.Rooms.Num(); ++Index)
		{
			const FProcRoom& Room = Candidate.Rooms[Index];
			const FProcRoom& Spawn = Candidate.Rooms[0];
			const FVector2D Delta(Room.CenterX - Spawn.CenterX, Room.CenterY - Spawn.CenterY);
			// Also enforce a Euclidean lower bound: cutting diagonally across rooms cannot defeat the minimum.
			if (Distances[Index] * CellSize >= MinimumDistanceCm && Delta.Size() * CellSize >= MinimumDistanceCm && Distances[Index] > BestDistance)
			{
				BestDistance = Distances[Index];
				Candidate.ExtractionRoomIndex = Index;
			}
		}
		if (Candidate.ExtractionRoomIndex != INDEX_NONE)
		{
			Candidate.ExtractionPathLengthCm = BestDistance * CellSize;
			OutLayout = MoveTemp(Candidate);
			return true;
		}
	}
	return false;
}
FGeneratedLayout FLiminalLayoutBuilder::Generate(int32 Seed, int32 Width, int32 Height,
	int32 RoomCount, int32 MinRoomSize, int32 MaxRoomSize, float ExtraLoopChance)
{
	FGeneratedLayout FinalLayout;
	if (Width < 5 || Height < 5 || Width > 512 || Height > 512 || RoomCount < 1 || RoomCount > 512 || MinRoomSize < 1 || MaxRoomSize < MinRoomSize || MaxRoomSize > FMath::Min(Width, Height) - 3 || !FMath::IsFinite(ExtraLoopChance)) return FinalLayout;

	for (int32 Attempt = 0; Attempt < 10; ++Attempt)
	{
		FRandomStream Stream(static_cast<int32>(static_cast<uint32>(Seed) + static_cast<uint32>(Attempt) * 7919u));
		FinalLayout = BuildAttempt(Stream, Width, Height, RoomCount, MinRoomSize, MaxRoomSize, ExtraLoopChance);
		FinalLayout.Hash = FinalLayout.ComputeHash();

		if (FinalLayout.IsEveryRoomConnected())
		{
			break;
		}
	}

	return FinalLayout;
}

FGeneratedLayout FLiminalLayoutBuilder::BuildAttempt(FRandomStream& Stream, int32 Width, int32 Height,
	int32 RoomCount, int32 MinRoomSize, int32 MaxRoomSize, float ExtraLoopChance)
{
	FGeneratedLayout Layout;
	Layout.Width = Width;
	Layout.Height = Height;
	Layout.Cells.Init(EProcCellType::Wall, Width * Height);

	int32 Tries = 0;
	const int32 MaxTries = FMath::Max(300, RoomCount * 40);
	while (Layout.Rooms.Num() < RoomCount && Tries < MaxTries)
	{
		++Tries;

		FProcRoom Room;
		Room.SizeX = Stream.RandRange(MinRoomSize, MaxRoomSize);
		Room.SizeY = Stream.RandRange(MinRoomSize, MaxRoomSize);
		Room.OriginX = Stream.RandRange(1, FMath::Max(1, Width - Room.SizeX - 2));
		Room.OriginY = Stream.RandRange(1, FMath::Max(1, Height - Room.SizeY - 2));
		Room.CenterX = Room.OriginX + Room.SizeX / 2;
		Room.CenterY = Room.OriginY + Room.SizeY / 2;

		bool bOverlaps = false;
		for (const FProcRoom& Existing : Layout.Rooms)
		{
			if (RoomsOverlap(Room, Existing, 1))
			{
				bOverlaps = true;
				break;
			}
		}
		if (bOverlaps)
		{
			continue;
		}

		CarveRect(Layout, Room);
		Layout.Rooms.Add(Room);
	}

	if (Layout.Rooms.Num() < 2)
	{
		return Layout;
	}

	const int32 RoomNum = Layout.Rooms.Num();

	TArray<bool> InTree;
	InTree.Init(false, RoomNum);
	InTree[0] = true;
	int32 TreeCount = 1;

	TSet<uint64> TreeEdges;
	auto EdgeKey = [](int32 A, int32 B)
	{
		return (static_cast<uint64>(FMath::Min(A, B)) << 32) | static_cast<uint64>(FMath::Max(A, B));
	};

	while (TreeCount < RoomNum)
	{
		int32 BestFrom = INDEX_NONE;
		int32 BestTo = INDEX_NONE;
		int64 BestDist = TNumericLimits<int64>::Max();

		for (int32 A = 0; A < RoomNum; ++A)
		{
			if (!InTree[A])
			{
				continue;
			}
			for (int32 B = 0; B < RoomNum; ++B)
			{
				if (InTree[B])
				{
					continue;
				}
				const int64 Dist = FMath::Abs(Layout.Rooms[A].CenterX - Layout.Rooms[B].CenterX) +
					FMath::Abs(Layout.Rooms[A].CenterY - Layout.Rooms[B].CenterY);
				if (Dist < BestDist)
				{
					BestDist = Dist;
					BestFrom = A;
					BestTo = B;
				}
			}
		}

		if (BestTo == INDEX_NONE)
		{
			break;
		}

		CarveCorridor(Layout, Layout.Rooms[BestFrom], Layout.Rooms[BestTo], Stream);
		InTree[BestTo] = true;
		TreeEdges.Add(EdgeKey(BestFrom, BestTo));
		++TreeCount;
	}

	for (int32 A = 0; A < RoomNum; ++A)
	{
		for (int32 B = A + 1; B < RoomNum; ++B)
		{
			if (TreeEdges.Contains(EdgeKey(A, B)))
			{
				continue;
			}
			const int64 Dist = FMath::Abs(Layout.Rooms[A].CenterX - Layout.Rooms[B].CenterX) +
				FMath::Abs(Layout.Rooms[A].CenterY - Layout.Rooms[B].CenterY);
			if (Dist <= 14 && Stream.FRand() < ExtraLoopChance)
			{
				CarveCorridor(Layout, Layout.Rooms[A], Layout.Rooms[B], Stream);
			}
		}
	}

	// Identifier et marquer la Hero Room (la plus spacieuse pour les puzzles d'extraction/centraux)
	if (Layout.Rooms.Num() > 0)
	{
		int32 LargestIndex = 0;
		int32 MaxArea = 0;
		for (int32 i = 0; i < Layout.Rooms.Num(); ++i)
		{
			const int32 Area = Layout.Rooms[i].SizeX * Layout.Rooms[i].SizeY;
			if (Area > MaxArea)
			{
				MaxArea = Area;
				LargestIndex = i;
			}
		}
		Layout.Rooms[LargestIndex].bIsHeroRoom = true;
	}

	bool bChanged = true;
	while (bChanged)
	{
		bChanged = false;
		for (int32 Y = 1; Y < Height - 1; ++Y)
		{
			for (int32 X = 1; X < Width - 1; ++X)
			{
				if (Layout.GetCell(X, Y) == EProcCellType::Corridor)
				{
					int32 WallCount = 0;
					if (Layout.GetCell(X + 1, Y) == EProcCellType::Wall) WallCount++;
					if (Layout.GetCell(X - 1, Y) == EProcCellType::Wall) WallCount++;
					if (Layout.GetCell(X, Y + 1) == EProcCellType::Wall) WallCount++;
					if (Layout.GetCell(X, Y - 1) == EProcCellType::Wall) WallCount++;

					if (WallCount >= 3)
					{
						Layout.SetCell(X, Y, EProcCellType::Wall);
						bChanged = true;
					}
				}
			}
		}
	}

	return Layout;
}

void FLiminalLayoutBuilder::CarveRect(FGeneratedLayout& Layout, const FProcRoom& Room)
{
	for (int32 Y = Room.OriginY; Y < Room.OriginY + Room.SizeY; ++Y)
	{
		for (int32 X = Room.OriginX; X < Room.OriginX + Room.SizeX; ++X)
		{
			Layout.SetFloor(X, Y);
		}
	}
}

void FLiminalLayoutBuilder::CarveCorridor(FGeneratedLayout& Layout, const FProcRoom& From,
	const FProcRoom& To, FRandomStream& Stream)
{
	const bool bHorizontalFirst = Stream.FRand() < 0.5f;

	auto CarvePoint = [&](int32 X, int32 Y)
	{
		if (X >= 1 && X < Layout.Width - 1 && Y >= 1 && Y < Layout.Height - 1)
		{
			const EProcCellType Current = Layout.GetCell(X, Y);
			if (Current == EProcCellType::Wall)
			{
				Layout.SetCell(X, Y, EProcCellType::Corridor);
			}
		}
	};

	auto CarveHorizontal = [&](int32 Y)
	{
		const int32 MinX = FMath::Min(From.CenterX, To.CenterX);
		const int32 MaxX = FMath::Max(From.CenterX, To.CenterX);
		for (int32 X = MinX; X <= MaxX; ++X)
		{
			CarvePoint(X, Y);
		}
	};
	auto CarveVertical = [&](int32 X)
	{
		const int32 MinY = FMath::Min(From.CenterY, To.CenterY);
		const int32 MaxY = FMath::Max(From.CenterY, To.CenterY);
		for (int32 Y = MinY; Y <= MaxY; ++Y)
		{
			CarvePoint(X, Y);
		}
	};

	if (bHorizontalFirst)
	{
		CarveHorizontal(From.CenterY);
		CarveVertical(To.CenterX);
	}
	else
	{
		CarveVertical(From.CenterX);
		CarveHorizontal(To.CenterY);
	}
}

bool FLiminalLayoutBuilder::RoomsOverlap(const FProcRoom& A, const FProcRoom& B, int32 Padding)
{
	return !(A.OriginX + A.SizeX + Padding <= B.OriginX ||
		B.OriginX + B.SizeX + Padding <= A.OriginX ||
		A.OriginY + A.SizeY + Padding <= B.OriginY ||
		B.OriginY + B.SizeY + Padding <= A.OriginY);
}
