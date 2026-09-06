#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LiminalDebriefHUD.generated.h"

/**
 * HUD de debriefing post-extraction.
 * Affiche un rapport officiel M.E.G. style formulaire analogique :
 * - Credits collectes vs quota cible
 * - Etat de la dette
 * - Statistiques par joueur (degats subis, distance, outils utilises)
 * - Verdict (QUOTA ATTEINT / DEFICIT)
 * - Bouton retour au Hub
 *
 * Ce HUD est defini comme HUDClass par un GameMode de transition post-extraction.
 */
USTRUCT(BlueprintType)
struct FPlayerMissionStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	FString PlayerName = TEXT("Recuperateur");

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 CreditsCollected = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	float DamageReceived = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	float DistanceTraveled = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 ToolsUsed = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	bool bSurvived = true;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	FString CauseOfDeath = TEXT("");
};

USTRUCT(BlueprintType)
struct FMissionDebriefData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 TotalCreditsExtracted = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 QuotaTarget = 100;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 PreviousDebt = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 NewDebt = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 BankBalanceAfter = 0;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	int32 QuotaCycleNumber = 1;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	FString BiomeName = TEXT("Niveau 0 — Le Lobby");

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	float MissionDurationSeconds = 0.0f;

	UPROPERTY(BlueprintReadWrite, Category = "Debrief")
	TArray<FPlayerMissionStats> PlayerStats;

	bool IsQuotaMet() const { return TotalCreditsExtracted >= QuotaTarget; }
};

UCLASS()
class MEG_RECLAMATION_API ALiminalDebriefHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALiminalDebriefHUD();

	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	/** Charge les donnees de debriefing (appele par le GameMode post-extraction) */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Debrief")
	void SetDebriefData(const FMissionDebriefData& InData);

protected:
	void DrawDebriefReport(float W, float H);
	void DrawPlayerStatsTable(float X, float Y, float Width, float Scale);
	void DrawStampVerdict(float X, float Y, float Scale);
	void HandleDebriefInput();

private:
	FMissionDebriefData DebriefData;

	// Animation
	float RevealTimer = 0.0f;
	float TypewriterProgress = 0.0f;
	float StampTimer = 0.0f;
	bool bStampRevealed = false;
	float PulseTimer = 0.0f;
	float ScanlineOffset = 0.0f;

	// Navigation
	bool bReadyToReturn = false;
	float InputCooldown = 0.0f;

	// Couleurs
	static const FLinearColor CRT_Amber;
	static const FLinearColor CRT_AmberDim;
	static const FLinearColor CRT_AmberBright;
	static const FLinearColor CRT_Green;
	static const FLinearColor CRT_Red;
	static const FLinearColor CRT_Background;
};
