#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalPauseMenuComponent.generated.h"

/**
 * Menu Pause in-game (superpose au HUD de mission).
 * Accessible via Echap. Dessine sur Canvas, style CRT ambre coherent.
 * Resume / Options / Quitter la Partie.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalPauseMenuComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalPauseMenuComponent();

	/** Dessine le menu pause par-dessus le HUD actif */
	void DrawPauseMenu(UCanvas* Canvas, float ScreenWidth, float ScreenHeight, float DeltaTime);

	/** Toggle ouverture/fermeture */
	void TogglePause();

	UFUNCTION(BlueprintPure, Category = "Liminal|Pause")
	bool IsPaused() const { return bIsPaused; }

	/** Gestion input (appele chaque frame par le HUD proprietaire) */
	void HandlePauseInput(APlayerController* PC);

protected:
	void OnResume();
	void OnOptions();
	void OnQuitToMenu();

private:
	bool bIsPaused = false;
	int32 SelectedButton = 0;
	float InputCooldown = 0.0f;
	float PulseTimer = 0.0f;

	// Settings inline (reduit)
	bool bShowInlineSettings = false;
	int32 InlineSettingsSelection = 0;

	static const int32 BUTTON_COUNT = 3;
};
