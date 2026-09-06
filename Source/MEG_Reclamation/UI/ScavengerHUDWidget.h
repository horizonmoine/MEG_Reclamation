#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Sanity/LiminalSanityTypes.h"
#include "ScavengerHUDWidget.generated.h"

class AScavengerCharacter;

/**
 * Widget HUD d'exploration pour l'escouade M.E.G.
 * Fournit les getters de donnees analogiques pour l'interface UMG :
 * - Sante et Sanite du joueur
 * - Palier de sanite et alertes UI diegetiques corrompues (Sanite 2.0)
 * - Endurance et Poids de la cargaison portee
 * - Credits recuperes et progression du Quota
 * - Statut de l'outil actif et charge de batterie
 */
UCLASS(Abstract, Blueprintable)
class MEG_RECLAMATION_API UScavengerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	float GetSanityPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	ESanityTier GetSanityTier() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	FText GetCorruptedAlertText() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	bool HasActiveHallucinationAlert() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	float GetWeightRatio() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	int32 GetCarriedCredits() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	FName GetCurrentToolName() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	float GetToolBatteryCharge() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	bool IsCriticallyLowSanity() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|HUD")
	bool IsCriticallyLowHealth() const;

protected:
	AScavengerCharacter* GetOwningScavenger() const;
};
