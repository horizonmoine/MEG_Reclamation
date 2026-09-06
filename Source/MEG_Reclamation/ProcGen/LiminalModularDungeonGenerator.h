#pragma once

#include "CoreMinimal.h"
#include "ProcGen/LiminalTileTypes.h"
#include "ProcGen/LiminalTileSet.h"
#include "LiminalModularDungeonGenerator.generated.h"

/**
 * Resultat de la generation procedurale modulaire.
 */
USTRUCT(BlueprintType)
struct FLiminalModularDungeonLayout
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 Seed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FLiminalPlacedTile> PlacedTiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FIntVector SpawnTileCoord = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FIntVector ExtractionTileCoord = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FIntVector> HeroRoomCoords;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FLiminalPropSocket> AllWorldPropSockets;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FIntVector CentralHubCoord = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FIntVector SecretRoomCoord = FIntVector::ZeroValue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TArray<FIntVector> DeadEndAmbushCoords;
};

/**
 * Generateur d'assemblage modulaire pre-fabrique (Tile Prefab Grammar / Wave Collapse approche).
 * Totalement deterministe et server-authoritative.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalModularDungeonGenerator : public UObject
{
	GENERATED_BODY()

public:
	ULiminalModularDungeonGenerator();

	/**
	 * Genere un complexe modulaire connecte a partir d'une seed et d'un TileSet.
	 */
	UFUNCTION(BlueprintCallable, Category = "ProcGen")
	static FLiminalModularDungeonLayout GenerateModularDungeon(
		int32 InSeed,
		int32 InGridWidth,
		int32 InGridHeight,
		int32 InTargetRoomCount,
		float InCellSize,
		const ULiminalTileSet* InTileSet
	);

	/**
	 * Verifie par parcours en largeur (BFS) si chaque tuile placee est accessible depuis le point de spawn.
	 */
	static bool IsDungeonFullyConnected(const FLiminalModularDungeonLayout& InLayout);
};
