#include "ProcGen/LiminalStreamingManager.h"
#include "AI/LiminalEntity.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

ALiminalStreamingManager::ALiminalStreamingManager()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 0.25f;
	bReplicates = false; // Gere localement cote client pour le rendu, et cote serveur pour l'IA
}

void ALiminalStreamingManager::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalStreamingManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	TimeSinceLastUpdate += DeltaSeconds;
	if (TimeSinceLastUpdate >= UpdateIntervalSeconds)
	{
		TimeSinceLastUpdate = 0.0f;
		UpdatePlayerStreaming();
	}
}

void ALiminalStreamingManager::InitializeGrid(int32 GridWidth, int32 GridHeight, float CellSize, int32 InChunkSize)
{
	ChunkCellDimension = FMath::Clamp(InChunkSize, 2, 32);
	ScaledCellSize = CellSize;
	ChunkWorldSize = ChunkCellDimension * ScaledCellSize;

	Chunks.Empty();

	const int32 ChunksX = FMath::DivideAndRoundUp(GridWidth, ChunkCellDimension);
	const int32 ChunksY = FMath::DivideAndRoundUp(GridHeight, ChunkCellDimension);

	for (int32 Cy = 0; Cy < ChunksY; ++Cy)
	{
		for (int32 Cx = 0; Cx < ChunksX; ++Cx)
		{
			FIntPoint Coord(Cx, Cy);
			FProcSpatialChunk Chunk;
			Chunk.ChunkCoord = Coord;

			const FVector Min(Cx * ChunkWorldSize, Cy * ChunkWorldSize, -500.0f);
			const FVector Max((Cx + 1) * ChunkWorldSize, (Cy + 1) * ChunkWorldSize, 2000.0f);
			Chunk.Bounds = FBox(Min, Max);
			Chunk.bIsActive = false;

			Chunks.Add(Coord, Chunk);
		}
	}
}

FIntPoint ALiminalStreamingManager::WorldToChunk(const FVector& WorldLocation) const
{
	if (ChunkWorldSize <= 0.0f)
	{
		return FIntPoint::ZeroValue;
	}

	const int32 Cx = FMath::FloorToInt(WorldLocation.X / ChunkWorldSize);
	const int32 Cy = FMath::FloorToInt(WorldLocation.Y / ChunkWorldSize);
	return FIntPoint(Cx, Cy);
}

void ALiminalStreamingManager::RegisterActor(AActor* InActor)
{
	if (!InActor)
	{
		return;
	}

	const FIntPoint Coord = WorldToChunk(InActor->GetActorLocation());
	if (FProcSpatialChunk* Chunk = Chunks.Find(Coord))
	{
		Chunk->ResidentActors.Add(InActor);
	}
}

void ALiminalStreamingManager::RegisterEntity(ALiminalEntity* InEntity)
{
	if (InEntity)
	{
		RegisteredEntities.AddUnique(InEntity);
	}
}

void ALiminalStreamingManager::ForceActivateAll()
{
	ActiveChunkCount = Chunks.Num();
	for (auto& Pair : Chunks)
	{
		Pair.Value.bIsActive = true;
		for (auto& ActorPtr : Pair.Value.ResidentActors)
		{
			if (AActor* Act = ActorPtr.Get())
			{
				Act->SetActorHiddenInGame(false);
				Act->SetActorEnableCollision(true);
				Act->SetActorTickEnabled(true);
			}
		}
	}
}

void ALiminalStreamingManager::UpdatePlayerStreaming()
{
	UWorld* World = GetWorld();
	if (!World || Chunks.Num() == 0)
	{
		return;
	}

	TArray<FVector> PlayerPositions;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (APawn* Pawn = PC->GetPawn())
			{
				PlayerPositions.Add(Pawn->GetActorLocation());
			}
		}
	}

	if (PlayerPositions.Num() == 0)
	{
		return;
	}

	// Identifier les chunks actifs selon la proximite de tous les joueurs
	TSet<FIntPoint> DesiredActiveChunks;
	for (const FVector& PlayerPos : PlayerPositions)
	{
		const FIntPoint CenterChunk = WorldToChunk(PlayerPos);

		for (int32 Dy = -ActiveChunkRadius; Dy <= ActiveChunkRadius; ++Dy)
		{
			for (int32 Dx = -ActiveChunkRadius; Dx <= ActiveChunkRadius; ++Dx)
			{
				DesiredActiveChunks.Add(CenterChunk + FIntPoint(Dx, Dy));
			}
		}
	}

	ActiveChunkCount = 0;

	for (auto& Pair : Chunks)
	{
		const bool bShouldBeActive = DesiredActiveChunks.Contains(Pair.Key);

		if (Pair.Value.bIsActive != bShouldBeActive)
		{
			Pair.Value.bIsActive = bShouldBeActive;

			for (auto& ActorPtr : Pair.Value.ResidentActors)
			{
				if (AActor* Act = ActorPtr.Get())
				{
					Act->SetActorHiddenInGame(!bShouldBeActive);
					Act->SetActorTickEnabled(bShouldBeActive);
				}
			}
		}

		if (Pair.Value.bIsActive)
		{
			ActiveChunkCount++;
		}
	}

	// Gestion du sommeil cognitif des entites IA eloignees
	const float SleepDistanceSq = FMath::Square(ChunkWorldSize * (ActiveChunkRadius + 1));
	for (auto& EntityPtr : RegisteredEntities)
	{
		if (ALiminalEntity* Entity = EntityPtr.Get())
		{
			float MinDistSq = FLT_MAX;
			const FVector EntityLoc = Entity->GetActorLocation();

			for (const FVector& PPos : PlayerPositions)
			{
				const float DistSq = FVector::DistSquared2D(EntityLoc, PPos);
				if (DistSq < MinDistSq)
				{
					MinDistSq = DistSq;
				}
			}

			const bool bFarFromPlayers = (MinDistSq > SleepDistanceSq);
			Entity->SetActorTickInterval(bFarFromPlayers ? 0.3f : 0.0f);
		}
	}
}
