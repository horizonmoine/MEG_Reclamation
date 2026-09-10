#include "ProcGen/LiminalLevelGenerator.h"

#include "AI/LiminalEntity_Clump.h"
#include "AI/LiminalEntity_Deathmoth.h"
#include "AI/LiminalEntity_Duller.h"
#include "AI/LiminalEntity_Jerry.h"
#include "AI/LiminalEntity_Partygoer.h"
#include "AI/LiminalEntity_Skinwalker.h"
#include "AI/LiminalEntity_Smiler.h"
#include "AI/LiminalEntity_Watcher.h"
#include "AI/LiminalEntity_Wretch.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Data/LiminalGameInstance.h"
#include "Engine/PointLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Net/UnrealNetwork.h"
#include "Objects/ExtractionZone.h"
#include "Objects/LootActor.h"
#include "Data/ItemData.h"
#include "Engine/DataTable.h"
#include "Objects/LiminalBreakerActor.h"
#include "Objects/LiminalKeypadActor.h"
#include "Objects/LiminalValvePuzzleActor.h"
#include "Objects/LiminalFuseBoxActor.h"
#include "Objects/LiminalKeyItemActor.h"
#include "Objects/LiminalDoorActor.h"
#include "Objects/LiminalHidingSpot.h"
#include "Objects/LiminalVentActor.h"
#include "ProcGen/LiminalPortalComponent.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "ProcGen/LiminalDecoratorSubsystem.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Audio/LiminalAudioSubsystem.h"
#include "ProcGen/LiminalFlickerLightComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/GameplayStatics.h"


ALiminalLevelGenerator::ALiminalLevelGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	FloorInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorInstances"));
	FloorInstances->SetupAttachment(RootComponent);

	WallInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WallInstances"));
	WallInstances->SetupAttachment(RootComponent);

	CeilingInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CeilingInstances"));
	CeilingInstances->SetupAttachment(RootComponent);
	CeilingInstances->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WaterInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("WaterInstances"));
	WaterInstances->SetupAttachment(RootComponent);
	WaterInstances->SetCollisionProfileName(TEXT("OverlapAll"));

	PillarInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PillarInstances"));
	PillarInstances->SetupAttachment(RootComponent);
	PillarInstances->SetCollisionProfileName(TEXT("BlockAll"));

	CeilingLightInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("CeilingLightInstances"));
	CeilingLightInstances->SetupAttachment(RootComponent);
	CeilingLightInstances->SetCollisionProfileName(TEXT("NoCollision"));

	PipeInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PipeInstances"));
	PipeInstances->SetupAttachment(RootComponent);
	PipeInstances->SetCollisionProfileName(TEXT("BlockAll"));

	PropInstances = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("PropInstances"));
	PropInstances->SetupAttachment(RootComponent);
	PropInstances->SetCollisionProfileName(TEXT("BlockAll"));

	FloorMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Floor_Tile_400x400.SM_Floor_Tile_400x400")));
	WallMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Wall_Modular_400x300.SM_Wall_Modular_400x300")));
	CeilingMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Ceiling_Tile_400x400.SM_Ceiling_Tile_400x400")));
	WallDoorwayMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Wall_Modular_Doorway_400x300.SM_Wall_Modular_Doorway_400x300")));
	PillarMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Pillar_Industrial.SM_Pillar_Industrial")));
	PoolColumnMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Pool_Column.SM_Pool_Column")));
	CeilingLightMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Ceiling_FluorescentLight.SM_Ceiling_FluorescentLight")));
	PipeMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Modular/SM_Industrial_Pipe.SM_Industrial_Pipe")));
	DeskMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk")));
	ChairMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair")));
	LockerMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker")));
	VentMeshAsset = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Game/Meshes/Props/SM_VentDuct.SM_VentDuct")));

	LootClass = ALootActor::StaticClass();
	HoundPawnClass = TSoftClassPtr<APawn>(FSoftObjectPath(TEXT("/Game/AI/BP_Hound.BP_Hound_C")));
	SmilerPawnClass = TSoftClassPtr<APawn>(ALiminalEntity_Smiler::StaticClass());
	ClumpPawnClass = TSoftClassPtr<APawn>(ALiminalEntity_Clump::StaticClass());
	WatcherPawnClass = TSoftClassPtr<APawn>(ALiminalEntity_Watcher::StaticClass());
	WretchPawnClass = TSoftClassPtr<APawn>(ALiminalEntity_Wretch::StaticClass());
	DeathmothPawnClass = TSoftClassPtr<APawn>(ALiminalEntity_Deathmoth::StaticClass());
	HidingSpotClass = ALiminalHidingSpot::StaticClass();
	VentActorClass = ALiminalVentActor::StaticClass();
	PortalActorClass = ALiminalPortalActor::StaticClass();
}

void ALiminalLevelGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalLevelGenerator, Seed);
	DOREPLIFETIME(ALiminalLevelGenerator, Biome);
}

void ALiminalLevelGenerator::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	if (GIsEditor && GetWorld() && !GetWorld()->IsGameWorld() && CurrentLayout.Cells.Num() == 0)
	{
		if (ComputeLayout()) BuildVisuals();
	}
#endif
}

void ALiminalLevelGenerator::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		const FString CurrentMapName = World ? World->GetMapName() : FString();

		if (CurrentMapName.Contains(TEXT("Lobby")) || CurrentMapName.Contains(TEXT("Level0")) || CurrentMapName.Contains(TEXT("00_")))
		{
			Biome = ELevelBiome::Level0_YellowLobby;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Habitable")) || CurrentMapName.Contains(TEXT("01_")))
		{
			Biome = ELevelBiome::Level1_HabitableZone;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Pipe")) || CurrentMapName.Contains(TEXT("02_")))
		{
			Biome = ELevelBiome::Level2_PipeDreams;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Electric")) || CurrentMapName.Contains(TEXT("03_")))
		{
			Biome = ELevelBiome::Level3_ElectricalStation;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Office")) || CurrentMapName.Contains(TEXT("04_")))
		{
			Biome = ELevelBiome::Level4_AbandonedOffice;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("LightsOut")) || CurrentMapName.Contains(TEXT("06_")))
		{
			Biome = ELevelBiome::Level6_LightsOut;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Cave")) || CurrentMapName.Contains(TEXT("08_")))
		{
			Biome = ELevelBiome::Level8_CaveSystem;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Suburbs")) || CurrentMapName.Contains(TEXT("09_")))
		{
			Biome = ELevelBiome::Level9_DarkSuburbs;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Wheat")) || CurrentMapName.Contains(TEXT("10_")))
		{
			Biome = ELevelBiome::Level10_WheatFields;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Poolrooms")) || CurrentMapName.Contains(TEXT("37_")))
		{
			Biome = ELevelBiome::Level37_Poolrooms;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (CurrentMapName.Contains(TEXT("Run")) || CurrentMapName.Contains(TEXT("Gauntlet")) || CurrentMapName.Contains(TEXT("99_")))
		{
			Biome = ELevelBiome::LevelRun_RunForYourLife;
			MapScale = EMapScalePreset::GrandLabyrinthe;
		}
		else if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
		{
			Biome = GI->GetSelectedBiome();
			MapScale = static_cast<EMapScalePreset>(FMath::Clamp(GI->GetSelectedMapScale(), 0, 3));
		}

		if (bGenerateOnBeginPlay)
		{
			Generate(Seed);
		}
	}
}

void ALiminalLevelGenerator::SetBiome(ELevelBiome NewBiome)
{
	Biome = NewBiome;
	if (UWorld* World = GetWorld())
	{
		if (ULiminalAudioSubsystem* AudioSub = World->GetSubsystem<ULiminalAudioSubsystem>())
		{
			AudioSub->SetCurrentBiome(Biome);
		}
	}
	if (HasAuthority())
	{
		Generate(Seed);
	}
}

void ALiminalLevelGenerator::Generate(int32 InSeed)
{
	const int32 PreviousSeed = Seed;
	Seed = InSeed;
	if (!ComputeLayout())
	{
		Seed = PreviousSeed;
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (ULiminalAudioSubsystem* AudioSub = World->GetSubsystem<ULiminalAudioSubsystem>())
		{
			AudioSub->SetCurrentBiome(Biome);
		}
	}

	if (HasAuthority())
	{
		BuildVisuals();
		ClearSpawnedActors();
		SpawnGameplayActors();
		UE_LOG(LogTemp, Log, TEXT("LiminalLevelGenerator : seed=%d biome=%d hash=%u salles=%d"),
			Seed, static_cast<int32>(Biome), CurrentLayout.Hash, CurrentLayout.Rooms.Num());
	}
	else
	{
		BuildVisuals();
	}

	if (UWorld* World = GetWorld())
	{
		if (ULiminalAudioSubsystem* AudioSub = World->GetSubsystem<ULiminalAudioSubsystem>())
		{
			if (CurrentLayout.Rooms.Num() > 0)
			{
				float TotalVolume = 0.0f;
				for (const FProcRoom& Room : CurrentLayout.Rooms)
				{
					const float WidthM = Room.SizeX * (CellSize / 100.0f);
					const float LengthM = Room.SizeY * (CellSize / 100.0f);
					const float HeightM = 3.5f;
					TotalVolume += (WidthM * LengthM * HeightM);
				}
				const float AvgVolume = TotalVolume / CurrentLayout.Rooms.Num();
				AudioSub->UpdateRoomAcoustics(AvgVolume, 0.8f);
			}
		}
	}

	if (UWorld* CurrentWorld = GetWorld())
	{
		if (CurrentWorld->IsGameWorld())
		{
		FString SoundPath = TEXT("");
		switch(Biome) {
			case ELevelBiome::Level0_YellowLobby: SoundPath = TEXT("/Game/Audio/S_Ambient_Lobby.S_Ambient_Lobby"); break;
			case ELevelBiome::Level1_HabitableZone: SoundPath = TEXT("/Game/Audio/S_Ambient_HabitableZone.S_Ambient_HabitableZone"); break;
			case ELevelBiome::Level2_PipeDreams: SoundPath = TEXT("/Game/Audio/S_Ambient_PipeDreams.S_Ambient_PipeDreams"); break;
			case ELevelBiome::Level3_ElectricalStation: SoundPath = TEXT("/Game/Audio/S_Ambient_Electrical.S_Ambient_Electrical"); break;
			case ELevelBiome::Level4_AbandonedOffice: SoundPath = TEXT("/Game/Audio/S_Ambient_Office.S_Ambient_Office"); break;
			case ELevelBiome::Level8_CaveSystem: SoundPath = TEXT("/Game/Audio/S_Ambient_Cave.S_Ambient_Cave"); break;
			case ELevelBiome::Level9_DarkSuburbs: SoundPath = TEXT("/Game/Audio/S_Ambient_Suburbs.S_Ambient_Suburbs"); break;
			case ELevelBiome::Level10_WheatFields: SoundPath = TEXT("/Game/Audio/S_Ambient_WheatFields.S_Ambient_WheatFields"); break;
			case ELevelBiome::Level37_Poolrooms: SoundPath = TEXT("/Game/Audio/S_Ambient_Poolrooms.S_Ambient_Poolrooms"); break;
			case ELevelBiome::LevelRun_RunForYourLife: SoundPath = TEXT("/Game/Audio/S_Ambient_Run.S_Ambient_Run"); break;
			default: break;
		}

		if (!SoundPath.IsEmpty())
		{
			USoundBase* AmbientSound = Cast<USoundBase>(StaticLoadObject(USoundBase::StaticClass(), nullptr, *SoundPath));
			if (AmbientSound)
			{
				UGameplayStatics::SpawnSound2D(this, AmbientSound);
			}
		}
	}
}

}

void ALiminalLevelGenerator::OnRep_Seed()
{
	if (ComputeLayout()) BuildVisuals();
}

void ALiminalLevelGenerator::OnRep_Biome()
{
	if (UWorld* World = GetWorld())
	{
		if (ULiminalAudioSubsystem* AudioSub = World->GetSubsystem<ULiminalAudioSubsystem>())
		{
			AudioSub->SetCurrentBiome(Biome);
		}
	}
	if (ComputeLayout()) BuildVisuals();
}

int32 ALiminalLevelGenerator::GetSeed() const
{
	return Seed;
}

int32 ALiminalLevelGenerator::GetLayoutHash() const
{
	return static_cast<int32>(CurrentLayout.Hash);
}

bool ALiminalLevelGenerator::ComputeLayout()
{
	switch (MapScale)
	{
	case EMapScalePreset::Compact:
		GridWidth = 24;
		GridHeight = 24;
		RoomCount = 8;
		LootCount = 12;
		break;
	case EMapScalePreset::Standard:
		GridWidth = 36;
		GridHeight = 36;
		RoomCount = 16;
		LootCount = 22;
		break;
	case EMapScalePreset::GrandLabyrinthe:
		GridWidth = 48;
		GridHeight = 48;
		RoomCount = 24;
		LootCount = 35;
		break;
	case EMapScalePreset::MegaExpedition:
		GridWidth = 64;
		GridHeight = 64;
		RoomCount = 36;
		LootCount = 50;
		break;
	default:
		break;
	}

	FGeneratedLayout Candidate;
	if (!FLiminalLayoutBuilder::TryGenerateExpedition(Seed, GridWidth, GridHeight,
		RoomCount, MinRoomSize, MaxRoomSize, ExtraLoopChance, CellSize, Candidate))
	{
		UE_LOG(LogTemp, Error, TEXT("Expedition generation rejected: seed=%d grid=%dx%d cell=%.1f; no connected 80m layout after 20 attempts. Existing geometry retained."), Seed, GridWidth, GridHeight, CellSize);
		return false;
	}
	CurrentLayout = MoveTemp(Candidate);
	return true;
}
FVector ALiminalLevelGenerator::CellToWorld(int32 X, int32 Y, float Z) const
{
	const float OriginX = -GridWidth * CellSize * 0.5f;
	const float OriginY = -GridHeight * CellSize * 0.5f;
	return FVector(OriginX + X * CellSize + CellSize * 0.5f,
		OriginY + Y * CellSize + CellSize * 0.5f, Z);
}

void ALiminalLevelGenerator::BuildVisuals()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UStaticMesh* FloorMesh = FloorMeshAsset.LoadSynchronous();
	if (!FloorMesh)
	{
		FloorMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	}
	UStaticMesh* WallMesh = WallMeshAsset.LoadSynchronous();
	if (!WallMesh)
	{
		WallMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	}
	UStaticMesh* CeilingMesh = bCeilings ? CeilingMeshAsset.LoadSynchronous() : nullptr;
	if (bCeilings && !CeilingMesh)
	{
		CeilingMesh = FloorMesh;
	}

	UStaticMesh* PillarMesh = (Biome == ELevelBiome::Level37_Poolrooms) ? PoolColumnMeshAsset.LoadSynchronous() : PillarMeshAsset.LoadSynchronous();
	if (!PillarMesh)
	{
		PillarMesh = WallMesh;
	}
	UStaticMesh* CeilingLightMesh = CeilingLightMeshAsset.LoadSynchronous();
	if (!CeilingLightMesh)
	{
		CeilingLightMesh = WallMesh;
	}
	UStaticMesh* PipeMesh = PipeMeshAsset.LoadSynchronous();
	if (!PipeMesh)
	{
		PipeMesh = WallMesh;
	}
	UStaticMesh* PropMesh = (Biome == ELevelBiome::Level4_AbandonedOffice) ? DeskMeshAsset.LoadSynchronous() : LockerMeshAsset.LoadSynchronous();
	if (!PropMesh)
	{
		PropMesh = WallMesh;
	}

	auto SetupInstances = [World](UInstancedStaticMeshComponent* Component, UStaticMesh* Mesh)
	{
		if (!Component)
		{
			return;
		}
		Component->ClearInstances();
		Component->SetStaticMesh(Mesh);
		Component->MarkRenderStateDirty();
	};

	SetupInstances(FloorInstances, FloorMesh);
	SetupInstances(WallInstances, WallMesh);
	SetupInstances(CeilingInstances, CeilingMesh);
	SetupInstances(PillarInstances, PillarMesh);
	SetupInstances(CeilingLightInstances, CeilingLightMesh);
	SetupInstances(PipeInstances, PipeMesh);
	SetupInstances(PropInstances, PropMesh);
	SetupInstances(WaterInstances, FloorMesh);

	ApplyBiomeMaterials();

	FloorInstances->SetCastShadow(false);
	CeilingInstances->SetCastShadow(false);

	for (AActor* Cosmetic : CosmeticActors)
	{
		if (IsValid(Cosmetic))
		{
			Cosmetic->Destroy();
		}
	}
	CosmeticActors.Empty();

	if (!FloorMesh || !WallMesh)
	{
		UE_LOG(LogTemp, Error, TEXT("LiminalLevelGenerator : meshes de tuiles manquants"));
		return;
	}

	const bool bIsCustomFloor = FloorMesh && (FloorMesh->GetBounds().BoxExtent.X > 150.0f);
	const float FloorScaleFactor = bIsCustomFloor ? (CellSize / 400.0f) : ((CellSize + 4.0f) / 100.0f);
	const FTransform FloorTransformBase(FRotator::ZeroRotator, FVector::ZeroVector,
		FVector(FloorScaleFactor, FloorScaleFactor, 1.0f));

	const bool bIsCustomCeiling = CeilingMesh && (CeilingMesh->GetBounds().BoxExtent.X > 150.0f);
	const float CeilingScaleFactor = bIsCustomCeiling ? (CellSize / 400.0f) : ((CellSize + 4.0f) / 100.0f);
	const FRotator CeilingRotation = bIsCustomCeiling ? FRotator::ZeroRotator : FRotator(180.0f, 0.0f, 0.0f);

	const bool bIsCustomWall = WallMesh && (WallMesh->GetBounds().BoxExtent.X > 150.0f);
	const FVector WallScale = bIsCustomWall ? 
		FVector(CellSize / 400.0f, 1.0f, WallHeight / 300.0f) : 
		FVector((CellSize + 50.0f) / 100.0f, 50.0f / 100.0f, WallHeight / 100.0f);

	for (int32 Y = 0; Y < CurrentLayout.Height; ++Y)
	{
		for (int32 X = 0; X < CurrentLayout.Width; ++X)
		{
			if (!CurrentLayout.IsFloor(X, Y))
			{
				continue;
			}

			const FVector Center = CellToWorld(X, Y);

			FTransform Instance = FloorTransformBase;
			Instance.SetTranslation(Center);
			FloorInstances->AddInstance(Instance);

			if (CeilingMesh)
			{
				Instance.SetTranslation(Center + FVector(0.0f, 0.0f, WallHeight));
				Instance.SetRotation(FQuat(CeilingRotation));
				Instance.SetScale3D(FVector(CeilingScaleFactor, CeilingScaleFactor, 1.0f));
				CeilingInstances->AddInstance(Instance);
			}

			const int32 NeighborOffsets[4][2] = { { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };
			for (const auto& Offset : NeighborOffsets)
			{
				const int32 Nx = X + Offset[0];
				const int32 Ny = Y + Offset[1];
				if (CurrentLayout.IsFloor(Nx, Ny))
				{
					continue;
				}

				const FVector Dir(static_cast<float>(Offset[0]), static_cast<float>(Offset[1]), 0.0f);
				const float Yaw = (Offset[0] != 0) ? 90.0f : 0.0f;

				const FVector WallPos = Center + Dir * (CellSize * 0.5f) + (bIsCustomWall ? FVector::ZeroVector : FVector(0.0f, 0.0f, WallHeight * 0.5f));

				FTransform WallInstance(FRotator(0.0f, Yaw, 0.0f), WallPos, WallScale);
				WallInstances->AddInstance(WallInstance);
			}
		}
	}

	SpawnPillarsAndFixtures();

	FloorInstances->MarkRenderStateDirty();
	WallInstances->MarkRenderStateDirty();
	CeilingInstances->MarkRenderStateDirty();
	PillarInstances->MarkRenderStateDirty();
	CeilingLightInstances->MarkRenderStateDirty();
	PipeInstances->MarkRenderStateDirty();
	PropInstances->MarkRenderStateDirty();
	WaterInstances->MarkRenderStateDirty();

	for (TActorIterator<ASkyLight> SkyIt(World); SkyIt; ++SkyIt)
	{
		SkyIt->GetLightComponent()->RecaptureSky();
	}

	const bool bGameWorld = World->IsGameWorld();

	if (bGameWorld)
	{
		for (const FProcRoom& Room : CurrentLayout.Rooms)
		{
		FActorSpawnParameters LightParams;
		LightParams.ObjectFlags |= RF_Transient;

		APointLight* Lamp = World->SpawnActor<APointLight>(APointLight::StaticClass(),
			CellToWorld(Room.CenterX, Room.CenterY, WallHeight - 40.0f), FRotator::ZeroRotator, LightParams);

		if (!Lamp)
		{
			continue;
		}

		Lamp->SetMobility(EComponentMobility::Movable);

		if (UPointLightComponent* LightComponent = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
		{
			FLinearColor BiomeLightColor = FLinearColor(1.0f, 0.96f, 0.82f);
			float BiomeIntensity = 2000.0f;

			switch (Biome)
			{
			case ELevelBiome::Level0_YellowLobby:
				BiomeLightColor = FLinearColor(1.0f, 0.95f, 0.75f);
				BiomeIntensity = 2000.0f;
				break;
			case ELevelBiome::Level1_HabitableZone:
				BiomeLightColor = FLinearColor(0.85f, 0.92f, 1.0f);
				BiomeIntensity = 1600.0f;
				break;
			case ELevelBiome::Level2_PipeDreams:
				BiomeLightColor = FLinearColor(1.0f, 0.45f, 0.12f);
				BiomeIntensity = 1400.0f;
				break;
			case ELevelBiome::Level3_ElectricalStation:
				BiomeLightColor = FLinearColor(0.25f, 0.75f, 1.0f);
				BiomeIntensity = 1750.0f;
				break;
			case ELevelBiome::Level4_AbandonedOffice:
				BiomeLightColor = FLinearColor(0.88f, 0.96f, 0.94f);
				BiomeIntensity = 2200.0f;
				break;
			case ELevelBiome::Level6_LightsOut:
				BiomeLightColor = FLinearColor(0.01f, 0.01f, 0.02f);
				BiomeIntensity = 20.0f;
				break;
			case ELevelBiome::Level8_CaveSystem:
				BiomeLightColor = FLinearColor(0.45f, 0.4f, 0.35f);
				BiomeIntensity = 700.0f;
				break;
			case ELevelBiome::Level9_DarkSuburbs:
				BiomeLightColor = FLinearColor(0.2f, 0.25f, 0.55f);
				BiomeIntensity = 1000.0f;
				break;
			case ELevelBiome::Level10_WheatFields:
				BiomeLightColor = FLinearColor(1.0f, 0.7f, 0.35f);
				BiomeIntensity = 1900.0f;
				break;
			case ELevelBiome::Level37_Poolrooms:
				BiomeLightColor = FLinearColor(0.65f, 0.95f, 1.0f);
				BiomeIntensity = 1800.0f;
				break;
			case ELevelBiome::LevelRun_RunForYourLife:
				BiomeLightColor = FLinearColor(1.0f, 0.05f, 0.05f);
				BiomeIntensity = 3500.0f;
				break;
			default:
				break;
			}

			LightComponent->SetLightColor(BiomeLightColor);
			LightComponent->SetIntensity(BiomeIntensity);
			LightComponent->SetAttenuationRadius(
				FMath::Max(Room.SizeX, Room.SizeY) * CellSize * 0.75f);
		}

		
		FRandomStream FlickerStream(static_cast<int32>(static_cast<uint32>(Seed) ^ (static_cast<uint32>(Room.CenterX) * 73856093u) ^ (static_cast<uint32>(Room.CenterY) * 19349663u)));
		if (FlickerStream.FRand() < 0.3f)
		{
			ULiminalFlickerLightComponent* Flicker = NewObject<ULiminalFlickerLightComponent>(Lamp);
			Flicker->RegisterComponent();
		}
		CosmeticActors.Add(Lamp);
		}

		AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (Fog && Fog->GetComponent())
		{
			Fog->GetComponent()->bEnableVolumetricFog = true;
			float FogDensity = 0.02f;
			if (Biome == ELevelBiome::Level0_YellowLobby || Biome == ELevelBiome::Level8_CaveSystem) FogDensity = 0.1f;
			else if (Biome == ELevelBiome::Level37_Poolrooms || Biome == ELevelBiome::Level4_AbandonedOffice) FogDensity = 0.005f;
			Fog->GetComponent()->SetFogDensity(FogDensity);
			CosmeticActors.Add(Fog);
		}

		APostProcessVolume* PPVolume = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		if (PPVolume)
		{
			PPVolume->bUnbound = true;
			FPostProcessSettings& Settings = PPVolume->Settings;
			if (Biome == ELevelBiome::Level0_YellowLobby)
			{
				Settings.bOverride_ColorSaturation = true;
				Settings.ColorSaturation = FVector4(1.2f, 1.1f, 0.9f, 1.0f);
				Settings.bOverride_FilmGrainIntensity = true;
				Settings.FilmGrainIntensity = 0.5f;
				Settings.bOverride_VignetteIntensity = true;
				Settings.VignetteIntensity = 0.4f;
			}
			else if (Biome == ELevelBiome::Level8_CaveSystem)
			{
				Settings.bOverride_ColorContrast = true;
				Settings.ColorContrast = FVector4(1.3f, 1.3f, 1.3f, 1.0f);
				Settings.bOverride_VignetteIntensity = true;
				Settings.VignetteIntensity = 0.8f;
			}
			else if (Biome == ELevelBiome::Level37_Poolrooms)
			{
				Settings.bOverride_ColorSaturation = true;
				Settings.ColorSaturation = FVector4(0.8f, 0.9f, 1.2f, 1.0f);
				Settings.bOverride_BloomIntensity = true;
				Settings.BloomIntensity = 1.5f;
				Settings.bOverride_SceneFringeIntensity = true;
				Settings.SceneFringeIntensity = 2.0f;
			}
			else if (Biome == ELevelBiome::LevelRun_RunForYourLife || Biome == ELevelBiome::Level4_AbandonedOffice)
			{
				Settings.bOverride_ColorSaturation = true;
				Settings.ColorSaturation = FVector4(0.6f, 0.6f, 0.6f, 1.0f);
				Settings.bOverride_ColorContrast = true;
				Settings.ColorContrast = FVector4(1.4f, 1.4f, 1.4f, 1.0f);
				Settings.bOverride_MotionBlurAmount = true;
				Settings.MotionBlurAmount = 0.8f;
			}
			CosmeticActors.Add(PPVolume);
		}
	}
}

void ALiminalLevelGenerator::ClearSpawnedActors()
{
	for (AActor* Actor : SpawnedActors)
	{
		if (IsValid(Actor))
		{
			Actor->Destroy();
		}
	}
	SpawnedActors.Empty();
}

void ALiminalLevelGenerator::SpawnGameplayActors()
{
	UWorld* World = GetWorld();
	if (!World || !HasAuthority() || CurrentLayout.Rooms.Num() == 0)
	{
		return;
	}

	SpawnPlayerStarts();
	SpawnEnvironmentalProps();

	if (!CurrentLayout.Rooms.IsValidIndex(CurrentLayout.ExtractionRoomIndex)) return;
	SpawnExtraction(CurrentLayout.Rooms[CurrentLayout.ExtractionRoomIndex]);

	SpawnLoots();
	SpawnHounds();

	if (Biome == ELevelBiome::Level0_YellowLobby)
	{
		SpawnSmilers();
		SpawnDullers();
	}
	else if (Biome == ELevelBiome::Level1_HabitableZone)
	{
		SpawnClumps();
		SpawnDullers();
	}
	else if (Biome == ELevelBiome::Level2_PipeDreams)
	{
		SpawnClumps();
		SpawnWretches();
	}
	else if (Biome == ELevelBiome::Level3_ElectricalStation)
	{
		SpawnSmilers();
		SpawnDullers();
	}
	else if (Biome == ELevelBiome::Level4_AbandonedOffice)
	{
		SpawnWatchers();
		SpawnSmilers();
		SpawnPartygoers();
	}
	else if (Biome == ELevelBiome::Level6_LightsOut)
	{
		SpawnWretches();
		SpawnSmilers();
		SpawnDullers();
	}
	else if (Biome == ELevelBiome::Level8_CaveSystem)
	{
		SpawnDeathmoths();
		SpawnClumps();
	}
	else if (Biome == ELevelBiome::Level9_DarkSuburbs)
	{
		SpawnWretches();
		SpawnSkinwalkers();
	}
	else if (Biome == ELevelBiome::Level10_WheatFields)
	{
		SpawnDeathmoths();
		SpawnSkinwalkers();
	}
	else if (Biome == ELevelBiome::Level37_Poolrooms)
	{
		SpawnSmilers();
		SpawnJerrys();
	}
	else if (Biome == ELevelBiome::LevelRun_RunForYourLife)
	{
		SpawnSmilers();
		SpawnWretches();
		SpawnPartygoers();
	}

	SpawnNonEuclideanPortals();
	SetupNavMeshBounds();

	const FProcRoom& SpawnRoom = CurrentLayout.Rooms[0];
	const FVector SafeSpawnLocation = CellToWorld(SpawnRoom.CenterX, SpawnRoom.CenterY, 110.0f);

	for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PlayerController = Iterator->Get())
		{
			if (APawn* ExistingPawn = PlayerController->GetPawn())
			{
				ExistingPawn->TeleportTo(SafeSpawnLocation, FRotator::ZeroRotator, false, true);
			}
			else if (AGameModeBase* GameMode = World->GetAuthGameMode())
			{
				GameMode->RestartPlayer(PlayerController);
			}
		}
	}
}

void ALiminalLevelGenerator::SpawnPlayerStarts()
{
	UWorld* World = GetWorld();
	const FProcRoom& FirstRoom = CurrentLayout.Rooms[0];

	for (int32 Index = 0; Index < 2; ++Index)
	{
		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;

		APlayerStart* Start = World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
			CellToWorld(FirstRoom.CenterX, FirstRoom.CenterY, 100.0f) + FVector(0.0f, Index * 150.0f - 75.0f, 0.0f),
			FRotator::ZeroRotator, Params);

		if (Start)
		{
			Start->PlayerStartTag = FName(*FString::Printf(TEXT("Scavenger%d"), Index + 1));
			SpawnedActors.Add(Start);
		}
	}
}

void ALiminalLevelGenerator::SpawnExtraction(const FProcRoom& FarthestRoom)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.ObjectFlags |= RF_Transient;

	AExtractionZone* Zone = World->SpawnActor<AExtractionZone>(AExtractionZone::StaticClass(),
		CellToWorld(FarthestRoom.CenterX, FarthestRoom.CenterY, 100.0f),
		FRotator::ZeroRotator, Params);

	if (!Zone)
	{
		return;
	}
	SpawnedActors.Add(Zone);

	// Puzzles d'extraction proceduraux (Disjoncteur / Clavier a code)
	// Si le niveau possede au moins 3 salles, on place un disjoncteur dans une salle intermediaire
	if (CurrentLayout.Rooms.Num() >= 3)
	{
		const int32 BreakerRoomIndex = FMath::Clamp(CurrentLayout.Rooms.Num() / 2, 1, CurrentLayout.Rooms.Num() - 1);
		const FProcRoom& BreakerRoom = CurrentLayout.Rooms[BreakerRoomIndex];
		const FVector BreakerLoc = CellToWorld(BreakerRoom.CenterX, BreakerRoom.CenterY, 120.0f);

		ALiminalBreakerActor* Breaker = World->SpawnActor<ALiminalBreakerActor>(
			ALiminalBreakerActor::StaticClass(),
			BreakerLoc,
			FRotator::ZeroRotator,
			Params);

		if (Breaker)
		{
			SpawnedActors.Add(Breaker);
			Zone->SetRequiresPower(true, Breaker);
		}
	}

	// Si le niveau est vaste (>= 6 salles) dans un complexe de bureaux ou electrique, ajout d'un digicode de securite
	if (CurrentLayout.Rooms.Num() >= 6 &&
		(Biome == ELevelBiome::Level3_ElectricalStation || Biome == ELevelBiome::Level4_AbandonedOffice || Biome == ELevelBiome::Level1_HabitableZone))
	{
		const int32 KeypadRoomIndex = FMath::Clamp(CurrentLayout.Rooms.Num() / 3, 1, CurrentLayout.Rooms.Num() - 1);
		if (KeypadRoomIndex != (CurrentLayout.Rooms.Num() / 2))
		{
			const FProcRoom& KeypadRoom = CurrentLayout.Rooms[KeypadRoomIndex];
			const FVector KeypadLoc = CellToWorld(KeypadRoom.CenterX, KeypadRoom.CenterY, 130.0f);

			ALiminalKeypadActor* Keypad = World->SpawnActor<ALiminalKeypadActor>(
				ALiminalKeypadActor::StaticClass(),
				KeypadLoc,
				FRotator::ZeroRotator,
				Params);

			if (Keypad)
			{
				FRandomStream CodeStream(Seed ^ 0x5A5A);
				const int32 D1 = CodeStream.RandRange(1, 9);
				const int32 D2 = CodeStream.RandRange(0, 9);
				const int32 D3 = CodeStream.RandRange(0, 9);
				const int32 D4 = CodeStream.RandRange(0, 9);
				const FString PinCode = FString::Printf(TEXT("%d%d%d%d"), D1, D2, D3, D4);
				Keypad->SetTargetCode(PinCode);

				SpawnedActors.Add(Keypad);
				Zone->SetRequiresKeypad(true, Keypad);
			}
		}
	}

	// Puzzle de vanne a vapeur sous pression (Pipe Dreams - Level 2)
	if (CurrentLayout.Rooms.Num() >= 3 && Biome == ELevelBiome::Level2_PipeDreams)
	{
		const int32 ValveRoomIndex = FMath::Clamp(CurrentLayout.Rooms.Num() / 3, 1, CurrentLayout.Rooms.Num() - 1);
		const FProcRoom& ValveRoom = CurrentLayout.Rooms[ValveRoomIndex];
		const FVector ValveLoc = CellToWorld(ValveRoom.CenterX, ValveRoom.CenterY, 120.0f);

		ALiminalValvePuzzleActor* ValveActor = World->SpawnActor<ALiminalValvePuzzleActor>(
			ALiminalValvePuzzleActor::StaticClass(),
			ValveLoc,
			FRotator::ZeroRotator,
			Params);

		if (ValveActor)
		{
			SpawnedActors.Add(ValveActor);
		}
	}

	// Boitier de fusibles M.E.G. (Habitable Zone - Level 1 & Station Electrique - Level 3)
	if (CurrentLayout.Rooms.Num() >= 4 &&
		(Biome == ELevelBiome::Level3_ElectricalStation || Biome == ELevelBiome::Level1_HabitableZone))
	{
		const int32 FuseRoomIndex = FMath::Clamp((CurrentLayout.Rooms.Num() * 2) / 3, 1, CurrentLayout.Rooms.Num() - 1);
		const FProcRoom& FuseRoom = CurrentLayout.Rooms[FuseRoomIndex];
		const FVector FuseLoc = CellToWorld(FuseRoom.CenterX, FuseRoom.CenterY, 120.0f);

		ALiminalFuseBoxActor* FuseBox = World->SpawnActor<ALiminalFuseBoxActor>(
			ALiminalFuseBoxActor::StaticClass(),
			FuseLoc,
			FRotator::ZeroRotator,
			Params);

		if (FuseBox)
		{
			SpawnedActors.Add(FuseBox);
		}
	}

	// Verrouillage de securite et cle physique M.E.G. dans les niveaux etendus (>= 4 salles)
	if (CurrentLayout.Rooms.Num() >= 4)
	{
		const int32 KeyRoomIndex = 1;
		const FProcRoom& KeyRoom = CurrentLayout.Rooms[KeyRoomIndex];
		const FVector KeyLoc = CellToWorld(KeyRoom.CenterX, KeyRoom.CenterY, 50.0f);

		ALiminalKeyItemActor* KeyItem = World->SpawnActor<ALiminalKeyItemActor>(
			ALiminalKeyItemActor::StaticClass(),
			KeyLoc,
			FRotator::ZeroRotator,
			Params);

		const FName ExtractionKeyTag(TEXT("Key_SectorA"));
		if (KeyItem)
		{
			KeyItem->SetKeyData(ExtractionKeyTag, EKeycardLevel::Level2_Maintenance, FText::FromString(TEXT("Pass M.E.G. Secteur Extraction")));
			SpawnedActors.Add(KeyItem);
		}

		// Porte blindee verouillee a l'entree de la zone d'extraction
		const FVector DoorLoc = CellToWorld(FarthestRoom.CenterX, FarthestRoom.OriginY, 100.0f);
		ALiminalDoorActor* SectorDoor = World->SpawnActor<ALiminalDoorActor>(
			ALiminalDoorActor::StaticClass(),
			DoorLoc,
			FRotator::ZeroRotator,
			Params);

		if (SectorDoor)
		{
			SectorDoor->RequiredKeyTag = ExtractionKeyTag;
			SectorDoor->Lock();
			SpawnedActors.Add(SectorDoor);
		}
	}
}

void ALiminalLevelGenerator::SpawnLoots()
{
	UWorld* World = GetWorld();
	if (!LootClass)
	{
		return;
	}

	FRandomStream LootStream(Seed * 31 + 7);

	TArray<FIntPoint> Candidates;
	for (const FProcRoom& Room : CurrentLayout.Rooms)
	{
		for (int32 Y = Room.OriginY; Y < Room.OriginY + Room.SizeY; ++Y)
		{
			for (int32 X = Room.OriginX; X < Room.OriginX + Room.SizeX; ++X)
			{
				Candidates.Add(FIntPoint(X, Y));
			}
		}
	}

	UDataTable* LootTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/DT_LootItems.DT_LootItems"));
	TArray<FLootItem*> LootRows;
	if (LootTable)
	{
		LootTable->GetAllRows<FLootItem>(TEXT("LiminalLevelGenerator"), LootRows);
	}

	TArray<FIntPoint> Chosen;
	for (int32 Attempt = 0; Attempt < LootCount * 20 && Chosen.Num() < LootCount && Candidates.Num() > 0; ++Attempt)
	{
		const int32 Index = LootStream.RandRange(0, Candidates.Num() - 1);
		const FIntPoint Candidate = Candidates[Index];

		bool bTooClose = false;
		for (const FIntPoint& ChosenCell : Chosen)
		{
			if (FMath::Abs(ChosenCell.X - Candidate.X) + FMath::Abs(ChosenCell.Y - Candidate.Y) < 3)
			{
				bTooClose = true;
				break;
			}
		}
		if (bTooClose)
		{
			continue;
		}

		Chosen.Add(Candidate);
		Candidates.RemoveAtSwap(Index, 1, EAllowShrinking::No);

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;

		ALootActor* Loot = World->SpawnActor<ALootActor>(LootClass,
			CellToWorld(Candidate.X, Candidate.Y, 60.0f), FRotator(0.0f, LootStream.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Loot)
		{
			if (LootRows.Num() > 0)
			{
				const int32 RowIdx = LootStream.RandRange(0, LootRows.Num() - 1);
				const FLootItem* ItemDef = LootRows[RowIdx];
				if (ItemDef)
				{
					UStaticMesh* LoadedMesh = ItemDef->Mesh.LoadSynchronous();
					USoundBase* LoadedSound = ItemDef->CollisionSound.LoadSynchronous();
					const float Variance = LootStream.FRandRange(0.85f, 1.15f);
					const float Weight = FMath::Max(0.2f, ItemDef->WeightKg * Variance);
					const int32 Credits = FMath::Max(5, FMath::RoundToInt(ItemDef->CreditsValue * Variance));
					Loot->SetCustomLoot(LoadedMesh, LoadedSound, Weight, Credits, ItemDef->ItemId);
				}
			}
			else
			{
				const float RandomWeight = LootStream.FRandRange(3.0f, 25.0f);
				const int32 RandomCredits = LootStream.RandRange(40, 350);
				Loot->SetRandomizedStats(RandomWeight, RandomCredits);
			}

			SpawnedActors.Add(Loot);
		}
	}
}

void ALiminalLevelGenerator::SpawnHounds()
{
	UWorld* World = GetWorld();
	if (CurrentLayout.Rooms.Num() < 2 || HoundCount <= 0)
	{
		return;
	}

	UClass* HoundClass = HoundPawnClass.LoadSynchronous();
	if (!HoundClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("LiminalLevelGenerator : classe hound introuvable (%s)"),
			*HoundPawnClass.ToString());
		return;
	}

	FRandomStream HoundStream(Seed * 17 + 3);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledHoundCount = FMath::RoundToInt(HoundCount * ScaleFactor);

	const FVector PlayerSpawnPos = CellToWorld(CurrentLayout.Rooms[0].CenterX, CurrentLayout.Rooms[0].CenterY, 100.0f);
	constexpr float MinSafetyDistance = 3500.0f; // 35 metres minimum de securite spawn

	TArray<int32> SafeRoomIndices;
	int32 FurthestRoomIndex = 1;
	float MaxDistSq = 0.0f;
	for (int32 R = 1; R < CurrentLayout.Rooms.Num(); ++R)
	{
		const FVector RoomPos = CellToWorld(CurrentLayout.Rooms[R].CenterX, CurrentLayout.Rooms[R].CenterY, 100.0f);
		const float DistSq = FVector::DistSquared(RoomPos, PlayerSpawnPos);
		if (DistSq > MaxDistSq)
		{
			MaxDistSq = DistSq;
			FurthestRoomIndex = R;
		}
		if (DistSq >= FMath::Square(MinSafetyDistance))
		{
			SafeRoomIndices.Add(R);
		}
	}
	if (SafeRoomIndices.Num() == 0)
	{
		SafeRoomIndices.Add(FurthestRoomIndex);
	}

	for (int32 Index = 0; Index < ScaledHoundCount; ++Index)
	{
		const int32 PickedIdx = SafeRoomIndices[HoundStream.RandRange(0, SafeRoomIndices.Num() - 1)];
		const FProcRoom& Room = CurrentLayout.Rooms[PickedIdx];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride =
			ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Hound = World->SpawnActor<APawn>(HoundClass,
			CellToWorld(Room.CenterX, Room.CenterY, 100.0f), FRotator(0.0f, HoundStream.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Hound)
		{
			SpawnedActors.Add(Hound);
		}
	}
}

void ALiminalLevelGenerator::SpawnSmilers()
{
	UWorld* World = GetWorld();
	if (CurrentLayout.Rooms.Num() < 2 || SmilerCount <= 0)
	{
		return;
	}

	UClass* SmilerClass = SmilerPawnClass.LoadSynchronous();
	if (!SmilerClass)
	{
		SmilerClass = ALiminalEntity_Smiler::StaticClass();
	}

	FRandomStream SmilerStream(Seed * 29 + 7);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledSmilerCount = FMath::RoundToInt(SmilerCount * ScaleFactor);

	const FVector PlayerSpawnPos = CellToWorld(CurrentLayout.Rooms[0].CenterX, CurrentLayout.Rooms[0].CenterY, 100.0f);
	constexpr float MinSafetyDistance = 3500.0f; // 35 metres minimum de securite spawn

	TArray<int32> SafeRoomIndices;
	int32 FurthestRoomIndex = 1;
	float MaxDistSq = 0.0f;
	for (int32 R = 1; R < CurrentLayout.Rooms.Num(); ++R)
	{
		const FVector RoomPos = CellToWorld(CurrentLayout.Rooms[R].CenterX, CurrentLayout.Rooms[R].CenterY, 100.0f);
		const float DistSq = FVector::DistSquared(RoomPos, PlayerSpawnPos);
		if (DistSq > MaxDistSq)
		{
			MaxDistSq = DistSq;
			FurthestRoomIndex = R;
		}
		if (DistSq >= FMath::Square(MinSafetyDistance))
		{
			SafeRoomIndices.Add(R);
		}
	}
	if (SafeRoomIndices.Num() == 0)
	{
		SafeRoomIndices.Add(FurthestRoomIndex);
	}

	for (int32 Index = 0; Index < ScaledSmilerCount; ++Index)
	{
		const int32 PickedIdx = SafeRoomIndices[SmilerStream.RandRange(0, SafeRoomIndices.Num() - 1)];
		const FProcRoom& Room = CurrentLayout.Rooms[PickedIdx];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Smiler = World->SpawnActor<APawn>(SmilerClass,
			CellToWorld(Room.CenterX, Room.CenterY, 100.0f), FRotator(0.0f, SmilerStream.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Smiler)
		{
			SpawnedActors.Add(Smiler);
		}
	}
}

void ALiminalLevelGenerator::SpawnClumps()
{
	UWorld* World = GetWorld();
	if (CurrentLayout.Rooms.Num() < 2 || ClumpCount <= 0)
	{
		return;
	}

	UClass* ClumpClass = ClumpPawnClass.LoadSynchronous();
	if (!ClumpClass)
	{
		ClumpClass = ALiminalEntity_Clump::StaticClass();
	}

	FRandomStream ClumpStream(Seed * 43 + 11);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledClumpCount = FMath::RoundToInt(ClumpCount * ScaleFactor);

	const FVector PlayerSpawnPos = CellToWorld(CurrentLayout.Rooms[0].CenterX, CurrentLayout.Rooms[0].CenterY, 100.0f);
	constexpr float MinSafetyDistance = 3500.0f; // 35 metres minimum de securite spawn

	TArray<int32> SafeRoomIndices;
	int32 FurthestRoomIndex = 1;
	float MaxDistSq = 0.0f;
	for (int32 R = 1; R < CurrentLayout.Rooms.Num(); ++R)
	{
		const FVector RoomPos = CellToWorld(CurrentLayout.Rooms[R].CenterX, CurrentLayout.Rooms[R].CenterY, 100.0f);
		const float DistSq = FVector::DistSquared(RoomPos, PlayerSpawnPos);
		if (DistSq > MaxDistSq)
		{
			MaxDistSq = DistSq;
			FurthestRoomIndex = R;
		}
		if (DistSq >= FMath::Square(MinSafetyDistance))
		{
			SafeRoomIndices.Add(R);
		}
	}
	if (SafeRoomIndices.Num() == 0)
	{
		SafeRoomIndices.Add(FurthestRoomIndex);
	}

	for (int32 Index = 0; Index < ScaledClumpCount; ++Index)
	{
		const int32 PickedIdx = SafeRoomIndices[ClumpStream.RandRange(0, SafeRoomIndices.Num() - 1)];
		const FProcRoom& Room = CurrentLayout.Rooms[PickedIdx];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Clump = World->SpawnActor<APawn>(ClumpClass,
			CellToWorld(Room.OriginX + 1, Room.OriginY + 1, 60.0f), FRotator::ZeroRotator, Params);

		if (Clump)
		{
			SpawnedActors.Add(Clump);
		}
	}
}

void ALiminalLevelGenerator::SpawnWatchers()
{
	UWorld* World = GetWorld();
	if (CurrentLayout.Rooms.Num() < 2 || WatcherCount <= 0)
	{
		return;
	}

	UClass* WatcherClass = WatcherPawnClass.LoadSynchronous();
	if (!WatcherClass)
	{
		WatcherClass = ALiminalEntity_Watcher::StaticClass();
	}

	FRandomStream WatcherStream(Seed * 53 + 19);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledWatcherCount = FMath::RoundToInt(WatcherCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledWatcherCount; ++Index)
	{
		const int32 RoomIndex = 1 + (WatcherStream.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Watcher = World->SpawnActor<APawn>(WatcherClass,
			CellToWorld(Room.CenterX, Room.CenterY, 60.0f),
			FRotator(0.0f, WatcherStream.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Watcher)
		{
			SpawnedActors.Add(Watcher);
		}
	}
}

void ALiminalLevelGenerator::SpawnWretches()
{
	UWorld* World = GetWorld();
	if (CurrentLayout.Rooms.Num() < 2 || WretchCount <= 0)
	{
		return;
	}

	UClass* WretchClass = WretchPawnClass.LoadSynchronous();
	if (!WretchClass)
	{
		WretchClass = ALiminalEntity_Wretch::StaticClass();
	}

	FRandomStream WretchStream(Seed * 67 + 23);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledWretchCount = FMath::RoundToInt(WretchCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledWretchCount; ++Index)
	{
		const int32 RoomIndex = 1 + (WretchStream.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Wretch = World->SpawnActor<APawn>(WretchClass,
			CellToWorld(Room.CenterX, Room.CenterY, 60.0f),
			FRotator(0.0f, WretchStream.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Wretch)
		{
			SpawnedActors.Add(Wretch);
		}
	}
}

void ALiminalLevelGenerator::SpawnDeathmoths()
{
	UWorld* World = GetWorld();
	if (CurrentLayout.Rooms.Num() < 2 || DeathmothCount <= 0)
	{
		return;
	}

	UClass* DeathmothClass = DeathmothPawnClass.LoadSynchronous();
	if (!DeathmothClass)
	{
		DeathmothClass = ALiminalEntity_Deathmoth::StaticClass();
	}

	FRandomStream DeathmothStream(Seed * 79 + 31);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledDeathmothCount = FMath::RoundToInt(DeathmothCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledDeathmothCount; ++Index)
	{
		const int32 RoomIndex = 1 + (DeathmothStream.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Deathmoth = World->SpawnActor<APawn>(DeathmothClass,
			CellToWorld(Room.CenterX, Room.CenterY, 150.0f),
			FRotator(0.0f, DeathmothStream.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Deathmoth)
		{
			SpawnedActors.Add(Deathmoth);
		}
	}
}

void ALiminalLevelGenerator::SpawnSkinwalkers()
{
	UWorld* World = GetWorld();
	if (!World || CurrentLayout.Rooms.Num() < 2)
	{
		return;
	}

	UClass* SkinwalkerClass = SkinwalkerPawnClass.LoadSynchronous();
	if (!SkinwalkerClass)
	{
		SkinwalkerClass = ALiminalEntity_Skinwalker::StaticClass();
	}

	FRandomStream Rnd(Seed * 83 + 17);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledSkinwalkerCount = FMath::RoundToInt(SkinwalkerCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledSkinwalkerCount; ++Index)
	{
		const int32 RoomIndex = 1 + (Rnd.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Entity = World->SpawnActor<APawn>(SkinwalkerClass,
			CellToWorld(Room.CenterX, Room.CenterY, 90.0f),
			FRotator(0.0f, Rnd.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Entity)
		{
			SpawnedActors.Add(Entity);
		}
	}
}

void ALiminalLevelGenerator::SpawnPartygoers()
{
	UWorld* World = GetWorld();
	if (!World || CurrentLayout.Rooms.Num() < 2)
	{
		return;
	}

	UClass* PartygoerClass = PartygoerPawnClass.LoadSynchronous();
	if (!PartygoerClass)
	{
		PartygoerClass = ALiminalEntity_Partygoer::StaticClass();
	}

	FRandomStream Rnd(Seed * 89 + 23);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledPartygoerCount = FMath::RoundToInt(PartygoerCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledPartygoerCount; ++Index)
	{
		const int32 RoomIndex = 1 + (Rnd.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Entity = World->SpawnActor<APawn>(PartygoerClass,
			CellToWorld(Room.CenterX, Room.CenterY, 90.0f),
			FRotator(0.0f, Rnd.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Entity)
		{
			SpawnedActors.Add(Entity);
		}
	}
}

void ALiminalLevelGenerator::SpawnDullers()
{
	UWorld* World = GetWorld();
	if (!World || CurrentLayout.Rooms.Num() < 2)
	{
		return;
	}

	UClass* DullerClass = DullerPawnClass.LoadSynchronous();
	if (!DullerClass)
	{
		DullerClass = ALiminalEntity_Duller::StaticClass();
	}

	FRandomStream Rnd(Seed * 97 + 29);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledDullerCount = FMath::RoundToInt(DullerCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledDullerCount; ++Index)
	{
		const int32 RoomIndex = 1 + (Rnd.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Entity = World->SpawnActor<APawn>(DullerClass,
			CellToWorld(Room.CenterX, Room.CenterY, 90.0f),
			FRotator(0.0f, Rnd.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Entity)
		{
			SpawnedActors.Add(Entity);
		}
	}
}

void ALiminalLevelGenerator::SpawnJerrys()
{
	UWorld* World = GetWorld();
	if (!World || CurrentLayout.Rooms.Num() < 2)
	{
		return;
	}

	UClass* JerryClass = JerryPawnClass.LoadSynchronous();
	if (!JerryClass)
	{
		JerryClass = ALiminalEntity_Jerry::StaticClass();
	}

	FRandomStream Rnd(Seed * 101 + 37);
	const float ScaleFactor = FMath::Clamp(GridWidth / 24.0f, 1.0f, 3.0f);
	const int32 ScaledJerryCount = FMath::RoundToInt(JerryCount * ScaleFactor);

	for (int32 Index = 0; Index < ScaledJerryCount; ++Index)
	{
		const int32 RoomIndex = 1 + (Rnd.RandRange(0, CurrentLayout.Rooms.Num() - 2));
		const FProcRoom& Room = CurrentLayout.Rooms[RoomIndex % CurrentLayout.Rooms.Num()];

		FActorSpawnParameters Params;
		Params.ObjectFlags |= RF_Transient;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		APawn* Entity = World->SpawnActor<APawn>(JerryClass,
			CellToWorld(Room.CenterX, Room.CenterY, 120.0f),
			FRotator(0.0f, Rnd.FRandRange(0.0f, 360.0f), 0.0f), Params);

		if (Entity)
		{
			SpawnedActors.Add(Entity);
		}
	}
}

void ALiminalLevelGenerator::ApplyBiomeMaterials()
{
	UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/MI_PrototypeGrid_Gray.MI_PrototypeGrid_Gray"));
	if (!BaseMat)
	{
		BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/M_PrototypeGrid.M_PrototypeGrid"));
	}
	if (!BaseMat)
	{
		BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/M_FlatCol.M_FlatCol"));
	}
	if (!BaseMat)
	{
		BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (!BaseMat)
	{
		BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial"));
	}
	if (!BaseMat)
	{
		return;
	}

	FString WallTexName;
	FString FloorTexName;
	FString CeilingTexName;
	FString PillarTexName;
	FLinearColor WallTint = FLinearColor::White;
	FLinearColor FloorTint = FLinearColor::White;
	FLinearColor CeilingTint = FLinearColor::White;
	FLinearColor PillarTint = FLinearColor::White;

	FLinearColor FloorBaseColor = FLinearColor::White;
	FLinearColor FloorGridColor = FLinearColor(0.2f, 0.2f, 0.2f);
	float FloorSpacing = 100.0f;
	float FloorRoughness = 0.8f;

	FLinearColor WallBaseColor = FLinearColor::White;
	FLinearColor WallGridColor = FLinearColor(0.2f, 0.2f, 0.2f);
	float WallSpacing = 100.0f;
	float WallRoughness = 0.8f;

	FLinearColor CeilingBaseColor = FLinearColor::White;
	FLinearColor CeilingGridColor = FLinearColor(0.3f, 0.3f, 0.3f);
	float CeilingSpacing = 200.0f;
	float CeilingRoughness = 0.9f;

	FLinearColor PillarBaseColor = FLinearColor::White;
	FLinearColor PillarGridColor = FLinearColor(0.2f, 0.2f, 0.2f);
	float PillarSpacing = 100.0f;

	FLinearColor LightGlowColor = FLinearColor(2.2f, 2.1f, 1.6f);

	switch (Biome)
	{
	case ELevelBiome::Level0_YellowLobby:
		// Iconic Mono-Yellow striped wallpaper & damp loop carpet
		WallTexName = TEXT("T_Lobby_Wallpaper.png");
		FloorTexName = TEXT("T_Lobby_Carpet.png");
		CeilingTexName = TEXT("T_Lobby_Ceiling.png");
		PillarTexName = TEXT("T_Lobby_Wallpaper.png");

		WallBaseColor = FLinearColor(0.88f, 0.78f, 0.38f);
		WallGridColor = FLinearColor(0.70f, 0.60f, 0.25f);
		WallSpacing = 80.0f;
		WallRoughness = 0.85f;

		FloorBaseColor = FLinearColor(0.40f, 0.34f, 0.18f);
		FloorGridColor = FLinearColor(0.24f, 0.19f, 0.08f);
		FloorSpacing = 40.0f;
		FloorRoughness = 0.95f;

		CeilingBaseColor = FLinearColor(0.88f, 0.88f, 0.82f);
		CeilingGridColor = FLinearColor(0.42f, 0.42f, 0.40f);
		CeilingSpacing = 200.0f;
		CeilingRoughness = 0.90f;

		PillarBaseColor = FLinearColor(0.82f, 0.72f, 0.35f);
		PillarGridColor = FLinearColor(0.68f, 0.58f, 0.26f);
		PillarSpacing = 100.0f;
		LightGlowColor = FLinearColor(2.2f, 2.1f, 1.6f);
		break;

	case ELevelBiome::Level1_HabitableZone:
		// Concrete industrial walls and damp concrete with expansion joints
		WallTexName = TEXT("T_Concrete_Industrial.png");
		FloorTexName = TEXT("T_Concrete_Industrial.png");
		CeilingTexName = TEXT("T_Concrete_Industrial.png");
		PillarTexName = TEXT("T_Concrete_Industrial.png");
		WallTint = FLinearColor(1.05f, 1.05f, 1.05f);
		FloorTint = FLinearColor(0.85f, 0.85f, 0.85f);
		CeilingTint = FLinearColor(0.70f, 0.70f, 0.75f);
		PillarTint = FLinearColor(0.90f, 0.90f, 0.90f);

		WallBaseColor = FLinearColor(0.38f, 0.40f, 0.42f);
		WallGridColor = FLinearColor(0.22f, 0.24f, 0.26f);
		WallSpacing = 200.0f;
		WallRoughness = 0.80f;

		FloorBaseColor = FLinearColor(0.26f, 0.27f, 0.28f);
		FloorGridColor = FLinearColor(0.12f, 0.13f, 0.14f);
		FloorSpacing = 400.0f;
		FloorRoughness = 0.65f;

		CeilingBaseColor = FLinearColor(0.20f, 0.22f, 0.24f);
		CeilingGridColor = FLinearColor(0.12f, 0.13f, 0.15f);
		CeilingSpacing = 200.0f;
		CeilingRoughness = 0.85f;

		PillarBaseColor = FLinearColor(0.32f, 0.34f, 0.36f);
		PillarGridColor = FLinearColor(0.18f, 0.20f, 0.22f);
		PillarSpacing = 100.0f;
		LightGlowColor = FLinearColor(1.6f, 1.8f, 2.2f);
		break;

	case ELevelBiome::Level2_PipeDreams:
		// Heavy rust and corrugated metal
		WallTexName = TEXT("T_Corrugated_Rust.png");
		FloorTexName = TEXT("T_Concrete_Industrial.png");
		CeilingTexName = TEXT("T_Corrugated_Rust.png");
		PillarTexName = TEXT("T_Corrugated_Rust.png");
		FloorTint = FLinearColor(0.75f, 0.65f, 0.55f);
		CeilingTint = FLinearColor(0.80f, 0.80f, 0.80f);

		WallBaseColor = FLinearColor(0.42f, 0.22f, 0.12f);
		WallGridColor = FLinearColor(0.15f, 0.08f, 0.04f);
		WallSpacing = 100.0f;
		WallRoughness = 0.60f;

		FloorBaseColor = FLinearColor(0.22f, 0.16f, 0.12f);
		FloorGridColor = FLinearColor(0.08f, 0.05f, 0.03f);
		FloorSpacing = 80.0f;
		FloorRoughness = 0.55f;

		CeilingBaseColor = FLinearColor(0.15f, 0.12f, 0.10f);
		CeilingGridColor = FLinearColor(0.06f, 0.04f, 0.03f);
		CeilingSpacing = 100.0f;
		CeilingRoughness = 0.70f;

		PillarBaseColor = FLinearColor(0.35f, 0.18f, 0.10f);
		PillarGridColor = FLinearColor(0.14f, 0.07f, 0.03f);
		PillarSpacing = 60.0f;
		LightGlowColor = FLinearColor(2.4f, 1.2f, 0.3f);
		break;

	case ELevelBiome::Level3_ElectricalStation:
		// Charcoal alloy with high-voltage cyan grid lines and electrical warning panels
		WallTexName = TEXT("T_Electrical_Panel.png");
		FloorTexName = TEXT("T_Concrete_Industrial.png");
		CeilingTexName = TEXT("T_Concrete_Industrial.png");
		PillarTexName = TEXT("T_Electrical_Panel.png");
		WallTint = FLinearColor(0.35f, 0.40f, 0.45f);
		FloorTint = FLinearColor(0.45f, 0.50f, 0.55f);
		CeilingTint = FLinearColor(0.25f, 0.30f, 0.35f);
		PillarTint = FLinearColor(0.35f, 0.40f, 0.45f);

		WallBaseColor = FLinearColor(0.10f, 0.12f, 0.14f);
		WallGridColor = FLinearColor(0.0f, 0.8f, 1.0f);
		WallSpacing = 50.0f;
		WallRoughness = 0.25f;

		FloorBaseColor = FLinearColor(0.18f, 0.20f, 0.22f);
		FloorGridColor = FLinearColor(0.0f, 0.5f, 0.8f);
		FloorSpacing = 100.0f;
		FloorRoughness = 0.35f;

		CeilingBaseColor = FLinearColor(0.10f, 0.11f, 0.12f);
		CeilingGridColor = FLinearColor(0.0f, 0.3f, 0.5f);
		CeilingSpacing = 100.0f;
		CeilingRoughness = 0.40f;

		PillarBaseColor = FLinearColor(0.12f, 0.14f, 0.16f);
		PillarGridColor = FLinearColor(0.0f, 0.8f, 1.0f);
		PillarSpacing = 50.0f;
		LightGlowColor = FLinearColor(0.2f, 1.8f, 2.4f);
		break;

	case ELevelBiome::Level4_AbandonedOffice:
		// Plasterboard beige walls and corporate slate blue carpet tiles
		WallTexName = TEXT("T_Office_WallPlaster.png");
		FloorTexName = TEXT("T_Office_CarpetTile.png");
		CeilingTexName = TEXT("T_Lobby_Ceiling.png");
		PillarTexName = TEXT("T_Office_WallPlaster.png");
		WallTint = FLinearColor(0.92f, 0.90f, 0.86f);
		FloorTint = FLinearColor(0.45f, 0.55f, 0.70f);
		PillarTint = FLinearColor(0.88f, 0.86f, 0.82f);

		WallBaseColor = FLinearColor(0.82f, 0.80f, 0.76f);
		WallGridColor = FLinearColor(0.65f, 0.63f, 0.60f);
		WallSpacing = 200.0f;
		WallRoughness = 0.85f;

		FloorBaseColor = FLinearColor(0.24f, 0.28f, 0.35f);
		FloorGridColor = FLinearColor(0.14f, 0.16f, 0.22f);
		FloorSpacing = 80.0f;
		FloorRoughness = 0.90f;

		CeilingBaseColor = FLinearColor(0.90f, 0.90f, 0.88f);
		CeilingGridColor = FLinearColor(0.50f, 0.50f, 0.48f);
		CeilingSpacing = 120.0f;
		CeilingRoughness = 0.88f;

		PillarBaseColor = FLinearColor(0.78f, 0.76f, 0.72f);
		PillarGridColor = FLinearColor(0.60f, 0.58f, 0.55f);
		PillarSpacing = 100.0f;
		LightGlowColor = FLinearColor(1.8f, 1.9f, 2.0f);
		break;

	case ELevelBiome::Level6_LightsOut:
		WallBaseColor = FLinearColor(0.015f, 0.015f, 0.015f);
		WallGridColor = FLinearColor(0.008f, 0.008f, 0.008f);
		WallSpacing = 100.0f;
		WallRoughness = 1.0f;

		FloorBaseColor = FLinearColor(0.015f, 0.015f, 0.015f);
		FloorGridColor = FLinearColor(0.008f, 0.008f, 0.008f);
		FloorSpacing = 100.0f;
		FloorRoughness = 1.0f;

		CeilingBaseColor = FLinearColor(0.01f, 0.01f, 0.01f);
		CeilingGridColor = FLinearColor(0.005f, 0.005f, 0.005f);
		CeilingSpacing = 100.0f;
		CeilingRoughness = 1.0f;

		PillarBaseColor = FLinearColor(0.015f, 0.015f, 0.015f);
		PillarGridColor = FLinearColor(0.008f, 0.008f, 0.008f);
		PillarSpacing = 100.0f;
		LightGlowColor = FLinearColor(0.05f, 0.05f, 0.05f);
		break;

	case ELevelBiome::Level8_CaveSystem:
		// Dark subterranean stratified limestone & granite rock
		WallTexName = TEXT("T_Cave_Rock.png");
		FloorTexName = TEXT("T_Cave_Rock.png");
		CeilingTexName = TEXT("T_Cave_Rock.png");
		PillarTexName = TEXT("T_Cave_Rock.png");
		WallTint = FLinearColor(0.35f, 0.30f, 0.25f);
		FloorTint = FLinearColor(0.30f, 0.25f, 0.20f);
		CeilingTint = FLinearColor(0.20f, 0.15f, 0.12f);
		PillarTint = FLinearColor(0.32f, 0.27f, 0.22f);

		WallBaseColor = FLinearColor(0.20f, 0.17f, 0.14f);
		WallGridColor = FLinearColor(0.08f, 0.07f, 0.06f);
		WallSpacing = 120.0f;
		WallRoughness = 0.95f;

		FloorBaseColor = FLinearColor(0.15f, 0.13f, 0.10f);
		FloorGridColor = FLinearColor(0.06f, 0.05f, 0.04f);
		FloorSpacing = 150.0f;
		FloorRoughness = 0.92f;

		CeilingBaseColor = FLinearColor(0.12f, 0.10f, 0.08f);
		CeilingGridColor = FLinearColor(0.05f, 0.04f, 0.03f);
		CeilingSpacing = 100.0f;
		CeilingRoughness = 0.98f;

		PillarBaseColor = FLinearColor(0.18f, 0.15f, 0.12f);
		PillarGridColor = FLinearColor(0.07f, 0.06f, 0.05f);
		PillarSpacing = 80.0f;
		LightGlowColor = FLinearColor(1.0f, 0.9f, 0.6f);
		break;

	case ELevelBiome::Level9_DarkSuburbs:
		// Asphalt roads and dark overcast night
		WallTexName = TEXT("T_Concrete_Industrial.png");
		FloorTexName = TEXT("T_Suburbs_Asphalt.png");
		CeilingTexName = TEXT("T_Concrete_Industrial.png");
		PillarTexName = TEXT("T_Concrete_Industrial.png");
		WallTint = FLinearColor(0.45f, 0.40f, 0.35f);
		FloorTint = FLinearColor(0.85f, 0.85f, 0.88f);
		CeilingTint = FLinearColor(0.08f, 0.08f, 0.12f);
		PillarTint = FLinearColor(0.40f, 0.35f, 0.30f);

		WallBaseColor = FLinearColor(0.24f, 0.20f, 0.18f);
		WallGridColor = FLinearColor(0.10f, 0.08f, 0.07f);
		WallSpacing = 150.0f;
		WallRoughness = 0.75f;

		FloorBaseColor = FLinearColor(0.12f, 0.12f, 0.13f);
		FloorGridColor = FLinearColor(0.05f, 0.05f, 0.06f);
		FloorSpacing = 300.0f;
		FloorRoughness = 0.65f;

		CeilingBaseColor = FLinearColor(0.04f, 0.05f, 0.07f);
		CeilingGridColor = FLinearColor(0.02f, 0.02f, 0.03f);
		CeilingSpacing = 200.0f;
		CeilingRoughness = 0.90f;

		PillarBaseColor = FLinearColor(0.20f, 0.18f, 0.16f);
		PillarGridColor = FLinearColor(0.08f, 0.07f, 0.06f);
		PillarSpacing = 100.0f;
		LightGlowColor = FLinearColor(2.0f, 1.8f, 1.1f);
		break;

	case ELevelBiome::Level10_WheatFields:
		WallTexName = TEXT("T_Barn_Wood.png");
		FloorTexName = TEXT("T_Barn_Wood.png");
		CeilingTexName = TEXT("T_Barn_Wood.png");
		PillarTexName = TEXT("T_Barn_Wood.png");
		WallTint = FLinearColor(0.85f, 0.75f, 0.65f);
		FloorTint = FLinearColor(0.80f, 0.70f, 0.60f);
		PillarTint = FLinearColor(0.85f, 0.75f, 0.65f);

		WallBaseColor = FLinearColor(0.55f, 0.42f, 0.28f);
		WallGridColor = FLinearColor(0.25f, 0.18f, 0.10f);
		WallSpacing = 50.0f;
		WallRoughness = 0.88f;

		FloorBaseColor = FLinearColor(0.48f, 0.38f, 0.22f);
		FloorGridColor = FLinearColor(0.22f, 0.16f, 0.08f);
		FloorSpacing = 60.0f;
		FloorRoughness = 0.92f;

		CeilingBaseColor = FLinearColor(0.35f, 0.28f, 0.18f);
		CeilingGridColor = FLinearColor(0.16f, 0.12f, 0.06f);
		CeilingSpacing = 80.0f;
		CeilingRoughness = 0.90f;

		PillarBaseColor = FLinearColor(0.50f, 0.38f, 0.24f);
		PillarGridColor = FLinearColor(0.22f, 0.15f, 0.08f);
		PillarSpacing = 50.0f;
		LightGlowColor = FLinearColor(2.2f, 1.9f, 1.2f);
		break;

	case ELevelBiome::Level37_Poolrooms:
		// Pristine square ceramic tiles with aquatic grout and wet gloss reflections
		WallTexName = TEXT("T_Poolrooms_Tile.png");
		FloorTexName = TEXT("T_Poolrooms_Tile.png");
		CeilingTexName = TEXT("T_Poolrooms_Tile.png");
		PillarTexName = TEXT("T_Poolrooms_Tile.png");
		WallTint = FLinearColor(0.98f, 1.0f, 1.0f);
		FloorTint = FLinearColor(0.90f, 0.97f, 1.0f);
		CeilingTint = FLinearColor(0.94f, 0.98f, 1.0f);
		PillarTint = FLinearColor(0.98f, 1.0f, 1.0f);

		WallBaseColor = FLinearColor(0.96f, 0.98f, 1.0f);
		WallGridColor = FLinearColor(0.20f, 0.42f, 0.52f);
		WallSpacing = 50.0f;
		WallRoughness = 0.08f;

		FloorBaseColor = FLinearColor(0.85f, 0.94f, 0.98f);
		FloorGridColor = FLinearColor(0.15f, 0.38f, 0.48f);
		FloorSpacing = 50.0f;
		FloorRoughness = 0.04f;

		CeilingBaseColor = FLinearColor(0.94f, 0.96f, 0.98f);
		CeilingGridColor = FLinearColor(0.30f, 0.45f, 0.55f);
		CeilingSpacing = 100.0f;
		CeilingRoughness = 0.15f;

		PillarBaseColor = FLinearColor(0.96f, 0.98f, 1.0f);
		PillarGridColor = FLinearColor(0.20f, 0.42f, 0.52f);
		PillarSpacing = 50.0f;
		LightGlowColor = FLinearColor(1.5f, 2.2f, 2.4f);
		break;

	case ELevelBiome::LevelRun_RunForYourLife:
		WallTexName = TEXT("T_Run_HospitalWall.png");
		FloorTexName = TEXT("T_Run_HospitalFloor.png");
		CeilingTexName = TEXT("T_Lobby_Ceiling.png");
		PillarTexName = TEXT("T_Run_HospitalWall.png");
		WallTint = FLinearColor(1.2f, 1.2f, 1.2f);
		FloorTint = FLinearColor(1.1f, 1.1f, 1.15f);
		CeilingTint = FLinearColor(1.1f, 1.1f, 1.1f);
		PillarTint = FLinearColor(1.2f, 1.2f, 1.2f);

		WallBaseColor = FLinearColor(0.92f, 0.92f, 0.92f);
		WallGridColor = FLinearColor(0.40f, 0.40f, 0.40f);
		WallSpacing = 150.0f;
		WallRoughness = 0.40f;

		FloorBaseColor = FLinearColor(0.82f, 0.82f, 0.84f);
		FloorGridColor = FLinearColor(0.30f, 0.30f, 0.32f);
		FloorSpacing = 100.0f;
		FloorRoughness = 0.20f;

		CeilingBaseColor = FLinearColor(0.88f, 0.88f, 0.88f);
		CeilingGridColor = FLinearColor(0.35f, 0.35f, 0.35f);
		CeilingSpacing = 150.0f;
		CeilingRoughness = 0.50f;

		PillarBaseColor = FLinearColor(0.90f, 0.90f, 0.90f);
		PillarGridColor = FLinearColor(0.40f, 0.40f, 0.40f);
		PillarSpacing = 100.0f;
		LightGlowColor = FLinearColor(4.5f, 0.05f, 0.05f);
		break;

	default:
		break;
	}

	UMaterialInterface* TexBaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/Weapons/Rifle/Materials/M_Weapon.M_Weapon"));

	auto LoadTexture = [this](const FString& InTexName) -> UTexture2D*
	{
		if (InTexName.IsEmpty())
		{
			return nullptr;
		}
		if (TObjectPtr<UTexture2D>* Found = LoadedTextures.Find(InTexName))
		{
			return *Found;
		}

		FString CleanName = InTexName;
		CleanName.RemoveFromEnd(TEXT(".png"));

		// 1. Charger depuis le package d'assets Unreal importe
		const FString AssetPath = FString::Printf(TEXT("/Game/Textures/Backrooms/%s.%s"), *CleanName, *CleanName);
		if (UTexture2D* Loaded = LoadObject<UTexture2D>(nullptr, *AssetPath))
		{
			LoadedTextures.Add(InTexName, Loaded);
			return Loaded;
		}

		// 2. Repli vers l'importation disque brute
		const FString FullPath = FPaths::ProjectContentDir() / TEXT("Textures/Backrooms") / (CleanName + TEXT(".png"));
		if (FPaths::FileExists(FullPath))
		{
			if (UTexture2D* Loaded = FImageUtils::ImportFileAsTexture2D(FullPath))
			{
				LoadedTextures.Add(InTexName, Loaded);
				return Loaded;
			}
		}
		return nullptr;
	};

	auto CreateMat = [&](const FString& TexName, const FLinearColor& TintCol, const FLinearColor& BaseCol, const FLinearColor& GridCol, float GridSpacing, float Roughness) -> UMaterialInstanceDynamic*
	{
		FString CleanName = TexName;
		CleanName.RemoveFromEnd(TEXT(".png"));

		UTexture2D* TexAlbedo = LoadTexture(CleanName);
		UTexture2D* TexNormal = LoadTexture(CleanName + TEXT("_N"));
		UTexture2D* TexAORM = LoadTexture(CleanName + TEXT("_ORM"));

		if (TexAlbedo && TexBaseMat)
		{
			UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(TexBaseMat, this);
			if (Dyn)
			{
				Dyn->SetTextureParameterValue(FName("Albedo"), TexAlbedo);
				Dyn->SetTextureParameterValue(FName("BaseColor"), TexAlbedo);
				if (TexNormal)
				{
					Dyn->SetTextureParameterValue(FName("Normal"), TexNormal);
				}
				if (TexAORM)
				{
					Dyn->SetTextureParameterValue(FName("AORM"), TexAORM);
					Dyn->SetTextureParameterValue(FName("Masks"), TexAORM);
				}
				Dyn->SetVectorParameterValue(FName("Team.WeaponTint"), TintCol);
				Dyn->SetVectorParameterValue(FName("TeamColor_Tint"), TintCol);
				Dyn->SetVectorParameterValue(FName("Color"), TintCol);
				Dyn->SetScalarParameterValue(FName("Roughness_Min"), FMath::Clamp(Roughness - 0.2f, 0.05f, 1.0f));
				Dyn->SetScalarParameterValue(FName("Roughness_Max"), FMath::Clamp(Roughness + 0.1f, 0.1f, 1.0f));
				Dyn->SetScalarParameterValue(FName("Roughness"), Roughness);
				Dyn->SetScalarParameterValue(FName("AO_Strength"), 1.0f);
				return Dyn;
			}
		}

		UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(BaseMat, this);
		if (Dyn)
		{
			Dyn->SetVectorParameterValue(FName("Color"), BaseCol);
			Dyn->SetVectorParameterValue(FName("BaseColor"), BaseCol);
			Dyn->SetVectorParameterValue(FName("BackgroundColor"), BaseCol);
			Dyn->SetVectorParameterValue(FName("SurfaceColor"), BaseCol);
			Dyn->SetVectorParameterValue(FName("TopSurfaceColor"), BaseCol);
			Dyn->SetVectorParameterValue(FName("GridColor"), GridCol);
			Dyn->SetVectorParameterValue(FName("TopGridColor"), GridCol);
			Dyn->SetVectorParameterValue(FName("SubGridColor"), GridCol);
			Dyn->SetVectorParameterValue(FName("TopSubGridGridColor"), GridCol);
			Dyn->SetScalarParameterValue(FName("GridSpacing"), GridSpacing);
			Dyn->SetScalarParameterValue(FName("TileSize"), GridSpacing);
			Dyn->SetScalarParameterValue(FName("Scale"), GridSpacing / 100.0f);
			Dyn->SetScalarParameterValue(FName("Roughness"), Roughness);
		}
		return Dyn;
	};

	UMaterialInstanceDynamic* DynFloorMat = CreateMat(FloorTexName, FloorTint, FloorBaseColor, FloorGridColor, FloorSpacing, FloorRoughness);
	UMaterialInstanceDynamic* DynWallMat = CreateMat(WallTexName, WallTint, WallBaseColor, WallGridColor, WallSpacing, WallRoughness);
	UMaterialInstanceDynamic* DynCeilingMat = CreateMat(CeilingTexName, CeilingTint, CeilingBaseColor, CeilingGridColor, CeilingSpacing, CeilingRoughness);
	UMaterialInstanceDynamic* DynPillarMat = CreateMat(PillarTexName, PillarTint, PillarBaseColor, PillarGridColor, PillarSpacing, 0.8f);

	if (FloorInstances && DynFloorMat)
	{
		FloorInstances->SetMaterial(0, DynFloorMat);
	}
	if (WallInstances && DynWallMat)
	{
		WallInstances->SetMaterial(0, DynWallMat);
	}
	if (CeilingInstances && DynCeilingMat)
	{
		CeilingInstances->SetMaterial(0, DynCeilingMat);
	}
	if (PillarInstances && DynPillarMat)
	{
		PillarInstances->SetMaterial(0, DynPillarMat);
	}

	UMaterialInstanceDynamic* DynWaterMat = UMaterialInstanceDynamic::Create(BaseMat, this);
	if (DynWaterMat)
	{
		const FLinearColor WaterColor(0.08f, 0.42f, 0.58f, 0.35f);
		DynWaterMat->SetVectorParameterValue(FName("Color"), WaterColor);
		DynWaterMat->SetVectorParameterValue(FName("BaseColor"), WaterColor);
		DynWaterMat->SetVectorParameterValue(FName("SurfaceColor"), WaterColor);
		DynWaterMat->SetVectorParameterValue(FName("TopSurfaceColor"), WaterColor);
		DynWaterMat->SetScalarParameterValue(FName("Roughness"), 0.02f);
	}
	if (WaterInstances && DynWaterMat)
	{
		WaterInstances->SetMaterial(0, DynWaterMat);
	}

	UMaterialInterface* GlowBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Interactable/JumpPad/Assets/Materials/M_SimpleGlow.M_SimpleGlow"));
	if (!GlowBase)
	{
		GlowBase = BaseMat;
	}
	UMaterialInstanceDynamic* DynGlowMat = UMaterialInstanceDynamic::Create(GlowBase, this);
	if (DynGlowMat)
	{
		DynGlowMat->SetVectorParameterValue(FName("Color"), LightGlowColor);
		DynGlowMat->SetVectorParameterValue(FName("BaseColor"), LightGlowColor);
	}
	if (CeilingLightInstances && DynGlowMat)
	{
		CeilingLightInstances->SetMaterial(0, DynGlowMat);
	}
}

void ALiminalLevelGenerator::SpawnPillarsAndFixtures()
{
	PillarInstances->ClearInstances();
	CeilingLightInstances->ClearInstances();
	PipeInstances->ClearInstances();
	WaterInstances->ClearInstances();

	const bool bIsCustomPillar = PillarInstances->GetStaticMesh() && (PillarInstances->GetStaticMesh()->GetBounds().BoxExtent.Z > 100.0f);

	for (const FProcRoom& Room : CurrentLayout.Rooms)
	{
		// 1. Colonnes & Piliers interieurs (caracteristique majeure du Level 0 et des Poolrooms)
		if (Room.SizeX >= 3 && Room.SizeY >= 3)
		{
			const FVector PillarScale = bIsCustomPillar ?
				FVector(1.0f, 1.0f, WallHeight / 300.0f) :
				FVector(0.55f, 0.55f, WallHeight / 100.0f);
			const float PillarZ = bIsCustomPillar ? 0.0f : (WallHeight * 0.5f);

			for (int32 Py = Room.OriginY + 1; Py < Room.OriginY + Room.SizeY - 1; Py += 2)
			{
				for (int32 Px = Room.OriginX + 1; Px < Room.OriginX + Room.SizeX - 1; Px += 2)
				{
					FTransform PillarTrans(FRotator::ZeroRotator, CellToWorld(Px, Py, PillarZ), PillarScale);
					PillarInstances->AddInstance(PillarTrans);
				}
			}
		}

		// 2. Boitier 3D des Neons Fluorescents au plafond
		FTransform LightFixtureTrans(FRotator::ZeroRotator, CellToWorld(Room.CenterX, Room.CenterY, WallHeight), FVector(1.0f, 1.0f, 1.0f));
		CeilingLightInstances->AddInstance(LightFixtureTrans);

		// 3. Tuyaux industriels pour le Level 2 (Pipe Dreams)
		if (Biome == ELevelBiome::Level2_PipeDreams)
		{
			const FVector PipeScale(CellSize * Room.SizeX / 400.0f, 1.0f, 1.0f);
			FTransform PipeTrans(FRotator::ZeroRotator, CellToWorld(Room.CenterX, Room.CenterY, WallHeight - 35.0f), PipeScale);
			PipeInstances->AddInstance(PipeTrans);
		}

		// 4. Bassin d'eau peu profonde pour le Level 37 (Poolrooms)
		if (Biome == ELevelBiome::Level37_Poolrooms)
		{
			const FVector WaterScale(CellSize * Room.SizeX / 400.0f, CellSize * Room.SizeY / 400.0f, 0.1f);
			FTransform WaterTrans(FRotator::ZeroRotator, CellToWorld(Room.CenterX, Room.CenterY, 12.0f), WaterScale);
			WaterInstances->AddInstance(WaterTrans);
		}
	}

	// 5. Neons (CeilingLights) dans les couloirs (Level 0 et Level 1)
	if (Biome == ELevelBiome::Level0_YellowLobby || Biome == ELevelBiome::Level1_HabitableZone)
	{
		FRandomStream CorridorRnd(Seed * 23 + 17);
		for (int32 Y = 1; Y < CurrentLayout.Height - 1; ++Y)
		{
			for (int32 X = 1; X < CurrentLayout.Width - 1; ++X)
			{
				if (CurrentLayout.GetCell(X, Y) == EProcCellType::Corridor)
				{
					if (CorridorRnd.FRand() < 0.35f)
					{
						FTransform LightTrans(FRotator::ZeroRotator, CellToWorld(X, Y, WallHeight), FVector(1.0f, 1.0f, 1.0f));
						CeilingLightInstances->AddInstance(LightTrans);
					}
				}
			}
		}
	}
}

void ALiminalLevelGenerator::SpawnEnvironmentalProps()
{
	PropInstances->ClearInstances();

	FRandomStream Rnd(Seed * 103 + 47);

	for (const FProcRoom& Room : CurrentLayout.Rooms)
	{
		// Casiers / Lockers ou bureaux selon le biome
		if (Biome == ELevelBiome::Level0_YellowLobby || Biome == ELevelBiome::Level1_HabitableZone)
		{
			FTransform LockerTrans(FRotator(0.0f, Rnd.FRandRange(0.0f, 360.0f), 0.0f),
				CellToWorld(Room.OriginX, Room.OriginY, 0.0f) + FVector(60.0f, 60.0f, 0.0f), FVector(1.0f, 1.0f, 1.0f));
			PropInstances->AddInstance(LockerTrans);
		}
		else if (Biome == ELevelBiome::Level4_AbandonedOffice)
		{
			FTransform DeskTrans(FRotator(0.0f, Rnd.FRandRange(0.0f, 360.0f), 0.0f),
				CellToWorld(Room.OriginX + 1, Room.OriginY + 1, 0.0f), FVector(1.0f, 1.0f, 1.0f));
			PropInstances->AddInstance(DeskTrans);
		}
	}

	// Scatter dynamique des decors proceduraux (chaises, conduits de ventilation, tuyaux, etc.)
	if (UWorld* World = GetWorld())
	{
		if (ULiminalDecoratorSubsystem* Decorator = World->GetSubsystem<ULiminalDecoratorSubsystem>())
		{
			TArray<FVector> RoomCenters;
			TArray<FVector> RoomSizes;
			RoomCenters.Reserve(CurrentLayout.Rooms.Num());
			RoomSizes.Reserve(CurrentLayout.Rooms.Num());

			for (const FProcRoom& Room : CurrentLayout.Rooms)
			{
				RoomCenters.Add(CellToWorld(Room.CenterX, Room.CenterY, 0.0f));
				RoomSizes.Add(FVector(Room.SizeX * CellSize, Room.SizeY * CellSize, WallHeight));
			}

			Decorator->DecorateRooms(Biome, RoomCenters, RoomSizes, Seed);
		}

		// Spawn des cachettes interactives serveur (casiers / placards / dessous de bureaux)
		if (HasAuthority() && HidingSpotClass)
		{
			FActorSpawnParameters SpotParams;
			SpotParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			for (int32 RoomIdx = 1; RoomIdx < CurrentLayout.Rooms.Num(); ++RoomIdx)
			{
				const FProcRoom& Room = CurrentLayout.Rooms[RoomIdx];
				const FVector SpotLoc = CellToWorld(Room.OriginX, Room.OriginY, 0.0f) + FVector(80.0f, 80.0f, 0.0f);
				const FRotator SpotRot(0.0f, Rnd.FRandRange(0.0f, 4.0f) * 90.0f, 0.0f);

				if (ALiminalHidingSpot* HidingSpot = World->SpawnActor<ALiminalHidingSpot>(HidingSpotClass, SpotLoc, SpotRot, SpotParams))
				{
					SpawnedActors.Add(HidingSpot);
				}
			}
		}

		// Spawn des conduits de ventilation traversables (vents)
		if (HasAuthority() && VentActorClass && CurrentLayout.Rooms.Num() >= 2)
		{
			FActorSpawnParameters VentParams;
			VentParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

			for (int32 RoomIdx = 1; RoomIdx + 1 < CurrentLayout.Rooms.Num(); RoomIdx += 2)
			{
				const FProcRoom& RoomA = CurrentLayout.Rooms[RoomIdx];
				const FProcRoom& RoomB = CurrentLayout.Rooms[RoomIdx + 1];

				const FVector LocA = CellToWorld(RoomA.CenterX, RoomA.CenterY, 30.0f);
				const FVector LocB = CellToWorld(RoomB.CenterX, RoomB.CenterY, 30.0f);
				const FVector MidPoint = (LocA + LocB) * 0.5f;
				const FRotator VentRot = (LocB - LocA).Rotation();

				if (ALiminalVentActor* VentActor = World->SpawnActor<ALiminalVentActor>(VentActorClass, MidPoint, VentRot, VentParams))
				{
					SpawnedActors.Add(VentActor);
				}
			}
		}
	}
}

void ALiminalLevelGenerator::SpawnNonEuclideanPortals()
{
	if (!HasAuthority() || !PortalActorClass || CurrentLayout.Rooms.Num() < 4 || ExtraLoopChance <= 0.0f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FRandomStream Rnd(Seed * 317 + 89);
	if (Rnd.FRand() > ExtraLoopChance)
	{
		return;
	}

	FActorSpawnParameters PortalParams;
	PortalParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	const int32 RoomIdxA = 1;
	const int32 RoomIdxB = FMath::Min(CurrentLayout.Rooms.Num() - 2, 3);
	if (RoomIdxA == RoomIdxB)
	{
		return;
	}

	const FProcRoom& RoomA = CurrentLayout.Rooms[RoomIdxA];
	const FProcRoom& RoomB = CurrentLayout.Rooms[RoomIdxB];

	const FVector LocA = CellToWorld(RoomA.CenterX, RoomA.CenterY, 150.0f);
	const FVector LocB = CellToWorld(RoomB.CenterX, RoomB.CenterY, 150.0f);
	const FRotator RotA(0.0f, 0.0f, 0.0f);
	const FRotator RotB(0.0f, 180.0f, 0.0f);

	ALiminalPortalActor* PortalA = World->SpawnActor<ALiminalPortalActor>(PortalActorClass, LocA, RotA, PortalParams);
	ALiminalPortalActor* PortalB = World->SpawnActor<ALiminalPortalActor>(PortalActorClass, LocB, RotB, PortalParams);

	if (PortalA && PortalB)
	{
		PortalA->PortalComponent->LinkTargetPortal(PortalB->PortalComponent);
		SpawnedActors.Add(PortalA);
		SpawnedActors.Add(PortalB);
		UE_LOG(LogTemp, Log, TEXT("LiminalLevelGenerator : Anomaly Portals deployed between Room %d and Room %d"), RoomIdxA, RoomIdxB);
	}
}

void ALiminalLevelGenerator::SetupNavMeshBounds()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const float TotalWidth = GridWidth * CellSize;
	const float TotalHeight = GridHeight * CellSize;
	const FVector TargetLocation(0.0f, 0.0f, WallHeight * 0.5f);
	const FVector DesiredScale((TotalWidth + 1000.0f) / 200.0f, (TotalHeight + 1000.0f) / 200.0f, (WallHeight + 800.0f) / 200.0f);

	ANavMeshBoundsVolume* NavBounds = nullptr;
	for (TActorIterator<ANavMeshBoundsVolume> It(World); It; ++It)
	{
		NavBounds = *It;
		break;
	}

	if (!NavBounds)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		NavBounds = World->SpawnActor<ANavMeshBoundsVolume>(ANavMeshBoundsVolume::StaticClass(), TargetLocation, FRotator::ZeroRotator, SpawnParams);
	}

	if (NavBounds)
	{
		NavBounds->SetActorLocation(TargetLocation);
		NavBounds->SetActorScale3D(DesiredScale);
	}

	if (UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
	{
		if (NavBounds)
		{
			NavSys->OnNavigationBoundsUpdated(NavBounds);
		}
		NavSys->Build();
		UE_LOG(LogTemp, Log, TEXT("LiminalLevelGenerator : Dynamic NavMeshBoundsVolume configured (Scale=%s) and NavSys rebuilt"), *DesiredScale.ToString());
	}
}

static void MEGRunGenProc(const TArray<FString>& Args)
{
	UWorld* World = nullptr;
	if (GEngine)
	{
		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			if (Context.World() && Context.World()->IsGameWorld())
			{
				World = Context.World();
				break;
			}
		}
	}
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("MEG.GenProc : aucun monde de jeu actif"));
		return;
	}

	bool bFound = false;
	for (TActorIterator<ALiminalLevelGenerator> Iterator(World); Iterator; ++Iterator)
	{
		const int32 NewSeed = Args.Num() > 0 ? FCString::Atoi(*Args[0]) : Iterator->GetSeed();
		Iterator->Generate(NewSeed);
		bFound = true;
		break;
	}

	if (!bFound)
	{
		UE_LOG(LogTemp, Warning, TEXT("MEG.GenProc : aucun ALiminalLevelGenerator dans le niveau"));
	}
}

static FAutoConsoleCommand GMEGGenProcCommand(
	TEXT("MEG.GenProc"),
	TEXT("Regenere le niveau procedura avec la seed donnee : MEG.GenProc <seed>"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&MEGRunGenProc));
