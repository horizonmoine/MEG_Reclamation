#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LiminalEntityDirector.generated.h"

class AScavengerCharacter;
class ALiminalEntity;

/**
 * Phase de tension de la mission.
 * Le Director module le rythme du jeu en faisant alterner
 * des periodes de calme et des pics de terreur.
 */
UENUM(BlueprintType)
enum class EDirectorPhase : uint8
{
	/** Calme relatif — les joueurs explorent, peu de menaces. */
	BuildUp        UMETA(DisplayName = "Build-Up (Calme)"),
	/** La tension monte — sons inquietants, quelques signes de danger. */
	Sustain        UMETA(DisplayName = "Sustain (Tension)"),
	/** Pic de terreur — evenement majeur, rush de monstres, blackout. */
	Peak           UMETA(DisplayName = "Peak (Terreur)"),
	/** Respiration — courte accalmie apres un pic pour eviter la fatigue du joueur. */
	Respite        UMETA(DisplayName = "Respite (Repos)")
};

/**
 * Metriques de stress de l'escouade en temps reel.
 */
USTRUCT(BlueprintType)
struct FSquadStressMetrics
{
	GENERATED_BODY()

	/** Sante moyenne de l'escouade (0-1). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	float AverageHealthPercent = 1.0f;

	/** Sanite moyenne de l'escouade (0-1). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	float AverageSanityPercent = 1.0f;

	/** Nombre de joueurs encore en vie. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	int32 AlivePlayerCount = 0;

	/** Nombre de joueurs a terre (downed). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	int32 DownedPlayerCount = 0;

	/** Nombre d'entites actuellement en chasse active. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	int32 ActiveChasingEntities = 0;

	/** Temps ecoule depuis le dernier evenement majeur (secondes). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	float TimeSinceLastPeak = 0.0f;

	/** Intensite de stress composite (0 = zen, 1 = terreur maximale). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Director")
	float CompositeStress = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDirectorPhaseChanged, EDirectorPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDirectorEventTriggered, FName, EventName);

/**
 * AI Director (inspire Left 4 Dead) — controle le rythme global de tension.
 *
 * Surveille en permanence l'etat de l'escouade (sante, sanite, entites en chasse,
 * temps depuis le dernier evenement) et module dynamiquement :
 * - L'agressivite et le spawn rate des entites
 * - Le declenchement d'evenements environnementaux (blackouts, reality shifts, infestations)
 * - Les periodes de calme pour eviter la fatigue du joueur
 *
 * Le Director ne spawn pas directement les entites — il communique avec le
 * LiminalEventSubsystem et le LiminalGameMode pour orchestrer la tension.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalEntityDirector : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULiminalEntityDirector();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** Appele chaque frame par le GameMode (server-only). */
	void UpdateDirector(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "Director")
	EDirectorPhase GetCurrentPhase() const { return CurrentPhase; }

	UFUNCTION(BlueprintPure, Category = "Director")
	const FSquadStressMetrics& GetStressMetrics() const { return StressMetrics; }

	UFUNCTION(BlueprintPure, Category = "Director")
	float GetGlobalIntensityMultiplier() const { return GlobalIntensityMultiplier; }

	/** Force un changement de phase (debug / scripted). */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void ForcePhase(EDirectorPhase NewPhase);

	/** Indique au Director qu'un evenement vient de se terminer. */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void NotifyEventCompleted(FName EventName);

	/** Indique qu'un joueur est mort — augmente la pression. */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void NotifyPlayerDied();

	/** Indique qu'un joueur a ete reanime — reduit legerement la pression. */
	UFUNCTION(BlueprintCallable, Category = "Director")
	void NotifyPlayerRevived();

	UPROPERTY(BlueprintAssignable, Category = "Director")
	FOnDirectorPhaseChanged OnPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Director")
	FOnDirectorEventTriggered OnEventTriggered;

protected:
	void GatherStressMetrics();
	void EvaluatePhaseTransition(float DeltaSeconds);
	void SelectAndTriggerEvent();

	/** Parametres de timing du Director. */

	/** Duree minimale de la phase Build-Up avant de pouvoir passer en Sustain (secondes). */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Timing")
	float MinBuildUpDuration = 45.0f;

	/** Duree maximale de la phase Build-Up (force la transition). */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Timing")
	float MaxBuildUpDuration = 120.0f;

	/** Duree de la phase Sustain avant le Peak. */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Timing")
	float SustainDuration = 30.0f;

	/** Duree minimale du Peak (l'evenement principal). */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Timing")
	float MinPeakDuration = 20.0f;

	/** Duree de la phase Respite (accalmie). */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Timing")
	float RespiteDuration = 25.0f;

	/** Seuil de stress composite au-dessus duquel le Director evite de declencher un Peak. */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float StressCapForPeak = 0.85f;

	/** Seuil de stress en-dessous duquel le Director considere que les joueurs sont trop en securite. */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Thresholds", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BoredomThreshold = 0.15f;

	/** Multiplicateur d'intensite applique aux entites (1 = normal, 2 = double agressivite). */
	UPROPERTY(EditDefaultsOnly, Category = "Director|Scaling", meta = (ClampMin = "0.5", ClampMax = "3.0"))
	float BaseIntensityMultiplier = 1.0f;

private:
	EDirectorPhase CurrentPhase = EDirectorPhase::BuildUp;
	FSquadStressMetrics StressMetrics;

	float PhaseTimer = 0.0f;
	float GlobalIntensityMultiplier = 1.0f;
	int32 PeakCount = 0;
	int32 DeathCount = 0;

	float TimeSinceLastUpdate = 0.0f;
	static constexpr float MetricsUpdateInterval = 0.5f;
};
