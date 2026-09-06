#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Data/LiminalGameInstance.h"
#include "LiminalDecoratorSubsystem.generated.h"

class UStaticMesh;
class UMaterialInterface;
class UInstancedStaticMeshComponent;

/**
 * Definition d'un prop decoratif scatterable par biome.
 */
USTRUCT(BlueprintType)
struct FBiomeScatterProp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	TSoftObjectPtr<UStaticMesh> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	FVector ScaleMin = FVector(0.8f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	FVector ScaleMax = FVector(1.2f);

	/** Offset vertical additionnel (pour poser sur le sol). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	float ZOffset = 0.0f;

	/** Rotation aleatoire autour de l'axe Z ? */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	bool bRandomYaw = true;

	/** Poids de selection relative. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter", meta = (ClampMin = "0.1"))
	float Weight = 1.0f;
};

/**
 * Configuration de scatter pour un biome.
 */
USTRUCT(BlueprintType)
struct FBiomeScatterConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	ELevelBiome Biome = ELevelBiome::Level0_YellowLobby;

	/** Props scatterables dans ce biome. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
	TArray<FBiomeScatterProp> Props;

	/** Densite de props par 100 unites carrees (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Density = 0.15f;

	/** Materiaux de decals pour ce biome (taches, empreintes, graffitis). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Decals")
	TArray<TSoftObjectPtr<UMaterialInterface>> DecalMaterials;

	/** Densite de decals. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Decals", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DecalDensity = 0.05f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDecorationComplete, ELevelBiome, Biome, int32, PropsPlaced);

/**
 * Sous-systeme de decoration procedurale des niveaux liminaux.
 * Parseme automatiquement des props decoratifs et des decals dans les salles
 * selon le biome actif, apres que la generation de layout est terminee.
 *
 * Objectif : transformer des salles vides et steriles en environnements vivants,
 * avec des papiers par terre, des chaises renversees, des taches d'humidite,
 * des graffitis d'avertissement laissés par des survivants precedents.
 */
UCLASS(BlueprintType)
class MEG_RECLAMATION_API ULiminalDecoratorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULiminalDecoratorSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * Decore un ensemble de salles avec des props et decals selon le biome.
	 * Appele par le LiminalLevelGenerator apres la generation du layout.
	 *
	 * @param Biome Le biome actuel.
	 * @param RoomCenters Positions centrales des salles a decorer.
	 * @param RoomSizes Dimensions des salles (en unites monde).
	 * @param Seed Seed pour la reproductibilite (meme seed = memes props).
	 */
	UFUNCTION(BlueprintCallable, Category = "Decorator")
	void DecorateRooms(ELevelBiome Biome, const TArray<FVector>& RoomCenters,
		const TArray<FVector>& RoomSizes, int32 Seed);

	/** Nettoie tous les props places. */
	UFUNCTION(BlueprintCallable, Category = "Decorator")
	void ClearAllDecorations();

	UFUNCTION(BlueprintPure, Category = "Decorator")
	int32 GetTotalPropsPlaced() const { return TotalPropsPlaced; }

	/** Enregistre ou ajoute un prop decoratif pour un biome specifique (accessible via Blueprint/Python). */
	UFUNCTION(BlueprintCallable, Category = "Decorator")
	void AddScatterProp(ELevelBiome Biome, const FString& MeshPath, float Weight = 1.0f,
		FVector ScaleMin = FVector(0.9f), FVector ScaleMax = FVector(1.1f), float ZOffset = 0.0f, bool bRandomYaw = true);

	/** Recupere la liste des props enregistres pour un biome. */
	UFUNCTION(BlueprintPure, Category = "Decorator")
	TArray<FBiomeScatterProp> GetScatterPropsForBiome(ELevelBiome Biome) const;

	/** Retourne la liste de tous les chemins d'assets de props configures par defaut. */
	UFUNCTION(BlueprintCallable, Category = "Decorator")
	static TArray<FString> GetDefaultPropAssetPaths();

	/** Verifie si un asset mesh donne est enregistre dans au moins un biome. */
	UFUNCTION(BlueprintCallable, Category = "Decorator")
	static bool IsPropRegisteredInAnyBiome(const FString& MeshPath);

	UPROPERTY(BlueprintAssignable, Category = "Decorator")
	FOnDecorationComplete OnDecorationComplete;

protected:
	void BuildDefaultConfigs();
	void PlacePropsInRoom(const FBiomeScatterConfig& Config, const FVector& RoomCenter,
		const FVector& RoomSize, FRandomStream& Stream);
	void PlaceDecalsInRoom(const FBiomeScatterConfig& Config, const FVector& RoomCenter,
		const FVector& RoomSize, FRandomStream& Stream);

private:
	TMap<ELevelBiome, FBiomeScatterConfig> BiomeConfigs;
	TArray<TWeakObjectPtr<AActor>> SpawnedDecorations;
	int32 TotalPropsPlaced = 0;
};
