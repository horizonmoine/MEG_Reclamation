#pragma once

#include "CoreMinimal.h"
#include "Data/LiminalGameInstance.h"
#include "LiminalTileTypes.generated.h"

/**
 * Types de tuiles modulaires pre-fabriquees pour les espaces liminaux.
 */
UENUM(BlueprintType)
enum class ELiminalTileType : uint8
{
	CorridorStraight      UMETA(DisplayName = "Couloir Droit"),
	CorridorCorner        UMETA(DisplayName = "Couloir Angle (L)"),
	CorridorTee           UMETA(DisplayName = "Carrefour en T"),
	CorridorCross         UMETA(DisplayName = "Carrefour 4 Voies"),
	CorridorDeadEnd       UMETA(DisplayName = "Cul-de-sac"),
	RoomSmall             UMETA(DisplayName = "Petite Salle (4x4)"),
	RoomMedium            UMETA(DisplayName = "Salle Moyenne (8x8)"),
	RoomLarge             UMETA(DisplayName = "Grande Salle (12x12)"),
	HeroRoom              UMETA(DisplayName = "Hero Room (Salle Clef de Puzzle)"),
	StairsUp              UMETA(DisplayName = "Escaliers Montants"),
	StairsDown            UMETA(DisplayName = "Escaliers Descendants"),
	ElevatorShaft         UMETA(DisplayName = "Puits d'Ascenseur"),
	MaintenanceAirlock    UMETA(DisplayName = "Sas de Maintenance / Extraction"),
	CentralHub            UMETA(DisplayName = "Hall Central / Carrefour"),
	SecretRoom            UMETA(DisplayName = "Salle Secrete Scellee"),
	StorageRoom           UMETA(DisplayName = "Salle de Stockage"),
	OfficeRoom            UMETA(DisplayName = "Bureau Desaffecte"),
	MachineRoom           UMETA(DisplayName = "Salle des Machines"),
	DormitoryRoom         UMETA(DisplayName = "Dortoir Improvise")
};

/**
 * Directions cardinales et verticales des connecteurs (Sockets).
 */
UENUM(BlueprintType)
enum class ELiminalSocketDirection : uint8
{
	North = 0,
	East  = 1,
	South = 2,
	West  = 3,
	Up    = 4,
	Down  = 5
};

/**
 * Type de connecteur pour la compatibilite d'assemblage modulaire.
 */
UENUM(BlueprintType)
enum class ELiminalSocketType : uint8
{
	ClosedWall     UMETA(DisplayName = "Mur Plein (Ferme)"),
	StandardDoor   UMETA(DisplayName = "Porte Standard (Cadre)"),
	OpenArchway    UMETA(DisplayName = "Passage Ouvert (Arceau)"),
	DoubleDoor     UMETA(DisplayName = "Double Porte Blindee"),
	Ventilation    UMETA(DisplayName = "Conduit de Ventilation"),
	Staircase      UMETA(DisplayName = "Connexion d'Escalier")
};

/**
 * Categorie de point d'ancrage diégetique (Spawn Sockets).
 */
UENUM(BlueprintType)
enum class ELiminalSpawnSocketCategory : uint8
{
	CeilingLight,     // Neon clignotant, ampoule nue, projecteur
	WallAppliance,    // Disjoncteur, clavier a code, extincteur, alarme
	FloorLoot,        // Emplacement de caisse de butin physique
	MonsterPatrol,    // Point de patrouille / spawn d'entite
	CeilingDecal,     // Humidite, moisissure, traces de suie
	WallDecoration,   // Papier peint dechire, signaletique M.E.G., tuyauterie
	FloorHazard       // Eau stagnante, plaque electrique, cable denude
};

/**
 * Description d'un connecteur de tuile modulaire.
 */
USTRUCT(BlueprintType)
struct FLiminalTileSocket
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	ELiminalSocketDirection Direction = ELiminalSocketDirection::North;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	ELiminalSocketType SocketType = ELiminalSocketType::StandardDoor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	FVector LocalOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket")
	bool bIsRequired = false;
};

/**
 * Point d'accroche pour spawn de props, lumieres et gameplay.
 */
USTRUCT(BlueprintType)
struct FLiminalPropSocket
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PropSocket")
	FName SocketId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PropSocket")
	ELiminalSpawnSocketCategory Category = ELiminalSpawnSocketCategory::FloorLoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PropSocket")
	FTransform RelativeTransform = FTransform::Identity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PropSocket", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SpawnProbability = 1.0f;
};

/**
 * Instance placee d'une tuile dans le donjon modulaire genere.
 */
USTRUCT(BlueprintType)
struct FLiminalPlacedTile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	int32 GridX = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	int32 GridY = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	int32 GridZ = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	float RotationYaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	ELiminalTileType TileType = ELiminalTileType::CorridorStraight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	FName TileAssetId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PlacedTile")
	TArray<FLiminalPropSocket> ResolvedPropSockets;
};
