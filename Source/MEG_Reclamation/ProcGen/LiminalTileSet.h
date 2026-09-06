#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/LiminalGameInstance.h"
#include "ProcGen/LiminalTileData.h"
#include "LiminalTileSet.generated.h"

/**
 * Collection de tuiles modulaires pre-fabriquees pour un biome specifique.
 */
UCLASS(BlueprintType)
class MEG_RECLAMATION_API ULiminalTileSet : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULiminalTileSet();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Biome")
	ELevelBiome TargetBiome = ELevelBiome::Level0_YellowLobby;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Corridors")
	TArray<TObjectPtr<ULiminalTileData>> CorridorTiles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Rooms")
	TArray<TObjectPtr<ULiminalTileData>> StandardRooms;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Special Rooms")
	TArray<TObjectPtr<ULiminalTileData>> HeroRooms;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Verticality")
	TArray<TObjectPtr<ULiminalTileData>> VerticalTiles;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Extraction")
	TObjectPtr<ULiminalTileData> AirlockExtractionTile;

	UFUNCTION(BlueprintPure, Category = "TileSet")
	ULiminalTileData* FindTileByType(ELiminalTileType DesiredType, FRandomStream& InRandomStream) const;

	UFUNCTION(BlueprintPure, Category = "TileSet")
	ULiminalTileData* FindTileWithMatchingSockets(const TArray<FLiminalTileSocket>& RequiredSockets, FRandomStream& InRandomStream, float& OutRotationYaw) const;
};
