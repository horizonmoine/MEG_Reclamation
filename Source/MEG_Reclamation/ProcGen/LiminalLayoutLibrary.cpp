#include "ProcGen/LiminalLayoutLibrary.h"

EProcCellType FGeneratedLayout::GetCell(int32 X, int32 Y) const
{
	const int32 Index = Y * Width + X;
	return Cells.IsValidIndex(Index) ? Cells[Index] : EProcCellType::Wall;
}

bool FGeneratedLayout::IsFloor(int32 X, int32 Y) const
{
	return IsFloorIndex(Y * Width + X);
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

bool FGeneratedLayout::IsEveryRoomConnected() const
{
	if (Rooms.Num() == 0)
	{
		return false;
	}

	TArray<bool> Visited;
	Visited.Init(false, Cells.Num());

	TArray<int32> OpenQueue;
	const int32 StartIndex = Rooms[0].CenterY * Width + Rooms[0].CenterX;
	Visited[StartIndex] = true;
	OpenQueue.Add(StartIndex);

	while (OpenQueue.Num() > 0)
	{
		const int32 Current = OpenQueue.Pop(EAllowShrinking::No);
		const int32 Cx = Current % Width;
		const int32 Cy = Current / Width;

		const int32 Neighbors[4][2] = { { Cx + 1, Cy }, { Cx - 1, Cy }, { Cx, Cy + 1 }, { Cx, Cy - 1 } };
		for (const auto& N : Neighbors)
		{
			const int32 Nx = N[0];
			const int32 Ny = N[1];
			if (Nx < 0 || Ny < 0 || Nx >= Width || Ny >= Height)
			{
				continue;
			}
			const int32 NeighborIndex = Ny * Width + Nx;
			if (!Visited[NeighborIndex] && IsFloorIndex(NeighborIndex))
			{
				Visited[NeighborIndex] = true;
				OpenQueue.Add(NeighborIndex);
			}
		}
	}

	for (const FProcRoom& Room : Rooms)
	{
		if (!Visited[Room.CenterY * Width + Room.CenterX])
		{
			return false;
		}
	}
	return true;
}

FGeneratedLayout FLiminalLayoutBuilder::Generate(int32 Seed, int32 Width, int32 Height,
	int32 RoomCount, int32 MinRoomSize, int32 MaxRoomSize, float ExtraLoopChance)
{
	FGeneratedLayout FinalLayout;

	for (int32 Attempt = 0; Attempt < 10; ++Attempt)
	{
		FRandomStream Stream(Seed + Attempt * 7919);
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
