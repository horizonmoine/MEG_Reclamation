#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProcGen/LiminalTileTypes.h"
#include "LiminalTileData.generated.h"

class UStaticMesh;

/**
 * Entree de maillage composant une tuile modulaire.
 */
USTRUCT(BlueprintType)
struct FLiminalTileMeshPart
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeshPart")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeshPart")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeshPart")
	bool bCastShadow = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MeshPart")
	bool bEnableCollision = true;
};

/**
 * DataAsset definissant une tuile modulaire pre-fabriquee.
 */
UCLASS(BlueprintType)
class MEG_RECLAMATION_API ULiminalTileData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	ULiminalTileData();

	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FName TileId = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	ELiminalTileType TileType = ELiminalTileType::CorridorStraight;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dimensions")
	FIntVector GridDimensions = FIntVector(1, 1, 1);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dimensions")
	float UnitCellSize = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Dimensions")
	float CeilingHeight = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Geometry")
	TArray<FLiminalTileMeshPart> MeshParts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Connectors")
	TArray<FLiminalTileSocket> Sockets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Props & Gameplay")
	TArray<FLiminalPropSocket> PropSockets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen Rules", meta = (ClampMin = "0.01"))
	float SelectionWeight = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen Rules")
	bool bAllowRotation = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen Rules")
	bool bIsUniquePerLevel = false;

	UFUNCTION(BlueprintPure, Category = "Tile")
	bool HasSocket(ELiminalSocketDirection Direction, ELiminalSocketType& OutType) const;
};
