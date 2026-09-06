#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "RealityAnchorTool.generated.h"

class AScavengerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAnchorStateChanged, bool, bIsDeployed);

/**
 * Ancre de Réalité (Outil Phase 2 - Etape 11 de la Roadmap).
 * Dispositif lourd (30 kg) déployable au sol.
 * Émet un champ de stabilisation dimensionnelle qui bloque le drain de sanité
 * et empêche les hallucinations d'apparaître dans son rayon d'action.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ARealityAnchorTool : public ABaseTool
{
	GENERATED_BODY()

public:
	ARealityAnchorTool();

	virtual bool Activate() override;
	virtual void Deactivate() override;
	virtual bool CanActivate() const override;

	UFUNCTION(BlueprintCallable, Category = "Anchor")
	void DeployAnchor();

	UFUNCTION(BlueprintCallable, Category = "Anchor")
	void PackUpAnchor();

	UFUNCTION(BlueprintPure, Category = "Anchor")
	bool IsDeployed() const { return bIsDeployed; }

	UFUNCTION(BlueprintPure, Category = "Anchor")
	float GetStabilizationRadius() const { return StabilizationRadius; }

	UFUNCTION(BlueprintPure, Category = "Anchor")
	float GetRemainingFieldDuration() const { return RemainingFieldDuration; }

	UFUNCTION(BlueprintPure, Category = "Anchor")
	float GetAnchorWeightKg() const { return WeightKg; }

	UPROPERTY(BlueprintAssignable, Category = "Anchor")
	FOnAnchorStateChanged OnAnchorStateChanged;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor", meta = (ClampMin = "10.0"))
	float WeightKg = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor", meta = (ClampMin = "300.0"))
	float StabilizationRadius = 850.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Anchor", meta = (ClampMin = "10.0"))
	float MaxFieldDurationSeconds = 60.0f;

private:
	void UpdateStabilizationField(float DeltaSeconds);

	bool bIsDeployed = false;
	float RemainingFieldDuration = 60.0f;
};
