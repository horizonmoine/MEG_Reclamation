#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Duller.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDullerRevealStateChanged, bool, bIsRevealed);

/**
 * Entite Duller (Etape 11 de la Roadmap).
 * Punit l'absence de scanner : invisible a l'œil nu dans les couloirs.
 * Se revele temporairement lorsqu'il est balaye par le Scanner LIDAR ou le Strobe.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Duller : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Duller();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Category = "Entity|Duller")
	void RevealFromScanner(float DurationSeconds);

	UFUNCTION(BlueprintPure, Category = "Entity|Duller")
	bool IsCloaked() const { return !bIsRevealed; }

	UFUNCTION(BlueprintPure, Category = "Entity|Duller")
	bool IsRevealed() const { return bIsRevealed; }

	UPROPERTY(BlueprintAssignable, Category = "Entity|Duller")
	FOnDullerRevealStateChanged OnDullerRevealStateChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_IsRevealed, VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Duller")
	bool bIsRevealed = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Duller", meta = (ClampMin = "1.0"))
	float DefaultRevealDuration = 6.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Duller", meta = (ClampMin = "1.0"))
	float AmbushDamageMultiplier = 1.75f;

	UFUNCTION()
	void OnRep_IsRevealed();

private:
	void UpdateCloaking(float DeltaSeconds);
	void UpdateVisualCloakAppearance();

	float RevealTimer = 0.0f;
};
