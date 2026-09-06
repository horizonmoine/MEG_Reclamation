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

	/**
	 * Formule canonique de calcul du quota logistique du M.E.G. (Cycles de 3 rotations).
	 * Q(k, N) = floor(Q_base * (1 + alpha)^(k-1) + beta * (k-1)^1.4) + delta * (N - 1)
	 */
	static int32 CalculateCycleQuota(int32 CycleIndex, int32 PlayerCount = 1)
	{
		const int32 k = FMath::Max(1, CycleIndex);
		const int32 N = FMath::Clamp(PlayerCount, 1, 4);
		const float Q0 = 180.0f;
		const float Alpha = 0.32f;
		const float Beta = 55.0f;
		const float Delta = 45.0f;

		const float ExpTerm = Q0 * FMath::Pow(1.0f + Alpha, static_cast<float>(k - 1));
		const float PolyTerm = Beta * FMath::Pow(static_cast<float>(k - 1), 1.4f);
		const float BaseQuota = FMath::FloorToFloat(ExpTerm + PolyTerm);
		const float TotalQuota = BaseQuota + Delta * static_cast<float>(N - 1);

		return FMath::Max(180, static_cast<int32>(TotalQuota));
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

	UFUNCTION(BlueprintCallable, Category = "Quota")
	void AdvanceCycle(int32 PlayerCount = 1);

	UFUNCTION(BlueprintPure, Category = "Quota")
	int32 GetCycleIndex() const { return CurrentCycleIndex; }

	UPROPERTY(BlueprintAssignable, Category = "Quota")
	FOnQuotaUpdated OnQuotaUpdated;

private:
	void BroadcastQuotaUpdate();

	FQuotaState State;
	int32 CurrentCycleIndex = 1;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(EditDefaultsOnly, Category = "Quota", meta = (ClampMin = "0"))
	int32 DefaultQuotaTarget = 100;
};
