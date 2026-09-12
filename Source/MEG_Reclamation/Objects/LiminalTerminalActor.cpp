#include "Objects/LiminalTerminalActor.h"

#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/LiminalGameInstance.h"
#include "Crafting/LiminalCraftingComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameModes/LiminalGameState.h"
#include "GameModes/LiminalLobbyGameMode.h"
#include "Player/LiminalPlayerState.h"
#include "Objects/LiminalAirlockActor.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"
#include "Tools/FlashStrobeTool.h"
#include "Tools/SonicMicrowaveTool.h"
#include "Tools/AudioDecoyTool.h"
#include "Tools/LidarScannerTool.h"
#include "Tools/SignalAnalyzerTool.h"
#include "Tools/RealityAnchorTool.h"
#include "Tools/TetherTool.h"
#include "Tools/ChalkMarkerTool.h"
#include "Tools/AdrenalineInjectorTool.h"
#include "UI/LiminalScavengerHUD.h"

ALiminalTerminalActor::ALiminalTerminalActor()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBox"));
	SetRootComponent(InteractionBox);
	InteractionBox->SetBoxExtent(FVector(120.0f, 120.0f, 100.0f));
	InteractionBox->SetCollisionProfileName(TEXT("Trigger"));

	TerminalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TerminalMesh"));
	TerminalMesh->SetupAttachment(InteractionBox);
	TerminalMesh->SetCollisionProfileName(TEXT("BlockAll"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TerminalFinder(
		TEXT("/Game/Meshes/Props/SM_Terminal_MEG.SM_Terminal_MEG"));
	if (TerminalFinder.Succeeded())
	{
		TerminalMesh->SetStaticMesh(TerminalFinder.Object);
		TerminalMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
			TEXT("/Game/LevelPrototyping/Meshes/SM_Cube.SM_Cube"));
		if (CubeFinder.Succeeded())
		{
			TerminalMesh->SetStaticMesh(CubeFinder.Object);
			TerminalMesh->SetRelativeScale3D(FVector(0.8f, 1.2f, 1.0f));
		}
	}

	ScreenLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ScreenLight"));
	ScreenLight->SetupAttachment(TerminalMesh);
	ScreenLight->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
	ScreenLight->SetLightColor(FLinearColor(0.1f, 0.8f, 1.0f));
	ScreenLight->SetIntensity(3500.0f);

	CraftingComponent = CreateDefaultSubobject<ULiminalCraftingComponent>(TEXT("CraftingComponent"));

	FTerminalStoreItem Batterie;
	Batterie.ItemId = FName("Item_Battery");
	Batterie.DisplayName = FText::FromString("Pile 9V Haute Capacite");
	Batterie.CostCredits = 40;
	StoreCatalog.Add(Batterie);

	FTerminalStoreItem AlmondWater;
	AlmondWater.ItemId = FName("Item_AlmondWaterBottle");
	AlmondWater.DisplayName = FText::FromString("Bouteille d'Eau d'Amande (500ml)");
	AlmondWater.CostCredits = 65;
	StoreCatalog.Add(AlmondWater);

	FTerminalStoreItem Strobe;
	Strobe.ItemId = FName("Tool_FlashStrobe");
	Strobe.DisplayName = FText::FromString("Flash Stroboscopique M.E.G.");
	Strobe.CostCredits = 140;
	StoreCatalog.Add(Strobe);

	FTerminalStoreItem Sonic;
	Sonic.ItemId = FName("Tool_SonicMicrowave");
	Sonic.DisplayName = FText::FromString("Emetteur Micro-ondes Sonique");
	Sonic.CostCredits = 190;
	StoreCatalog.Add(Sonic);

	FTerminalStoreItem Decoy;
	Decoy.ItemId = FName("Tool_AudioDecoy");
	Decoy.DisplayName = FText::FromString("Leurre Sonore Portable");
	Decoy.CostCredits = 120;
	StoreCatalog.Add(Decoy);

	FTerminalStoreItem Lidar;
	Lidar.ItemId = FName("Tool_LidarScanner");
	Lidar.DisplayName = FText::FromString("Scanner Conique LIDAR (Detecte Dullers)");
	Lidar.CostCredits = 220;
	StoreCatalog.Add(Lidar);

	FTerminalStoreItem Signal;
	Signal.ItemId = FName("Tool_SignalAnalyzer");
	Signal.DisplayName = FText::FromString("Analyseur de Signal Spectrographique");
	Signal.CostCredits = 260;
	StoreCatalog.Add(Signal);

	FTerminalStoreItem Anchor;
	Anchor.ItemId = FName("Tool_RealityAnchor");
	Anchor.DisplayName = FText::FromString("Ancre de Realite 30kg (Bulle Anti-Folie)");
	Anchor.CostCredits = 380;
	StoreCatalog.Add(Anchor);

	FTerminalStoreItem Tether;
	Tether.ItemId = FName("Tool_Tether");
	Tether.DisplayName = FText::FromString("Cable de Liaison d'Escouade");
	Tether.CostCredits = 110;
	StoreCatalog.Add(Tether);

	FTerminalStoreItem Chalk;
	Chalk.ItemId = FName("Tool_ChalkMarker");
	Chalk.DisplayName = FText::FromString("Craie Fluo (16 Marques de Navigation)");
	Chalk.CostCredits = 45;
	StoreCatalog.Add(Chalk);

	FTerminalStoreItem Adrenaline;
	Adrenaline.ItemId = FName("Tool_AdrenalineInjector");
	Adrenaline.DisplayName = FText::FromString("Autoinjecteur d'Adrenaline M.E.G. (Defibrillateur/Rush)");
	Adrenaline.CostCredits = 210;
	StoreCatalog.Add(Adrenaline);
}

void ALiminalTerminalActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALiminalTerminalActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALiminalTerminalActor, SelectedBiome);
}

void ALiminalTerminalActor::Interact(AScavengerCharacter* InteractingPlayer)
{
	if (!InteractingPlayer)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(InteractingPlayer->GetController()))
	{
		if (ALiminalScavengerHUD* HUD = Cast<ALiminalScavengerHUD>(PC->GetHUD()))
		{
			if (HUD->IsTerminalOpen())
			{
				HUD->CloseTerminalUI();
			}
			else
			{
				HUD->OpenTerminalUI(this);
			}
			return;
		}
	}
}

void ALiminalTerminalActor::PurchaseStoreItem(FName ItemId, AScavengerCharacter* Buyer)
{
	if (!HasAuthority() || !Buyer)
	{
		return;
	}

	const FTerminalStoreItem* FoundItem = StoreCatalog.FindByPredicate(
		[&](const FTerminalStoreItem& It) { return It.ItemId == ItemId; });

	if (!FoundItem)
	{
		return;
	}

	ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance());
	if (!GI)
	{
		return;
	}

	if (GI->SpendCredits(FoundItem->CostCredits))
	{
		if (ItemId == FName("Item_Battery"))
		{
			Buyer->RechargeCurrentToolBattery(100.0f);
		}
		else if (ItemId == FName("Item_AlmondWaterBottle"))
		{
			Buyer->HealAndRestoreSanity(35.0f, 40.0f);
		}
		else
		{
			TSubclassOf<ABaseTool> ToolClass = nullptr;
			if (ItemId == FName("Tool_FlashStrobe")) ToolClass = AFlashStrobeTool::StaticClass();
			else if (ItemId == FName("Tool_SonicMicrowave")) ToolClass = ASonicMicrowaveTool::StaticClass();
			else if (ItemId == FName("Tool_AudioDecoy")) ToolClass = AAudioDecoyTool::StaticClass();
			else if (ItemId == FName("Tool_LidarScanner")) ToolClass = ALidarScannerTool::StaticClass();
			else if (ItemId == FName("Tool_SignalAnalyzer")) ToolClass = ASignalAnalyzerTool::StaticClass();
			else if (ItemId == FName("Tool_RealityAnchor")) ToolClass = ARealityAnchorTool::StaticClass();
			else if (ItemId == FName("Tool_Tether")) ToolClass = ATetherTool::StaticClass();
			else if (ItemId == FName("Tool_ChalkMarker")) ToolClass = AChalkMarkerTool::StaticClass();
			else if (ItemId == FName("Tool_AdrenalineInjector")) ToolClass = AAdrenalineInjectorTool::StaticClass();

			if (ToolClass)
			{
				Buyer->AddOwnedTool(ToolClass);
				GI->AddStoredTool(ItemId);
			}
		}

#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
				FString::Printf(TEXT("Achat confirme : %s (-%d cr). Solde restant : %d cr"),
					*FoundItem->DisplayName.ToString(), FoundItem->CostCredits, GI->GetTotalCredits()));
		}
#endif
	}
}

void ALiminalTerminalActor::SelectBiome(ELevelBiome Biome)
{
	if (!HasAuthority())
	{
		return;
	}

	SelectedBiome = Biome;
	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
	{
		GI->SetSelectedBiome(Biome);
	}

	FLinearColor BiomeColor = FLinearColor::White;
	switch (SelectedBiome)
	{
	case ELevelBiome::Level0_YellowLobby: BiomeColor = FLinearColor(1.0f, 0.85f, 0.2f); break;
	case ELevelBiome::Level1_HabitableZone: BiomeColor = FLinearColor(0.6f, 0.65f, 0.7f); break;
	case ELevelBiome::Level2_PipeDreams: BiomeColor = FLinearColor(0.85f, 0.45f, 0.1f); break;
	case ELevelBiome::Level3_ElectricalStation: BiomeColor = FLinearColor(0.1f, 0.9f, 1.0f); break;
	case ELevelBiome::Level4_AbandonedOffice: BiomeColor = FLinearColor(0.8f, 0.85f, 0.9f); break;
	case ELevelBiome::Level6_LightsOut: BiomeColor = FLinearColor(0.05f, 0.05f, 0.1f); break;
	case ELevelBiome::Level8_CaveSystem: BiomeColor = FLinearColor(0.4f, 0.35f, 0.3f); break;
	case ELevelBiome::Level9_DarkSuburbs: BiomeColor = FLinearColor(0.15f, 0.2f, 0.35f); break;
	case ELevelBiome::Level10_WheatFields: BiomeColor = FLinearColor(0.95f, 0.75f, 0.2f); break;
	case ELevelBiome::Level37_Poolrooms: BiomeColor = FLinearColor(0.0f, 0.85f, 0.95f); break;
	case ELevelBiome::LevelRun_RunForYourLife: BiomeColor = FLinearColor(1.0f, 0.05f, 0.05f); break;
	default: break;
	}

	if (ScreenLight)
	{
		ScreenLight->SetLightColor(BiomeColor);
	}
}

void ALiminalTerminalActor::LaunchIncursion()
{
	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	ALiminalAirlockActor* Airlock = nullptr;
	for (TActorIterator<ALiminalAirlockActor> It(World); It; ++It)
	{
		Airlock = *It;
		break;
	}

	if (!Airlock)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Terminal] Incursion refusee : aucun sas d'embarquement ALiminalAirlockActor detecte."));
		return;
	}

	TArray<ALiminalPlayerState*> SquadPlayerStates;
	if (ALiminalGameState* GS = World->GetGameState<ALiminalGameState>())
	{
		for (APlayerState* BasePS : GS->PlayerArray)
		{
			if (ALiminalPlayerState* PS = Cast<ALiminalPlayerState>(BasePS))
			{
				SquadPlayerStates.Add(PS);
			}
		}
	}
	if (SquadPlayerStates.Num() == 0)
	{
		for (TActorIterator<ALiminalPlayerState> It(World); It; ++It)
		{
			if (ALiminalPlayerState* PS = *It)
			{
				SquadPlayerStates.Add(PS);
			}
		}
	}

	if (SquadPlayerStates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Terminal] Incursion refusee : aucun PlayerState joueur dans la session."));
		return;
	}

	for (ALiminalPlayerState* PS : SquadPlayerStates)
	{
		if (!PS->IsReady())
		{
			UE_LOG(LogTemp, Warning, TEXT("[Terminal] Incursion refusee : joueur %s non pret (!bIsReady)."), *PS->GetPlayerName());
			return;
		}

		if (PS->GetStatus() != EScavengerStatus::Alive)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Terminal] Incursion refusee : joueur %s non vivant (Statut: %d)."),
				*PS->GetPlayerName(), static_cast<int32>(PS->GetStatus()));
			return;
		}

		APawn* Pawn = PS->GetPawn();
		if (!Pawn || !Airlock->IsActorInsideAirlock(Pawn))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Terminal] Incursion refusee : le joueur %s n'est pas a l'interieur du sas."), *PS->GetPlayerName());
			return;
		}
	}

	if (ALiminalLobbyGameMode* LobbyGM = Cast<ALiminalLobbyGameMode>(World->GetAuthGameMode()))
	{
		LobbyGM->LaunchSquadMission(SelectedBiome);
	}
}
