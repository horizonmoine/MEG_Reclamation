#include "ProcGen/LiminalModularDungeonGenerator.h"

#include "Containers/Queue.h"
#include "Containers/Set.h"

ULiminalModularDungeonGenerator::ULiminalModularDungeonGenerator()
{
}

FLiminalModularDungeonLayout ULiminalModularDungeonGenerator::GenerateModularDungeon(
	int32 InSeed,
	int32 InGridWidth,
	int32 InGridHeight,
	int32 InTargetRoomCount,
	float InCellSize,
	const ULiminalTileSet* InTileSet)
{
	FLiminalModularDungeonLayout Result;
	Result.Seed = InSeed;

	FRandomStream Rnd(InSeed);

	const int32 Width = FMath::Clamp(InGridWidth, 12, 128);
	const int32 Height = FMath::Clamp(InGridHeight, 12, 128);
	const int32 TargetRooms = FMath::Clamp(InTargetRoomCount, 3, 48);
	const float CellSize = FMath::Max(200.0f, InCellSize);

	// Grille 2D d'occupation (-1 = vide, 0 = couloir, >0 = ID de salle)
	TArray<int32> Grid;
	Grid.Init(-1, Width * Height);

	auto GetCell = [&](int32 X, int32 Y) -> int32
	{
		if (X < 0 || X >= Width || Y < 0 || Y >= Height)
		{
			return -1;
		}
		return Grid[Y * Width + X];
	};

	auto SetCell = [&](int32 X, int32 Y, int32 Val)
	{
		if (X >= 0 && X < Width && Y >= 0 && Y < Height)
		{
			Grid[Y * Width + X] = Val;
		}
	};

	struct FRoomBounds
	{
		int32 Id;
		int32 X, Y;
		int32 W, H;
		int32 CenterX() const { return X + W / 2; }
		int32 CenterY() const { return Y + H / 2; }
	};

	TArray<FRoomBounds> Rooms;

	// 1. Placement des salles sans chevauchement
	for (int32 Attempt = 0; Attempt < TargetRooms * 25 && Rooms.Num() < TargetRooms; ++Attempt)
	{
		const int32 RW = Rnd.RandRange(2, 4);
		const int32 RH = Rnd.RandRange(2, 4);
		const int32 RX = Rnd.RandRange(1, Width - RW - 2);
		const int32 RY = Rnd.RandRange(1, Height - RH - 2);

		bool bOverlap = false;
		for (int32 CheckY = RY - 1; CheckY <= RY + RH; ++CheckY)
		{
			for (int32 CheckX = RX - 1; CheckX <= RX + RW; ++CheckX)
			{
				if (GetCell(CheckX, CheckY) != -1)
				{
					bOverlap = true;
					break;
				}
			}
			if (bOverlap) break;
		}

		if (!bOverlap)
		{
			const int32 NewId = Rooms.Num() + 1;
			FRoomBounds NewRoom{ NewId, RX, RY, RW, RH };
			Rooms.Add(NewRoom);

			for (int32 Y = RY; Y < RY + RH; ++Y)
			{
				for (int32 X = RX; X < RX + RW; ++X)
				{
					SetCell(X, Y, NewId);
				}
			}
		}
	}

	if (Rooms.Num() == 0)
	{
		// Fallback securise au moins 1 salle
		FRoomBounds DefRoom{ 1, Width / 2 - 1, Height / 2 - 1, 3, 3 };
		Rooms.Add(DefRoom);
		for (int32 Y = DefRoom.Y; Y < DefRoom.Y + DefRoom.H; ++Y)
		{
			for (int32 X = DefRoom.X; X < DefRoom.X + DefRoom.W; ++X)
			{
				SetCell(X, Y, 1);
			}
		}
	}

	// 2. Connexion par couloirs en L des salles consecutives
	for (int32 i = 0; i < Rooms.Num() - 1; ++i)
	{
		int32 CurX = Rooms[i].CenterX();
		int32 CurY = Rooms[i].CenterY();
		const int32 TargetX = Rooms[i + 1].CenterX();
		const int32 TargetY = Rooms[i + 1].CenterY();

		const bool bHorizontalFirst = (Rnd.RandRange(0, 1) == 0);

		auto StepH = [&]()
		{
			while (CurX != TargetX)
			{
				CurX += (TargetX > CurX) ? 1 : -1;
				if (GetCell(CurX, CurY) == -1)
				{
					SetCell(CurX, CurY, 0);
				}
			}
		};

		auto StepV = [&]()
		{
			while (CurY != TargetY)
			{
				CurY += (TargetY > CurY) ? 1 : -1;
				if (GetCell(CurX, CurY) == -1)
				{
					SetCell(CurX, CurY, 0);
				}
			}
		};

		if (bHorizontalFirst)
		{
			StepH();
			StepV();
		}
		else
		{
			StepV();
			StepH();
		}
	}

	// 3. Boucles additionnelles pour navigabilite
	for (int32 Loop = 0; Loop < FMath::Max(1, Rooms.Num() / 3); ++Loop)
	{
		const int32 IdxA = Rnd.RandRange(0, Rooms.Num() - 1);
		const int32 IdxB = Rnd.RandRange(0, Rooms.Num() - 1);
		if (IdxA != IdxB)
		{
			int32 CurX = Rooms[IdxA].CenterX();
			int32 CurY = Rooms[IdxA].CenterY();
			const int32 TargetX = Rooms[IdxB].CenterX();
			const int32 TargetY = Rooms[IdxB].CenterY();

			while (CurX != TargetX)
			{
				CurX += (TargetX > CurX) ? 1 : -1;
				if (GetCell(CurX, CurY) == -1) SetCell(CurX, CurY, 0);
			}
			while (CurY != TargetY)
			{
				CurY += (TargetY > CurY) ? 1 : -1;
				if (GetCell(CurX, CurY) == -1) SetCell(CurX, CurY, 0);
			}
		}
	}

	// 4. Point de spawn (premiere salle) et point d'extraction (salle la plus eloignee)
	Result.SpawnTileCoord = FIntVector(Rooms[0].CenterX(), Rooms[0].CenterY(), 0);

	int32 FarthestIndex = 0;
	float MaxDistSq = -1.0f;
	const FVector2D SpawnPos(static_cast<float>(Rooms[0].CenterX()), static_cast<float>(Rooms[0].CenterY()));

	for (int32 i = 1; i < Rooms.Num(); ++i)
	{
		const FVector2D RoomPos(static_cast<float>(Rooms[i].CenterX()), static_cast<float>(Rooms[i].CenterY()));
		const float DistSq = FVector2D::DistSquared(SpawnPos, RoomPos);
		if (DistSq > MaxDistSq)
		{
			MaxDistSq = DistSq;
			FarthestIndex = i;
		}
	}

	Result.ExtractionTileCoord = FIntVector(Rooms[FarthestIndex].CenterX(), Rooms[FarthestIndex].CenterY(), 0);

	// Hero room intermédiaire pour les puzzles
	if (Rooms.Num() >= 3)
	{
		const int32 HeroIdx = FMath::Clamp(Rooms.Num() / 2, 1, Rooms.Num() - 1);
		Result.HeroRoomCoords.Add(FIntVector(Rooms[HeroIdx].CenterX(), Rooms[HeroIdx].CenterY(), 0));
	}

	// 5. Instanciation des tuiles modulaires placees
	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			const int32 CellVal = GetCell(X, Y);
			if (CellVal == -1)
			{
				continue;
			}

			// Analyse du voisinage pour determiner la topologie de la tuile
			const bool bN = (GetCell(X, Y + 1) != -1);
			const bool bS = (GetCell(X, Y - 1) != -1);
			const bool bE = (GetCell(X + 1, Y) != -1);
			const bool bW = (GetCell(X - 1, Y) != -1);

			const int32 NeighborCount = (bN ? 1 : 0) + (bS ? 1 : 0) + (bE ? 1 : 0) + (bW ? 1 : 0);

			ELiminalTileType AssignedType = ELiminalTileType::CorridorStraight;
			float Yaw = 0.0f;

			if (CellVal > 0)
			{
				// Tuile appartenant a une salle
				if (X == Result.ExtractionTileCoord.X && Y == Result.ExtractionTileCoord.Y)
				{
					AssignedType = ELiminalTileType::MaintenanceAirlock;
				}
				else if (Result.HeroRoomCoords.Contains(FIntVector(X, Y, 0)))
				{
					AssignedType = ELiminalTileType::HeroRoom;
				}
				else
				{
					AssignedType = ELiminalTileType::RoomMedium;
				}
			}
			else
			{
				// Tuile de couloir selon voisins
				if (NeighborCount == 1)
				{
					AssignedType = ELiminalTileType::CorridorDeadEnd;
					if (bN) Yaw = 180.0f;
					else if (bS) Yaw = 0.0f;
					else if (bE) Yaw = 270.0f;
					else if (bW) Yaw = 90.0f;
				}
				else if (NeighborCount == 2)
				{
					if (bN && bS)
					{
						AssignedType = ELiminalTileType::CorridorStraight;
						Yaw = 0.0f;
					}
					else if (bE && bW)
					{
						AssignedType = ELiminalTileType::CorridorStraight;
						Yaw = 90.0f;
					}
					else
					{
						AssignedType = ELiminalTileType::CorridorCorner;
						if (bN && bE) Yaw = 0.0f;
						else if (bE && bS) Yaw = 90.0f;
						else if (bS && bW) Yaw = 180.0f;
						else if (bW && bN) Yaw = 270.0f;
					}
				}
				else if (NeighborCount == 3)
				{
					AssignedType = ELiminalTileType::CorridorTee;
					if (!bW) Yaw = 0.0f;
					else if (!bN) Yaw = 90.0f;
					else if (!bE) Yaw = 180.0f;
					else if (!bS) Yaw = 270.0f;
				}
				else
				{
					AssignedType = ELiminalTileType::CorridorCross;
					Yaw = 0.0f;
				}
			}

			FLiminalPlacedTile Placed;
			Placed.GridX = X;
			Placed.GridY = Y;
			Placed.GridZ = 0;
			Placed.RotationYaw = Yaw;
			Placed.TileType = AssignedType;
			Placed.WorldLocation = FVector(
				(X - Width * 0.5f) * CellSize,
				(Y - Height * 0.5f) * CellSize,
				0.0f
			);

			// Generation diegetique des sockets de props (neon, loot, decals)
			if (AssignedType == ELiminalTileType::RoomMedium || AssignedType == ELiminalTileType::HeroRoom)
			{
				FLiminalPropSocket LightSocket;
				LightSocket.SocketId = FName("CeilingLight_Center");
				LightSocket.Category = ELiminalSpawnSocketCategory::CeilingLight;
				LightSocket.RelativeTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 280.0f));
				Placed.ResolvedPropSockets.Add(LightSocket);

				if (Rnd.FRand() < 0.45f)
				{
					FLiminalPropSocket LootSocket;
					LootSocket.SocketId = FName("FloorLoot_Center");
					LootSocket.Category = ELiminalSpawnSocketCategory::FloorLoot;
					LootSocket.RelativeTransform = FTransform(FRotator::ZeroRotator, FVector(Rnd.FRandRange(-80.0f, 80.0f), Rnd.FRandRange(-80.0f, 80.0f), 20.0f));
					Placed.ResolvedPropSockets.Add(LootSocket);
				}
			}
			else if (AssignedType == ELiminalTileType::CorridorStraight && Rnd.FRand() < 0.35f)
			{
				FLiminalPropSocket CorridorLight;
				CorridorLight.SocketId = FName("CeilingLight_Corridor");
				CorridorLight.Category = ELiminalSpawnSocketCategory::CeilingLight;
				CorridorLight.RelativeTransform = FTransform(FRotator::ZeroRotator, FVector(0.0f, 0.0f, 280.0f));
				Placed.ResolvedPropSockets.Add(CorridorLight);
			}

			Result.PlacedTiles.Add(Placed);

			for (const FLiminalPropSocket& S : Placed.ResolvedPropSockets)
			{
				FLiminalPropSocket WorldSocket = S;
				WorldSocket.RelativeTransform = S.RelativeTransform * FTransform(FRotator(0.0f, Placed.RotationYaw, 0.0f), Placed.WorldLocation);
				Result.AllWorldPropSockets.Add(WorldSocket);
			}
		}
	}

	return Result;
}

bool ULiminalModularDungeonGenerator::IsDungeonFullyConnected(const FLiminalModularDungeonLayout& InLayout)
{
	if (InLayout.PlacedTiles.Num() == 0)
	{
		return false;
	}

	TSet<FIntPoint> AllCells;
	for (const FLiminalPlacedTile& T : InLayout.PlacedTiles)
	{
		AllCells.Add(FIntPoint(T.GridX, T.GridY));
	}

	const FIntPoint Start(InLayout.SpawnTileCoord.X, InLayout.SpawnTileCoord.Y);
	if (!AllCells.Contains(Start))
	{
		return false;
	}

	TSet<FIntPoint> Visited;
	TQueue<FIntPoint> Queue;

	Visited.Add(Start);
	Queue.Enqueue(Start);

	const FIntPoint Offsets[4] = {
		FIntPoint(0, 1),
		FIntPoint(0, -1),
		FIntPoint(1, 0),
		FIntPoint(-1, 0)
	};

	while (!Queue.IsEmpty())
	{
		FIntPoint Current;
		Queue.Dequeue(Current);

		for (const FIntPoint& Off : Offsets)
		{
			const FIntPoint Next = Current + Off;
			if (AllCells.Contains(Next) && !Visited.Contains(Next))
			{
				Visited.Add(Next);
				Queue.Enqueue(Next);
			}
		}
	}

	return (Visited.Num() == AllCells.Num());
}
