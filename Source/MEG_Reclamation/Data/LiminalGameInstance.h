#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameFramework/SaveGame.h"
#include "LiminalGameInstance.generated.h"

UENUM(BlueprintType)
enum class ELevelBiome : uint8
{
	Level0_YellowLobby UMETA(DisplayName = "Niveau 0 - Le Lobby"),
	Level1_HabitableZone UMETA(DisplayName = "Niveau 1 - Zone Habitable"),
	Level2_PipeDreams UMETA(DisplayName = "Niveau 2 - Tuyauteries & Vapeur (Pipe Dreams)"),
	Level3_ElectricalStation UMETA(DisplayName = "Niveau 3 - Station Electrique"),
	Level4_AbandonedOffice UMETA(DisplayName = "Niveau 4 - Bureaux Abandonnes"),
	Level6_LightsOut UMETA(DisplayName = "Niveau 6 - Noir Absolu (Lights Out)"),
	Level8_CaveSystem UMETA(DisplayName = "Niveau 8 - Reseau de Cavernes"),
	Level9_DarkSuburbs UMETA(DisplayName = "Niveau 9 - Le Faubourg Obscur"),
	Level10_WheatFields UMETA(DisplayName = "Niveau 10 - Les Champs de Ble"),
	Level37_Poolrooms UMETA(DisplayName = "Niveau 37 - Les Poolrooms"),
	LevelRun_RunForYourLife UMETA(DisplayName = "Niveau ! - Fuyez pour survivre (Run For Your Life)")
};

USTRUCT(BlueprintType)
struct FLiminalSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 TotalBankCredits = 250;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 ActiveQuotaCycle = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 CurrentDebt = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FName> StoredToolIds;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<ELevelBiome> UnlockedBiomes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Hub")
	int32 HubLevel = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData|Hub")
	TArray<FName> UnlockedUpgrades;
};

UCLASS()
class MEG_RECLAMATION_API ULiminalSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Save")
	FLiminalSaveData Data;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCreditsChanged, int32, NewTotalCredits);

UCLASS()
class MEG_RECLAMATION_API ULiminalGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	ULiminalGameInstance();

	virtual void Init() override;

	UFUNCTION(BlueprintCallable, Category = "Liminal|Economy")
	void AddCredits(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Liminal|Economy")
	bool SpendCredits(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Liminal|Economy")
	int32 GetTotalCredits() const { return SaveData.TotalBankCredits; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Economy")
	int32 GetActiveQuotaCycle() const { return SaveData.ActiveQuotaCycle; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Save")
	const FLiminalSaveData& GetSaveData() const { return SaveData; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|Save")
	void AddStoredTool(FName ToolId);

	UFUNCTION(BlueprintCallable, Category = "Liminal|Save")
	void UnlockBiome(ELevelBiome BiomeToUnlock);

	UFUNCTION(BlueprintCallable, Category = "Liminal|Mission")
	void SetSelectedBiome(ELevelBiome NewBiome) { SelectedBiome = NewBiome; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Mission")
	ELevelBiome GetSelectedBiome() const { return SelectedBiome; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|Mission")
	void SetSelectedMapScale(int32 NewScale) { SelectedMapScale = FMath::Clamp(NewScale, 0, 3); }

	UFUNCTION(BlueprintPure, Category = "Liminal|Mission")
	int32 GetSelectedMapScale() const { return SelectedMapScale; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|Mission")
	void TravelToMission();

	UFUNCTION(BlueprintCallable, Category = "Liminal|Mission")
	void ReturnToHub();

	UFUNCTION(BlueprintCallable, Category = "Liminal|Save")
	bool SaveGameToDisk();

	UFUNCTION(BlueprintCallable, Category = "Liminal|Save")
	bool LoadGameFromDisk();

	UFUNCTION(BlueprintCallable, Category = "Liminal|Save")
	void ResetCampaign();

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Events")
	FOnCreditsChanged OnCreditsChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liminal|Data")
	FLiminalSaveData SaveData;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liminal|Mission")
	ELevelBiome SelectedBiome = ELevelBiome::Level0_YellowLobby;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liminal|Mission")
	int32 SelectedMapScale = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Liminal|Save")
	FString SaveSlotName = TEXT("MEG_Reclamation_SaveSlot_0");
};
