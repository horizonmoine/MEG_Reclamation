#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LiminalDeathScreenHUD.generated.h"

/**
 * HUD diegetique d'ecran de mort M.E.G.
 * Affiche un rapport de telemetrie d'echec sur moniteur CRT ambre/rouge :
 * - Statut K.I.A. / Signal perdu
 * - Cause du deces (Entite tueuse / Traumatisme)
 * - Valeur du butin abandonne dans le secteur
 * - Duree de survie avant elimination
 * - Invite d'activation de la telemetrie spectateur CCTV
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalDeathScreenHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALiminalDeathScreenHUD();

	virtual void DrawHUD() override;

	UFUNCTION(BlueprintCallable, Category = "Liminal|Death")
	void SetDeathDetails(const FString& InKillerName, int32 InLostCredits, float InSurvivalTimeSeconds);

	UFUNCTION(BlueprintCallable, Category = "Liminal|Death")
	void TransitionToSpectator();

protected:
	virtual void BeginPlay() override;

	void DrawCRTBackground(float Width, float Height);
	void DrawDeathReport(float Width, float Height);
	void DrawScanlines(float Width, float Height);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liminal|Death")
	FString KillerName = TEXT("ENTITÉ INCONNUE");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liminal|Death")
	int32 LostCredits = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liminal|Death")
	float SurvivalTimeSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Liminal|Death")
	float AutoTransitionDelay = 6.0f;

private:
	float AnimationTimer = 0.0f;
	float ScanlineOffset = 0.0f;
	bool bTransitionTriggered = false;
};
