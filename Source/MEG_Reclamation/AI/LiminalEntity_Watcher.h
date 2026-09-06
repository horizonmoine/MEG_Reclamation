#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Watcher.generated.h"

class AScavengerCharacter;
class UPointLightComponent;

/**
 * Watcher (Entite psychologique des Backrooms) :
 * Observe silencieusement les techniciens M.E.G. depuis les coins des pieces.
 * Punit la curiosite : regarder le Watcher draine violemment la Sante Mentale.
 * Contre-mesure : Detourner le regard et passer son chemin.
 * Si un joueur s'approche a moins de 2,5 m, l'entite se dematerialise dans le neant.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Watcher : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Watcher();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	void TeleportAway();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Watcher|Visuals")
	TObjectPtr<UPointLightComponent> EyeGlow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher", meta = (ClampMin = "1.0"))
	float SanityDrainPerSecondLooking = 18.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher", meta = (ClampMin = "100.0"))
	float ObservationRange = 1800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher", meta = (ClampMin = "50.0"))
	float TeleportProximityThreshold = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Watcher", meta = (ClampMin = "0.5", ClampMax = "1.0"))
	float GazeDotThreshold = 0.85f;

private:
	float CurrentEyeIntensity = 220.0f;
};
