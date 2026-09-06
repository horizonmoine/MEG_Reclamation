#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LiminalEventSubsystem.generated.h"

class ULiminalEntityDirector;
class ALiminalGameMode;

/**
 * Descripteur d'un evenement dynamique de mission.
 */
USTRUCT(BlueprintType)
struct FLiminalEventDescriptor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FName EventId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event")
	FText DisplayName;

	/** Duree de l'evenement en secondes (0 = instantane). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (ClampMin = "0.0"))
	float DurationSeconds = 30.0f;

	/** Cooldown avant que cet evenement puisse etre redeclenche. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 90.0f;

	/** Multiplicateur de loudness pour l'IA durant cet evenement. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (ClampMin = "0.0"))
	float EntityAggressionMultiplier = 1.5f;

	/** Nombre minimum de Peaks avant que l'evenement soit disponible. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event", meta = (ClampMin = "0"))
	int32 MinPeakCountRequired = 0;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLiminalEventStarted, FName, EventId, float, Duration);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLiminalEventEnded, FName, EventId);

/**
 * Sous-systeme gerant les evenements dynamiques de mission.
 * Orchestre les blackouts, reality shifts, infestations, alarmes et effondrements.
 * Ecoute le LiminalEntityDirector pour savoir QUAND declencher,
 * et fournit la logique de COMMENT chaque evenement fonctionne.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalEventSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	ULiminalEventSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	/** Appele chaque frame par le GameMode. */
	void UpdateEvents(float DeltaSeconds);

	/** Declenche un evenement par son ID. */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void TriggerEvent(FName EventId);

	/** Force l'arret de l'evenement actif. */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void EndCurrentEvent();

	UFUNCTION(BlueprintPure, Category = "Events")
	bool IsEventActive() const { return bEventActive; }

	UFUNCTION(BlueprintPure, Category = "Events")
	FName GetActiveEventId() const { return ActiveEventId; }

	UFUNCTION(BlueprintPure, Category = "Events")
	float GetActiveEventRemainingSeconds() const { return ActiveEventTimer; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLiminalEventStarted OnEventStarted;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLiminalEventEnded OnEventEnded;

protected:
	void RegisterDefaultEvents();

	void ExecuteBlackout();
	void ExecuteRealityShift();
	void ExecuteInfestation();
	void ExecuteAlarm();
	void ExecuteCollapse();

	void EndBlackout();
	void EndRealityShift();
	void EndInfestation();
	void EndAlarm();
	void EndCollapse();

private:
	TMap<FName, FLiminalEventDescriptor> EventRegistry;
	TMap<FName, double> EventCooldowns;

	bool bEventActive = false;
	FName ActiveEventId;
	float ActiveEventTimer = 0.0f;
};
