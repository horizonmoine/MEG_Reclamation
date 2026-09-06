#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalStreamingManager.generated.h"

class UInstancedStaticMeshComponent;
class ALiminalEntity;

USTRUCT(BlueprintType)
struct FProcSpatialChunk
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Streaming")
	FIntPoint ChunkCoord = FIntPoint::ZeroValue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Streaming")
	FBox Bounds = FBox(ForceInit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Streaming")
	bool bIsActive = false;

	TArray<int32> InstanceIndices;
	TArray<TWeakObjectPtr<AActor>> ResidentActors;
};

/**
 * Gestionnaire de streaming spatial pour les cartes procgen 64x64+.
 * Découpe la carte en chunks spatiaux et culling dynamique
 * (rendu, collisions et veille IA) selon la distance aux joueurs.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalStreamingManager : public AActor
{
	GENERATED_BODY()

public:
	ALiminalStreamingManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Initialise le partitionnement d'une carte générée */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Streaming")
	void InitializeGrid(int32 GridWidth, int32 GridHeight, float CellSize, int32 InChunkSize = 8);

	/** Enregistre un acteur de gameplay dans son chunk spatial */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Streaming")
	void RegisterActor(AActor* InActor);

	/** Enregistre une entité IA pour la gestion de sommeil cognitif */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Streaming")
	void RegisterEntity(ALiminalEntity* InEntity);

	/** Force l'activation de tous les chunks (debug ou petites cartes) */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Streaming")
	void ForceActivateAll();

	UFUNCTION(BlueprintPure, Category = "Liminal|Streaming")
	int32 GetActiveChunkCount() const { return ActiveChunkCount; }

protected:
	void UpdatePlayerStreaming();
	FIntPoint WorldToChunk(const FVector& WorldLocation) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Streaming", meta = (ClampMin = "2", ClampMax = "32"))
	int32 ChunkCellDimension = 8;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Streaming", meta = (ClampMin = "1", ClampMax = "5"))
	int32 ActiveChunkRadius = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Streaming")
	float UpdateIntervalSeconds = 0.5f;

	float TimeSinceLastUpdate = 0.0f;
	float ScaledCellSize = 400.0f;
	float ChunkWorldSize = 3200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liminal|Streaming")
	int32 ActiveChunkCount = 0;

	TMap<FIntPoint, FProcSpatialChunk> Chunks;
	TArray<TWeakObjectPtr<ALiminalEntity>> RegisteredEntities;
};
