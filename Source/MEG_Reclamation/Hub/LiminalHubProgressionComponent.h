#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalHubProgressionComponent.generated.h"

UENUM(BlueprintType)
enum class EHubTier : uint8
{
	MakeshiftCamp UMETA(DisplayName = "Palier 1 : Campement de fortune"),
	ReinforcedOutpost UMETA(DisplayName = "Palier 2 : Avant-poste fortifie"),
	ScientificLab UMETA(DisplayName = "Palier 3 : Laboratoire de confinement M.E.G.")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHubTierChanged, EHubTier, NewTier);

/**
 * Composant de progression visuelle et structurelle du Hub (Etape 11 de la Roadmap).
 *
 * Fait evoluer l'apparence du Hub (Base Alpha) selon les credits accumules
 * et le nombre de cycles de quotas surmontes par l'escouade.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalHubProgressionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalHubProgressionComponent();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Hub|Progression")
	void RefreshHubTier(int32 TotalBankCredits, int32 CompletedCycles);

	UFUNCTION(BlueprintPure, Category = "Hub|Progression")
	EHubTier GetCurrentTier() const { return CurrentTier; }

	UFUNCTION(BlueprintPure, Category = "Hub|Progression")
	FText GetTierDisplayName() const;

	UPROPERTY(BlueprintAssignable, Category = "Hub|Progression")
	FOnHubTierChanged OnHubTierChanged;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hub|Progression")
	EHubTier CurrentTier = EHubTier::MakeshiftCamp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub|Progression", meta = (ClampMin = "100"))
	int32 Tier2CreditsThreshold = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hub|Progression", meta = (ClampMin = "500"))
	int32 Tier3CreditsThreshold = 1500;
};
