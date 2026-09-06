#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LiminalGameMode.generated.h"

class AScavengerCharacter;
class ALiminalSpectatorPawn;

UENUM(BlueprintType)
enum class EExtractionMatchState : uint8
{
	InMission,
	ExtractionPending,
	MissionSuccess,
	SquadWiped
};

/**
 * GameMode des niveaux hostiles (boucle extraction).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALiminalGameMode();

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	void OnPlayerDied(AScavengerCharacter* DeadCharacter);

	UFUNCTION(BlueprintCallable, Category = "Extraction")
	void TriggerExtraction(AScavengerCharacter* Extractor);

	UFUNCTION(BlueprintPure, Category = "Extraction")
	EExtractionMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintPure, Category = "Extraction")
	int32 GetAlivePlayerCount() const;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Mission")
	float GetRealityCollapseRemainingSeconds() const { return RealityCollapseTimer; }

	UFUNCTION(BlueprintPure, Category = "Mission")
	float GetTotalMissionDurationSeconds() const { return TotalMissionDuration; }

	UFUNCTION(BlueprintCallable, Category = "Mission|Events")
	void TriggerBlackout(float DurationSeconds = 45.0f);

	UFUNCTION(BlueprintCallable, Category = "Mission|Events")
	void RestorePower();

	UFUNCTION(BlueprintPure, Category = "Mission|Events")
	bool IsBlackoutActive() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Events")
	float GetBlackoutRemainingSeconds() const { return BlackoutTimer; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Classes")
	TSubclassOf<ALiminalSpectatorPawn> SpectatorPawnClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extraction")
	EExtractionMatchState MatchState = EExtractionMatchState::InMission;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mission", meta = (ClampMin = "60.0"))
	float TotalMissionDuration = 480.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission")
	float RealityCollapseTimer = 480.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mission|Events")
	float BlackoutTimer = 0.0f;

	float NextRandomBlackoutTime = 120.0f;

	float StateTransitionTimer = 0.0f;
	bool bTransitionPending = false;

private:
	void CheckGameOverCondition();
	void HandleSquadWiped();
	void HandleMissionSuccess();

	TSet<TWeakObjectPtr<AActor>> UsedPlayerStarts;
};
