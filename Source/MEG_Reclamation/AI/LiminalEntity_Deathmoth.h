#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Deathmoth.generated.h"

class AScavengerCharacter;
class UPointLightComponent;

/**
 * Deathmoth (Phalene de la Mort) :
 * Insecte volant colossal des Backrooms.
 * Attire par la lumiere vive : fonce en plongeon sur les lampes et les flashs.
 * Percute le joueur et peut lui faire lacher son equipement.
 * Contre-mesure : Eteindre immediatement toute source lumineuse.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Deathmoth : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Deathmoth();

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deathmoth|Visuals")
	TObjectPtr<UPointLightComponent> AbdomenBioluminescence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deathmoth|Flight", meta = (ClampMin = "100.0"))
	float FlightHoverAltitude = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deathmoth|Flight", meta = (ClampMin = "200.0"))
	float DiveSpeed = 750.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Deathmoth|Perception", meta = (ClampMin = "300.0"))
	float LightAttractionRadius = 2200.0f;
};
