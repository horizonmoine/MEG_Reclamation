#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Sanity/LiminalSanityTypes.h"
#include "LiminalScavengerHUD.generated.h"

class AScavengerCharacter;
class ALiminalSpectatorPawn;

/**
 * HUD natif pour les recuperateurs du M.E.G.
 * Dessine directement sur le Canvas sans dependance Blueprint :
 * - Reticule central analogique avec detection d'interaction
 * - Jauge d'endurance dynamique (vert / ambre / rouge)
 * - Moniteur de sanite 2.0 (palier mental, glitches et fausses alertes)
 * - Statut de charge et nom de l'outil equipe
 * - Poids de la cargaison et alerte de surpoids
 * - Objectifs de quota et statut d'extraction
 * - Moniteur CRT de surveillance en mode spectateur (co-op)
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalScavengerHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALiminalScavengerHUD();

	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "Liminal|HUD")
	void ToggleFieldManual() { bShowFieldManual = !bShowFieldManual; }

	UFUNCTION(BlueprintPure, Category = "Liminal|HUD")
	bool IsFieldManualOpen() const { return bShowFieldManual; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|HUD")
	void ToggleVHSOverlay() { bShowVHSOverlay = !bShowVHSOverlay; }

	UFUNCTION(BlueprintPure, Category = "Liminal|HUD")
	bool IsVHSOverlayActive() const { return bShowVHSOverlay; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|HUD")
	void ShowLootPickupNotification(int32 Credits, float WeightKg);

protected:
	AScavengerCharacter* GetOwningScavenger() const;
	ALiminalSpectatorPawn* GetOwningSpectator() const;

	void DrawReticle(AScavengerCharacter* Scavenger, float CenterX, float CenterY);
	void DrawSurvivalGauges(AScavengerCharacter* Scavenger, float CenterX, float CenterY);
	void DrawToolStatus(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight);
	void DrawMissionStatus(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight);
	void DrawSanityGlitches(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight);
	void DrawSpectatorHUD(ALiminalSpectatorPawn* Spectator, float ScreenWidth, float ScreenHeight);
	void DrawFieldManual(float ScreenWidth, float ScreenHeight);
	void DrawVHSBodycamOSD(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight);
	void DrawDamageVignette(float Intensity, float ScreenWidth, float ScreenHeight);
	void DrawDownedIndicator(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight);

	void DrawProgressBar(float X, float Y, float Width, float Height, float Percent,
		const FLinearColor& FillColor, const FLinearColor& BackColor, const FString& Label);

private:
	float GlitchTimer = 0.0f;
	float ScanlineOffset = 0.0f;
	bool bShowFieldManual = false;
	bool bShowVHSOverlay = true;
	float LootNotificationTimer = 0.0f;
	FString LootNotificationText;
};
