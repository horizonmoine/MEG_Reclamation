#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "TetherTool.generated.h"

class AScavengerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTetherConnectionChanged, bool, bIsConnected, AScavengerCharacter*, Partner);

/**
 * Câble de liaison / Tether (Outil Phase 2 - Etape 11 de la Roadmap).
 * Relie physiquement deux explorateurs de l'escouade pour empêcher la dispersion
 * et la désorientation dans les biomes sans visibilité (Lights Out, Dark Suburbs).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ATetherTool : public ABaseTool
{
	GENERATED_BODY()

public:
	ATetherTool();

	virtual bool Activate() override;
	virtual void Deactivate() override;
	virtual bool CanActivate() const override;

	UFUNCTION(BlueprintCallable, Category = "Tether")
	bool ConnectToNearestPartner();

	UFUNCTION(BlueprintCallable, Category = "Tether")
	void DisconnectTether();

	UFUNCTION(BlueprintPure, Category = "Tether")
	bool IsTethered() const { return ConnectedPartner.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Tether")
	AScavengerCharacter* GetConnectedPartner() const { return ConnectedPartner.Get(); }

	UFUNCTION(BlueprintPure, Category = "Tether")
	float GetMaxTetherLength() const { return MaxTetherLength; }

	UFUNCTION(BlueprintPure, Category = "Tether")
	float GetCurrentTetherDistance() const;

	UPROPERTY(BlueprintAssignable, Category = "Tether")
	FOnTetherConnectionChanged OnTetherConnectionChanged;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tether", meta = (ClampMin = "300.0"))
	float MaxTetherLength = 1500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tether", meta = (ClampMin = "100.0"))
	float ConnectRange = 600.0f;

private:
	TWeakObjectPtr<AScavengerCharacter> ConnectedPartner;
};
