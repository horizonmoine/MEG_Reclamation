#pragma once

#include "CoreMinimal.h"
#include "Data/LiminalGameInstance.h"
#include "GameFramework/Actor.h"
#include "ProcGen/LiminalLayoutLibrary.h"
#include "LiminalLevelGenerator.generated.h"

class UInstancedStaticMeshComponent;
class AExtractionZone;
class ALootActor;
class APointLight;
class ALiminalHidingSpot;
class ALiminalVentActor;
class ALiminalPortalActor;
class ANavMeshBoundsVolume;

UENUM(BlueprintType)
enum class EMapScalePreset : uint8
{
	Compact UMETA(DisplayName = "Compact (24x24, 8 Salles)"),
	Standard UMETA(DisplayName = "Standard (36x36, 16 Salles)"),
	GrandLabyrinthe UMETA(DisplayName = "Grand Labyrinthe (48x48, 24 Salles)"),
	MegaExpedition UMETA(DisplayName = "Mega Expedition (64x64, 36 Salles)")
};

/**
 * Generateur deterministe d'etage liminal (grille plate, salles + couloirs en L).
 * Server-authoritative : la seed est repliquee ; les clients reconstruisent les
 * visuels a l'identique, le serveur peuple le gameplay (spawns, extraction, IA).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalLevelGenerator : public AActor
{
	GENERATED_BODY()

public:
	ALiminalLevelGenerator();

	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "ProcGen")
	void Generate(int32 InSeed);

	UFUNCTION(BlueprintPure, Category = "ProcGen")
	int32 GetSeed() const;

	UFUNCTION(BlueprintPure, Category = "ProcGen")
	int32 GetLayoutHash() const;

	UFUNCTION(BlueprintCallable, Category = "ProcGen")
	void SetBiome(ELevelBiome NewBiome);

	UFUNCTION(BlueprintPure, Category = "ProcGen")
	ELevelBiome GetBiome() const { return Biome; }

protected:
	UFUNCTION()
	void OnRep_Seed();

	UFUNCTION()
	void OnRep_Biome();

	void ComputeLayout();
	void BuildVisuals();
	void ClearSpawnedActors();
	void SpawnGameplayActors();
	void SpawnPlayerStarts();
	void SpawnExtraction(const FProcRoom& FarthestRoom);
	void SpawnLoots();
	void SpawnHounds();
	void SpawnSmilers();
	void SpawnClumps();
	void SpawnWatchers();
	void SpawnWretches();
	void SpawnDeathmoths();
	void SpawnSkinwalkers();
	void SpawnPartygoers();
	void SpawnDullers();
	void SpawnJerrys();
	void ApplyBiomeMaterials();
	void SpawnPillarsAndFixtures();
	void SpawnEnvironmentalProps();
	void SpawnNonEuclideanPortals();
	void SetupNavMeshBounds();
	FVector CellToWorld(int32 X, int32 Y, float Z = 0.0f) const;

public:
	UPROPERTY(ReplicatedUsing = OnRep_Seed)
	int32 Seed = 42;

	UPROPERTY(ReplicatedUsing = OnRep_Biome, EditAnywhere, BlueprintReadOnly, Category = "ProcGen|Biome")
	ELevelBiome Biome = ELevelBiome::Level0_YellowLobby;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProcGen|Echelle")
	EMapScalePreset MapScale = EMapScalePreset::GrandLabyrinthe;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProcGen|Grille", meta = (ClampMin = "8", ClampMax = "128"))
	int32 GridWidth = 48;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProcGen|Grille", meta = (ClampMin = "8", ClampMax = "128"))
	int32 GridHeight = 48;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Grille", meta = (ClampMin = "200", ClampMax = "800"))
	float CellSize = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Grille", meta = (ClampMin = "200", ClampMax = "600"))
	float WallHeight = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ProcGen|Salles", meta = (ClampMin = "2", ClampMax = "64"))
	int32 RoomCount = 24;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Salles", meta = (ClampMin = "2"))
	int32 MinRoomSize = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Salles", meta = (ClampMin = "2"))
	int32 MaxRoomSize = 8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Salles", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ExtraLoopChance = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 LootCount = 35;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 HoundCount = 4;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 SmilerCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 ClumpCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 WatcherCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 WretchCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 DeathmothCount = 3;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	bool bCeilings = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen")
	bool bGenerateOnBeginPlay = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> FloorMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> WallMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> CeilingMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> WallDoorwayMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> PillarMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> PoolColumnMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> CeilingLightMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> PipeMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> DeskMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> ChairMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> LockerMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Rendu")
	TSoftObjectPtr<UStaticMesh> VentMeshAsset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSubclassOf<ALootActor> LootClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> HoundPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> SmilerPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> ClumpPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> WatcherPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> WretchPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> DeathmothPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> SkinwalkerPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> PartygoerPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> DullerPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement")
	TSoftClassPtr<APawn> JerryPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 SkinwalkerCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 PartygoerCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 DullerCount = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Peuplement", meta = (ClampMin = "0"))
	int32 JerryCount = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Interactions")
	TSubclassOf<ALiminalHidingSpot> HidingSpotClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Interactions")
	TSubclassOf<ALiminalVentActor> VentActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ProcGen|Interactions")
	TSubclassOf<ALiminalPortalActor> PortalActorClass;

private:
	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> FloorInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> WallInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> CeilingInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> WaterInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> PillarInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> CeilingLightInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> PipeInstances;

	UPROPERTY(VisibleAnywhere, Category = "ProcGen", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInstancedStaticMeshComponent> PropInstances;

	FGeneratedLayout CurrentLayout;
	TArray<TObjectPtr<AActor>> SpawnedActors;
	TArray<TObjectPtr<AActor>> CosmeticActors;

	UPROPERTY(Transient)
	TMap<FString, TObjectPtr<UTexture2D>> LoadedTextures;
};
