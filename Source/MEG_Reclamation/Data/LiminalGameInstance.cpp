#include "Data/LiminalGameInstance.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameModes/LiminalGameState.h"
#include "Player/LiminalPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/Paths.h"

ULiminalGameInstance::ULiminalGameInstance()
{
	SaveData.UnlockedBiomes.Add(ELevelBiome::Level0_YellowLobby);
	SaveData.UnlockedBiomes.Add(ELevelBiome::Level1_HabitableZone);
	SaveData.UnlockedBiomes.Add(ELevelBiome::Level37_Poolrooms);
}

void ULiminalGameInstance::Init()
{
	Super::Init();

	LoadGameFromDisk();
}

void ULiminalGameInstance::AddCredits(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	SaveData.TotalBankCredits += Amount;
	SaveGameToDisk();
	OnCreditsChanged.Broadcast(SaveData.TotalBankCredits);
}

bool ULiminalGameInstance::SpendCredits(int32 Amount)
{
	if (Amount <= 0 || SaveData.TotalBankCredits < Amount)
	{
		return false;
	}

	SaveData.TotalBankCredits -= Amount;
	SaveGameToDisk();
	OnCreditsChanged.Broadcast(SaveData.TotalBankCredits);
	return true;
}

void ULiminalGameInstance::AddStoredTool(FName ToolId)
{
	if (!ToolId.IsNone())
	{
		SaveData.StoredToolIds.AddUnique(ToolId);
		SaveGameToDisk();
	}
}

void ULiminalGameInstance::UnlockBiome(ELevelBiome BiomeToUnlock)
{
	SaveData.UnlockedBiomes.AddUnique(BiomeToUnlock);
	SaveGameToDisk();
}

void ULiminalGameInstance::TravelToMission()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FString BaseMap = TEXT("/Game/Maps/Lvl_ProcGen");

	// Routage automatique vers la carte dediee du biome selectionne
	switch (SelectedBiome)
	{
	case ELevelBiome::Level0_YellowLobby:
		BaseMap = TEXT("/Game/Maps/Lvl_00_Lobby");
		break;
	case ELevelBiome::Level1_HabitableZone:
		BaseMap = TEXT("/Game/Maps/Lvl_01_HabitableZone");
		break;
	case ELevelBiome::Level2_PipeDreams:
		BaseMap = TEXT("/Game/Maps/Lvl_02_PipeDreams");
		break;
	case ELevelBiome::Level3_ElectricalStation:
		BaseMap = TEXT("/Game/Maps/Lvl_03_ElectricalStation");
		break;
	case ELevelBiome::Level4_AbandonedOffice:
		BaseMap = TEXT("/Game/Maps/Lvl_04_AbandonedOffice");
		break;
	case ELevelBiome::Level6_LightsOut:
		BaseMap = TEXT("/Game/Maps/Lvl_06_LightsOut");
		break;
	case ELevelBiome::Level8_CaveSystem:
		BaseMap = TEXT("/Game/Maps/Lvl_08_CaveSystem");
		break;
	case ELevelBiome::Level9_DarkSuburbs:
		BaseMap = TEXT("/Game/Maps/Lvl_09_DarkSuburbs");
		break;
	case ELevelBiome::Level10_WheatFields:
		BaseMap = TEXT("/Game/Maps/Lvl_10_WheatFields");
		break;
	case ELevelBiome::Level37_Poolrooms:
		BaseMap = TEXT("/Game/Maps/Lvl_37_Poolrooms");
		break;
	case ELevelBiome::LevelRun_RunForYourLife:
		BaseMap = TEXT("/Game/Maps/Lvl_99_RunForYourLife");
		break;
	default:
		BaseMap = TEXT("/Game/Maps/Lvl_00_Lobby");
		break;
	}

	const FString TargetMapName = FString::Printf(TEXT("%s?listen&game=/Script/MEG_Reclamation.LiminalGameMode&Biome=%d&Scale=%d"),
		*BaseMap, static_cast<int32>(SelectedBiome), SelectedMapScale);

	SaveGameToDisk();
	if (World->GetNetMode() == NM_ListenServer || World->GetNetMode() == NM_DedicatedServer)
	{
		World->ServerTravel(TargetMapName);
	}
	else
	{
		UGameplayStatics::OpenLevel(World, FName(*TargetMapName));
	}
}

void ULiminalGameInstance::ReturnToHub()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (ALiminalGameState* GS = World->GetGameState<ALiminalGameState>())
	{
		for (APlayerState* BasePS : GS->PlayerArray)
		{
			if (ALiminalPlayerState* PS = Cast<ALiminalPlayerState>(BasePS))
			{
				PS->AuthResetForNewRun();
			}
		}
		GS->AuthSetMissionPhase(EMissionPhase::Hub);
	}
	else
	{
		for (TActorIterator<ALiminalPlayerState> It(World); It; ++It)
		{
			if (ALiminalPlayerState* PS = *It)
			{
				PS->AuthResetForNewRun();
			}
		}
	}

	const FString HubMap = TEXT("/Game/Maps/Lvl_Hub_BaseAlpha");

	const FString TargetMapName = FString::Printf(TEXT("%s?listen&game=/Script/MEG_Reclamation.LiminalLobbyGameMode"), *HubMap);
	SaveGameToDisk();
	if (World->GetNetMode() == NM_ListenServer || World->GetNetMode() == NM_DedicatedServer)
	{
		World->ServerTravel(TargetMapName);
	}
	else
	{
		UGameplayStatics::OpenLevel(World, FName(*TargetMapName));
	}
}

bool ULiminalGameInstance::SaveGameToDisk()
{
	ULiminalSaveGame* SaveObject = Cast<ULiminalSaveGame>(
		UGameplayStatics::CreateSaveGameObject(ULiminalSaveGame::StaticClass()));
	if (!SaveObject)
	{
		return false;
	}

	SaveObject->Data = SaveData;
	return UGameplayStatics::SaveGameToSlot(SaveObject, SaveSlotName, 0);
}

bool ULiminalGameInstance::LoadGameFromDisk()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		SaveGameToDisk();
		return false;
	}

	ULiminalSaveGame* LoadedObject = Cast<ULiminalSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!LoadedObject)
	{
		return false;
	}

	SaveData = LoadedObject->Data;
	OnCreditsChanged.Broadcast(SaveData.TotalBankCredits);
	return true;
}

void ULiminalGameInstance::ResetCampaign()
{
	SaveData = FLiminalSaveData();
	SaveData.UnlockedBiomes.Empty();
	SaveData.UnlockedBiomes.Add(ELevelBiome::Level0_YellowLobby);
	SaveData.UnlockedBiomes.Add(ELevelBiome::Level1_HabitableZone);
	SaveData.UnlockedBiomes.Add(ELevelBiome::Level37_Poolrooms);
	SaveGameToDisk();
	OnCreditsChanged.Broadcast(SaveData.TotalBankCredits);
}
