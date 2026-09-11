#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "LiminalGameState.generated.h"

UENUM(BlueprintType)
enum class EStabilityPhase : uint8
{
	Normal UMETA(DisplayName = "100-60% Stable"),
	Destabilized UMETA(DisplayName = "60-20% Déstabilisé (Lumières & Portes)"),
	Collapse UMETA(DisplayName = "20-0% Effondrement (Hordes & Dégâts)")
};

UENUM(BlueprintType)
enum class EMissionPhase : uint8
{
	Hub,
	Airlock,
	Incursion,
	Collapsing,
	Extracted,
	Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMissionPhaseChanged, EMissionPhase, OldPhase, EMissionPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeamBankCreditsChanged, int32, NewCredits);

/**
 * Etat de mission replique a tous les clients.
 * Le timer d'effondrement n'est jamais un compteur decremente : on replique un timestamp
 * serveur + une duree, chaque client derive le temps restant via GetServerWorldTimeSeconds().
 * Les transitions de phase sont decidees par ALiminalGameMode (serveur) via AuthSetMissionPhase.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALiminalGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// --- Blackout (existant) ---

	UPROPERTY(ReplicatedUsing = OnRep_BlackoutActive, BlueprintReadOnly, Category = "Mission|Events")
	bool bIsBlackoutActive;

	UFUNCTION()
	void OnRep_BlackoutActive();

	// --- Stabilite (existant, conserve pour compatibilite) ---

	/** Stabilite globale du niveau liminal (100% -> 0%). Derivee du timer quand celui-ci tourne. */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStability, BlueprintReadOnly, Category = "Mission|Stability")
	float CurrentStability;

	UFUNCTION()
	void OnRep_CurrentStability();

	UFUNCTION(BlueprintPure, Category = "Mission|Stability")
	EStabilityPhase GetStabilityPhase() const;

	UFUNCTION(BlueprintCallable, Category = "Mission|Stability")
	void SetStability(float NewStability);

	UFUNCTION(BlueprintCallable, Category = "Mission|Stability")
	void DrainStability(float Amount);

	// --- Phase de mission ---

	UFUNCTION(BlueprintPure, Category = "Mission|Phase")
	EMissionPhase GetMissionPhase() const { return MissionPhase; }

	/** Serveur uniquement. Retourne false et logue si la transition est invalide. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission|Phase")
	bool AuthSetMissionPhase(EMissionPhase NewPhase);

	static bool IsPhaseTransitionValid(EMissionPhase From, EMissionPhase To);

	UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
	FOnMissionPhaseChanged OnMissionPhaseChanged;

	// --- Timer d'effondrement (Reality Collapse) ---

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission|Collapse")
	void AuthStartCollapseTimer(float DurationSeconds);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission|Collapse")
	void AuthStopCollapseTimer();

	UFUNCTION(BlueprintPure, Category = "Mission|Collapse")
	bool IsCollapseTimerRunning() const { return bCollapseTimerRunning; }

	/** Temps restant en secondes, calcule localement a partir du temps serveur. */
	UFUNCTION(BlueprintPure, Category = "Mission|Collapse")
	float GetCollapseTimeRemaining() const;

	/** Ratio ecoule 0..1. */
	UFUNCTION(BlueprintPure, Category = "Mission|Collapse")
	float GetCollapseProgress() const;

	UFUNCTION(BlueprintPure, Category = "Mission|Collapse")
	float GetCollapseDurationSeconds() const { return CollapseDurationSeconds; }

	// --- Seed procedurale partagee ---

	UFUNCTION(BlueprintPure, Category = "Mission|ProcGen")
	int32 GetMissionSeed() const { return MissionSeed; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission|ProcGen")
	void AuthSetMissionSeed(int32 NewSeed);

	// --- Banque equipe ---

	UFUNCTION(BlueprintPure, Category = "Mission|Economy")
	int32 GetTeamBankCredits() const { return TeamBankCredits; }

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Mission|Economy")
	void AuthAddTeamBankCredits(int32 Delta);

	UPROPERTY(BlueprintAssignable, Category = "Mission|Events")
	FOnTeamBankCreditsChanged OnTeamBankCreditsChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MissionPhase, VisibleInstanceOnly, Category = "Mission|Phase")
	EMissionPhase MissionPhase;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Mission|Collapse")
	float CollapseStartServerTime;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Mission|Collapse")
	float CollapseDurationSeconds;

	UPROPERTY(Replicated, VisibleInstanceOnly, Category = "Mission|Collapse")
	bool bCollapseTimerRunning;

	UPROPERTY(ReplicatedUsing = OnRep_MissionSeed, VisibleInstanceOnly, Category = "Mission|ProcGen")
	int32 MissionSeed;

	UPROPERTY(ReplicatedUsing = OnRep_TeamBankCredits, VisibleInstanceOnly, Category = "Mission|Economy")
	int32 TeamBankCredits;

	UFUNCTION()
	void OnRep_MissionPhase(EMissionPhase OldPhase);

	UFUNCTION()
	void OnRep_MissionSeed();

	UFUNCTION()
	void OnRep_TeamBankCredits();
};

