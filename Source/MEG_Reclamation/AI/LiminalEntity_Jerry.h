#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Jerry.generated.h"

class AScavengerCharacter;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnJerryHypnosisChanged, bool, bIsActive, AScavengerCharacter*, HypnotizedTarget);

/**
 * Entite Jerry (Etape 11 de la Roadmap).
 * Oiseau hypnotique psionique bloquant les mouvements de la cible dans son regard.
 * Neutralisable uniquement si un coéquipier le frappe au Micro-Ondes Sonique ou lui inflige des dégâts.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Jerry : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Jerry();

	virtual void Tick(float DeltaSeconds) override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Entity|Jerry")
	void BreakHypnosis();

	UFUNCTION(BlueprintPure, Category = "Entity|Jerry")
	bool IsHypnotizing() const { return bIsHypnotizing; }

	UFUNCTION(BlueprintPure, Category = "Entity|Jerry")
	AScavengerCharacter* GetHypnotizedTarget() const { return HypnotizedTarget.Get(); }

	UFUNCTION(BlueprintPure, Category = "Entity|Jerry")
	float GetHypnosisGazeRange() const { return HypnosisGazeRange; }

	UFUNCTION(BlueprintPure, Category = "Entity|Jerry")
	float GetHypnosisSanityDrainPerSecond() const { return HypnosisSanityDrainPerSecond; }

	UPROPERTY(BlueprintAssignable, Category = "Entity|Jerry")
	FOnJerryHypnosisChanged OnJerryHypnosisChanged;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Jerry|Visuals")
	TObjectPtr<UPointLightComponent> PsionicAura;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Jerry", meta = (ClampMin = "200.0"))
	float HypnosisGazeRange = 1100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Jerry", meta = (ClampMin = "1.0"))
	float HypnosisSanityDrainPerSecond = 8.0f;

private:
	void UpdateHypnoticGaze(float DeltaSeconds);
	bool HasLineOfSightTo(const AActor* Target) const;

	bool bIsHypnotizing = false;
	TWeakObjectPtr<AScavengerCharacter> HypnotizedTarget;
};
