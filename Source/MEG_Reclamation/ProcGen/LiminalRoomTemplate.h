#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/LiminalGameInstance.h"
#include "LiminalRoomTemplate.generated.h"

class UStaticMesh;
class USoundBase;

/**
 * Type fonctionnel d'une salle generee procedurallement.
 */
UENUM(BlueprintType)
enum class ERoomType : uint8
{
	Corridor       UMETA(DisplayName = "Couloir"),
	SmallRoom      UMETA(DisplayName = "Petite salle"),
	MediumRoom     UMETA(DisplayName = "Salle moyenne"),
	LargeHall      UMETA(DisplayName = "Grande salle"),
	Closet         UMETA(DisplayName = "Placard / Reduit"),
	Stairwell      UMETA(DisplayName = "Cage d'escalier"),
	Junction       UMETA(DisplayName = "Intersection"),
	DeadEnd        UMETA(DisplayName = "Cul-de-sac"),
	SecretRoom     UMETA(DisplayName = "Salle secrete"),
	Elevator       UMETA(DisplayName = "Ascenseur"),
	Bathroom       UMETA(DisplayName = "Sanitaires"),
	Office         UMETA(DisplayName = "Bureau"),
	Utility        UMETA(DisplayName = "Local technique"),
	Storage        UMETA(DisplayName = "Salle de Stockage"),
	Dormitory      UMETA(DisplayName = "Dortoir Improvise")
};

/**
 * Point de spawn pour un prop decoratif dans une salle.
 */
USTRUCT(BlueprintType)
struct FRoomPropSpawnPoint
{
	GENERATED_BODY()

	/** Offset relatif au centre de la salle. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	FVector RelativeOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	FRotator Rotation = FRotator::ZeroRotator;

	/** Mesh a utiliser. Si null, un mesh aleatoire du biome sera choisi. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	TSoftObjectPtr<UStaticMesh> PropMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop")
	FVector Scale = FVector(1.0f);

	/** Probabilite que ce prop soit place (0-1). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prop", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnChance = 0.8f;
};

/**
 * Point de spawn de lumiere dans une salle.
 */
USTRUCT(BlueprintType)
struct FRoomLightPoint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	FVector RelativeOffset = FVector(0.0f, 0.0f, 280.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light")
	FLinearColor LightColor = FLinearColor(1.0f, 0.96f, 0.88f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0"))
	float Intensity = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0"))
	float AttenuationRadius = 800.0f;

	/** Probabilite de dysfonctionnement (clignotement, eteinte). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float MalfunctionChance = 0.15f;
};

/**
 * DataAsset definissant un template de salle pre-designe.
 * Chaque biome a plusieurs templates par type de salle.
 * La procgen selectionne un template compatible selon le biome et la taille.
 */
UCLASS(BlueprintType)
class MEG_RECLAMATION_API ULiminalRoomTemplate : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	/** Identifiant unique du template. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	FName TemplateId;

	/** Description humaine du template. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	FText Description;

	/** Biome(s) pour lesquels ce template est valide. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	TArray<ELevelBiome> CompatibleBiomes;

	/** Type de salle que ce template represente. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template")
	ERoomType RoomType = ERoomType::MediumRoom;

	/** Taille minimale de la salle en cells pour utiliser ce template. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template", meta = (ClampMin = "1"))
	int32 MinRoomSizeCells = 3;

	/** Taille maximale de la salle en cells. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template", meta = (ClampMin = "1"))
	int32 MaxRoomSizeCells = 8;

	/** Hauteur du plafond en cm (ecrase le WallHeight du biome). 0 = utiliser le defaut. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Geometry", meta = (ClampMin = "0.0"))
	float CeilingHeightOverride = 0.0f;

	/** Meshes de mur specifiques a ce template (ecrase les meshes du biome). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Meshes")
	TSoftObjectPtr<UStaticMesh> WallMeshOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Meshes")
	TSoftObjectPtr<UStaticMesh> FloorMeshOverride;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Meshes")
	TSoftObjectPtr<UStaticMesh> CeilingMeshOverride;

	/** Props decoratifs pre-places dans cette salle. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Decoration")
	TArray<FRoomPropSpawnPoint> PropSpawnPoints;

	/** Points de lumiere. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Lighting")
	TArray<FRoomLightPoint> LightPoints;

	/** Points de spawn de loot (probabiliste). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Gameplay")
	TArray<FVector> LootSpawnOffsets;

	/** Points de spawn potentiels d'entites (probabiliste). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Gameplay")
	TArray<FVector> EntitySpawnOffsets;

	/** Ce template peut-il contenir une porte ? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Connectivity")
	bool bCanHaveDoor = true;

	/** Ce template peut-il contenir un conduit de ventilation ? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Connectivity")
	bool bCanHaveVent = false;

	/** Ce template peut-il contenir une cachette ? */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Gameplay")
	bool bCanHaveHidingSpot = true;

	/** Ambiance sonore specifique a ce template (ecrase l'ambiance du biome). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template|Audio")
	TSoftObjectPtr<USoundBase> AmbientSoundOverride;

	/** Poids de selection par rapport aux autres templates compatibles (plus eleve = plus frequent). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Template", meta = (ClampMin = "0.1"))
	float SelectionWeight = 1.0f;
};
