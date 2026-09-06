#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "QuotaManager.generated.h"

/**
 * Etat pur du quota : aucune dependance UObject, testable hors moteur.
 */
struct FQuotaState
{
	int32 Delivered = 0;
	int32 Required = 0;
	int32 OutstandingDebt = 0;
};

/**
 * Logique pure du quota/dette - meme comportement que le subsystem,
 * verifiable par tests d'automatisation sans instanciation d'UObject.
 */
class FQuotaLogic
{
public:
	static int32 ClampTarget(int32 Value)
	{
		return FMath::Max(Value, 0);
	}

	static void AddDelivered(FQuotaState& State, int32 Amount)
	{
		if (Amount > 0)
		{
			State.Delivered += Amount;
		}
	}

	static int32 TotalDue(const FQuotaState& State)
	{
		return State.Required + State.OutstandingDebt;
	}

	static bool IsMet(const FQuotaState& State)
	{
		return State.Delivered >= TotalDue(State);
	}

	static void ApplyFailure(FQuotaState& State)
	{
		State.OutstandingDebt += FMath::Max(TotalDue(State) - State.Delivered, 0);
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnQuotaUpdated, int32, DeliveredValue, int32, TotalDue);

/**
 * Gere la progression du quota et la dette globale du Hub.
 *
 * Server-authoritative : toutes les mutations doivent etre appelees cote serveur (listen server).
 * Ce subsystem n'est pas replique ; la lecture cote client passera par l'etat replique
 * de la hierarchie PlayerController/GameState (etapes suivantes de la roadmap).
 * La logique est deleguee a FQuotaLogic (pure) pour rester testable sans monde.
 */
UCLASS(BlueprintType)
class MEG_RECLAMATION_API UQuotaManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Quota")
	void SetQuotaTarget(int32 NewRequiredValue);

	UFUNCTION(BlueprintCallable, Category = "Quota")
	void AddDeliveredValue(int32 ValueToAdd);

	UFUNCTION(BlueprintCallable, Category = "Quota")
	void ApplyQuotaFailure();

	UFUNCTION(BlueprintPure, Category = "Quota")
	bool IsQuotaMet() const;

	UFUNCTION(BlueprintPure, Category = "Quota")
	int32 GetDeliveredValue() const;

	UFUNCTION(BlueprintPure, Category = "Quota")
	int32 GetRequiredValue() const;

	UFUNCTION(BlueprintPure, Category = "Quota")
	int32 GetOutstandingDebt() const;

	UFUNCTION(BlueprintPure, Category = "Quota")
	int32 GetTotalDue() const;

	UPROPERTY(BlueprintAssignable, Category = "Quota")
	FOnQuotaUpdated OnQuotaUpdated;

private:
	void BroadcastQuotaUpdate();

	FQuotaState State;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(EditDefaultsOnly, Category = "Quota", meta = (ClampMin = "0"))
	int32 DefaultQuotaTarget = 100;
};
