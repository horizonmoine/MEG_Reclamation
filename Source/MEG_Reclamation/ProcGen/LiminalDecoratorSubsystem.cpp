#include "ProcGen/LiminalDecoratorSubsystem.h"

#include "Components/DecalComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

ULiminalDecoratorSubsystem::ULiminalDecoratorSubsystem()
{
}

void ULiminalDecoratorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	BuildDefaultConfigs();
}

void ULiminalDecoratorSubsystem::Deinitialize()
{
	ClearAllDecorations();
	Super::Deinitialize();
}

void ULiminalDecoratorSubsystem::BuildDefaultConfigs()
{
	auto AddProp = [](FBiomeScatterConfig& InConfig, const TCHAR* Path, float Weight, FVector MinScale = FVector(0.9f), FVector MaxScale = FVector(1.1f), float ZOff = 0.0f)
	{
		FBiomeScatterProp Prop;
		Prop.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Path));
		Prop.Weight = Weight;
		Prop.ScaleMin = MinScale;
		Prop.ScaleMax = MaxScale;
		Prop.ZOffset = ZOff;
		Prop.bRandomYaw = true;
		InConfig.Props.Add(Prop);
	};

	// Level 0 — Yellow Lobby : chaises renversees, conduits de ventilation, casiers, dechets, caisses MEG, distributeur vintage
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level0_YellowLobby;
		Config.Density = 0.025f;
		Config.DecalDensity = 0.03f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"), 2.0f, FVector(0.85f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 0.7f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 0.5f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 2.5f, FVector(0.7f), FVector(1.2f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 0.4f, FVector(0.7f), FVector(0.85f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine"), 0.6f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"), 0.5f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 0.6f, FVector(0.9f), FVector(1.1f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 1 — Habitable Zone : casiers, tuyauterie industrielle, bureaux, chaises, caisses, medkits, chariot loot, disjoncteur
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level1_HabitableZone;
		Config.Density = 0.035f;
		Config.DecalDensity = 0.04f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 2.0f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe"), 1.5f, FVector(0.5f), FVector(0.8f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk"), 1.0f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"), 1.2f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 1.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 2.0f, FVector(0.8f), FVector(1.2f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 1.0f, FVector(0.75f), FVector(0.9f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Medkit_MEG.SM_Medkit_MEG"), 0.5f, FVector(0.8f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine"), 0.8f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Emergency_Breaker.SM_Emergency_Breaker"), 1.2f, FVector(0.95f), FVector(1.05f), 40.0f);
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"), 1.2f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 1.5f, FVector(0.9f), FVector(1.1f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 2 — Pipe Dreams : tuyaux industriels massifs, gaines de ventilation, caisses, dechets, tuyauteries plafond
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level2_PipeDreams;
		Config.Density = 0.04f;
		Config.DecalDensity = 0.03f;
		AddProp(Config, TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe"), 3.0f, FVector(0.6f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"), 1.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 2.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 2.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 3.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Emergency_Breaker.SM_Emergency_Breaker"), 1.5f, FVector(0.95f), FVector(1.05f), 40.0f);
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 0.6f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 1.5f, FVector(0.8f), FVector(1.2f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 0.8f, FVector(0.75f), FVector(0.9f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"), 0.8f, FVector(0.95f), FVector(1.05f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 3 — Electrical Station : armoires, tuyauteries electriques, disjoncteurs urgence, casiers, caisses MEG
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level3_ElectricalStation;
		Config.Density = 0.035f;
		Config.DecalDensity = 0.03f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Emergency_Breaker.SM_Emergency_Breaker"), 2.5f, FVector(0.95f), FVector(1.05f), 40.0f);
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 2.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 1.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 1.2f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 2.0f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe"), 2.0f, FVector(0.6f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 1.5f, FVector(0.8f), FVector(1.2f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 0.8f, FVector(0.75f), FVector(0.9f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"), 1.0f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine"), 0.5f, FVector(0.95f), FVector(1.05f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 4 — Abandoned Office : cubicles, distributeurs vintage, chariots, bureaux abandonnes, chaises
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level4_AbandonedOffice;
		Config.Density = 0.05f;
		Config.DecalDensity = 0.05f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk"), 3.0f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"), 3.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine"), 1.8f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"), 1.0f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Emergency_Breaker.SM_Emergency_Breaker"), 0.8f, FVector(0.95f), FVector(1.05f), 40.0f);
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 0.8f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 0.8f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"), 0.6f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 3.0f, FVector(0.8f), FVector(1.3f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 0.7f, FVector(0.75f), FVector(0.9f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Medkit_MEG.SM_Medkit_MEG"), 0.4f, FVector(0.8f), FVector(1.0f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 6 — Lights Out : Identique au Level 0 mais tres sparse dans les tenebres
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level6_LightsOut;
		Config.Density = 0.015f;
		Config.DecalDensity = 0.02f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 0.5f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 1.0f, FVector(0.8f), FVector(1.1f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 8 — Cave System : roches, dechets anciens, caisses expedition
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level8_CaveSystem;
		Config.Density = 0.03f;
		Config.DecalDensity = 0.02f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 1.2f, FVector(0.8f), FVector(1.2f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 0.6f, FVector(0.75f), FVector(0.9f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 9 — Suburban : mobilier de maison, dechets urbains
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level9_DarkSuburbs;
		Config.Density = 0.04f;
		Config.DecalDensity = 0.04f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 1.5f, FVector(0.8f), FVector(1.2f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 10 — Wheat Fields : caisses d'expedition, epaves
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level10_WheatFields;
		Config.Density = 0.02f;
		Config.DecalDensity = 0.01f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 0.5f, FVector(0.75f), FVector(0.9f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level 37 — Poolrooms : colonnes de piscine carrees, carrelage, conduits
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::Level37_Poolrooms;
		Config.Density = 0.02f;
		Config.DecalDensity = 0.02f;
		AddProp(Config, TEXT("/Game/Meshes/Modular/SM_Pool_Column.SM_Pool_Column"), 2.0f, FVector(0.8f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"), 0.5f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 0.8f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 0.6f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 0.3f, FVector(0.6f), FVector(0.9f));
		BiomeConfigs.Add(Config.Biome, Config);
	}

	// Level ! — Run For Your Life : obstacles de couloir, casiers, caisses renverses, chaises, dechets, medkits, chariots
	{
		FBiomeScatterConfig Config;
		Config.Biome = ELevelBiome::LevelRun_RunForYourLife;
		Config.Density = 0.04f;
		Config.DecalDensity = 0.06f;
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"), 1.5f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk"), 1.0f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"), 2.0f, FVector(1.0f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe"), 1.0f, FVector(0.7f), FVector(0.7f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"), 2.0f, FVector(0.8f), FVector(1.2f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"), 1.0f, FVector(0.8f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Medkit_MEG.SM_Medkit_MEG"), 0.6f, FVector(0.8f), FVector(1.0f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine"), 0.8f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"), 1.5f, FVector(0.95f), FVector(1.05f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Emergency_Breaker.SM_Emergency_Breaker"), 0.8f, FVector(0.95f), FVector(1.05f), 40.0f);
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"), 1.2f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"), 1.0f, FVector(0.9f), FVector(1.1f));
		AddProp(Config, TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"), 0.8f, FVector(0.9f), FVector(1.1f));
		BiomeConfigs.Add(Config.Biome, Config);
	}
}

void ULiminalDecoratorSubsystem::DecorateRooms(ELevelBiome Biome,
	const TArray<FVector>& RoomCenters, const TArray<FVector>& RoomSizes, int32 Seed)
{
	ClearAllDecorations();

	const FBiomeScatterConfig* Config = BiomeConfigs.Find(Biome);
	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("DecoratorSubsystem: No scatter config for biome %d"), static_cast<int32>(Biome));
		return;
	}

	FRandomStream Stream(Seed + 7919); // Offset du seed pour decorations distinctes du layout
	TotalPropsPlaced = 0;

	for (int32 i = 0; i < RoomCenters.Num() && i < RoomSizes.Num(); ++i)
	{
		PlacePropsInRoom(*Config, RoomCenters[i], RoomSizes[i], Stream);
		PlaceDecalsInRoom(*Config, RoomCenters[i], RoomSizes[i], Stream);
	}

	OnDecorationComplete.Broadcast(Biome, TotalPropsPlaced);

	UE_LOG(LogTemp, Log, TEXT("DecoratorSubsystem: Placed %d props in %d rooms for biome %d"),
		TotalPropsPlaced, RoomCenters.Num(), static_cast<int32>(Biome));
}

void ULiminalDecoratorSubsystem::PlacePropsInRoom(const FBiomeScatterConfig& Config,
	const FVector& RoomCenter, const FVector& RoomSize, FRandomStream& Stream)
{
	if (Config.Props.Num() == 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Calculer le nombre de props base sur la densite et la surface
	const float Area = RoomSize.X * RoomSize.Y;
	const int32 TargetPropCount = FMath::RoundToInt32(Area * Config.Density / 10000.0f);

	// Calculer le poids total pour la selection ponderee
	float TotalWeight = 0.0f;
	for (const FBiomeScatterProp& Prop : Config.Props)
	{
		TotalWeight += Prop.Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return;
	}

	for (int32 i = 0; i < TargetPropCount; ++i)
	{
		// Selectionner un prop aleatoirement (pondere)
		float Roll = Stream.FRandRange(0.0f, TotalWeight);
		int32 SelectedIndex = 0;
		for (int32 j = 0; j < Config.Props.Num(); ++j)
		{
			Roll -= Config.Props[j].Weight;
			if (Roll <= 0.0f)
			{
				SelectedIndex = j;
				break;
			}
		}

		const FBiomeScatterProp& SelectedProp = Config.Props[SelectedIndex];

		// Position aleatoire dans la salle (avec marge interieure)
		const float Margin = 50.0f;
		const float X = Stream.FRandRange(-RoomSize.X * 0.5f + Margin, RoomSize.X * 0.5f - Margin);
		const float Y = Stream.FRandRange(-RoomSize.Y * 0.5f + Margin, RoomSize.Y * 0.5f - Margin);
		const FVector PropLocation = RoomCenter + FVector(X, Y, SelectedProp.ZOffset);

		// Rotation
		FRotator PropRotation = FRotator::ZeroRotator;
		if (SelectedProp.bRandomYaw)
		{
			PropRotation.Yaw = Stream.FRandRange(0.0f, 360.0f);
		}

		// Scale aleatoire dans la plage
		const FVector PropScale = FVector(
			Stream.FRandRange(SelectedProp.ScaleMin.X, SelectedProp.ScaleMax.X),
			Stream.FRandRange(SelectedProp.ScaleMin.Y, SelectedProp.ScaleMax.Y),
			Stream.FRandRange(SelectedProp.ScaleMin.Z, SelectedProp.ScaleMax.Z));

		// Spawner le prop seulement si le mesh est charge
		UStaticMesh* Mesh = SelectedProp.Mesh.LoadSynchronous();
		if (!Mesh)
		{
			continue;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;

		AStaticMeshActor* PropActor = World->SpawnActor<AStaticMeshActor>(PropLocation, PropRotation, SpawnParams);
		if (PropActor)
		{
			PropActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			PropActor->SetActorScale3D(PropScale);
			PropActor->GetStaticMeshComponent()->SetMobility(EComponentMobility::Stationary);
			SpawnedDecorations.Add(PropActor);
			TotalPropsPlaced++;
		}
	}
}

void ULiminalDecoratorSubsystem::PlaceDecalsInRoom(const FBiomeScatterConfig& Config,
	const FVector& RoomCenter, const FVector& RoomSize, FRandomStream& Stream)
{
	if (Config.DecalMaterials.Num() == 0 || Config.DecalDensity <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float Area = RoomSize.X * RoomSize.Y;
	const int32 TargetDecalCount = FMath::RoundToInt32(Area * Config.DecalDensity / 10000.0f);

	for (int32 i = 0; i < TargetDecalCount; ++i)
	{
		// Position aleatoire au sol
		const float X = Stream.FRandRange(-RoomSize.X * 0.5f, RoomSize.X * 0.5f);
		const float Y = Stream.FRandRange(-RoomSize.Y * 0.5f, RoomSize.Y * 0.5f);
		const FVector DecalLocation = RoomCenter + FVector(X, Y, 1.0f); // Juste au-dessus du sol

		// Selectionner un materiau aleatoire
		const int32 MatIndex = Stream.RandRange(0, Config.DecalMaterials.Num() - 1);
		UMaterialInterface* DecalMat = Config.DecalMaterials[MatIndex].LoadSynchronous();
		if (!DecalMat)
		{
			continue;
		}

		// Spawner le decal
		const FRotator DecalRotation(90.0f, Stream.FRandRange(0.0f, 360.0f), 0.0f);
		const FVector DecalSize(128.0f, 64.0f, 64.0f);

		AActor* DecalActor = World->SpawnActor<AActor>(DecalLocation, DecalRotation);
		if (DecalActor)
		{
			UDecalComponent* DecalComp = NewObject<UDecalComponent>(DecalActor);
			DecalComp->SetDecalMaterial(DecalMat);
			DecalComp->DecalSize = DecalSize;
			DecalComp->RegisterComponent();
			DecalActor->AddOwnedComponent(DecalComp);
			DecalComp->AttachToComponent(DecalActor->GetRootComponent(),
				FAttachmentTransformRules::KeepRelativeTransform);

			SpawnedDecorations.Add(DecalActor);
		}
	}
}

void ULiminalDecoratorSubsystem::ClearAllDecorations()
{
	for (auto& WeakActor : SpawnedDecorations)
	{
		if (AActor* Actor = WeakActor.Get())
		{
			Actor->Destroy();
		}
	}
	SpawnedDecorations.Empty();
	TotalPropsPlaced = 0;
}

void ULiminalDecoratorSubsystem::AddScatterProp(ELevelBiome Biome, const FString& MeshPath, float Weight,
	FVector ScaleMin, FVector ScaleMax, float ZOffset, bool bRandomYaw)
{
	FBiomeScatterConfig& Config = BiomeConfigs.FindOrAdd(Biome);
	Config.Biome = Biome;

	FBiomeScatterProp Prop;
	Prop.Mesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(MeshPath));
	Prop.Weight = Weight;
	Prop.ScaleMin = ScaleMin;
	Prop.ScaleMax = ScaleMax;
	Prop.ZOffset = ZOffset;
	Prop.bRandomYaw = bRandomYaw;

	Config.Props.Add(Prop);
}

TArray<FBiomeScatterProp> ULiminalDecoratorSubsystem::GetScatterPropsForBiome(ELevelBiome Biome) const
{
	if (const FBiomeScatterConfig* Config = BiomeConfigs.Find(Biome))
	{
		return Config->Props;
	}
	return TArray<FBiomeScatterProp>();
}

TArray<FString> ULiminalDecoratorSubsystem::GetDefaultPropAssetPaths()
{
	TArray<FString> Paths;
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Vending_Machine.SM_Vending_Machine"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Vent_Duct_Straight.SM_Vent_Duct_Straight"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Vent_Duct_Corner.SM_Vent_Duct_Corner"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Emergency_Breaker.SM_Emergency_Breaker"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Loot_Cart.SM_Loot_Cart"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Ceiling_Pipes.SM_Ceiling_Pipes"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Debris_Trash.SM_Debris_Trash"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk"));
	Paths.Add(TEXT("/Game/Meshes/Props/SM_Medkit_MEG.SM_Medkit_MEG"));
	Paths.Add(TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe"));
	Paths.Add(TEXT("/Game/Meshes/Modular/SM_Pool_Column.SM_Pool_Column"));
	return Paths;
}

bool ULiminalDecoratorSubsystem::IsPropRegisteredInAnyBiome(const FString& MeshPath)
{
	TArray<FString> Defaults = GetDefaultPropAssetPaths();
	for (const FString& P : Defaults)
	{
		if (P.Equals(MeshPath, ESearchCase::IgnoreCase) || P.Contains(MeshPath))
		{
			return true;
		}
	}
	return false;
}

