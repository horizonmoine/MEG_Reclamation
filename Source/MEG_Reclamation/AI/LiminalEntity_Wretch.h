#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Wretch.generated.h"

/**
 * Wretch (L'Insense des Backrooms) :
 * Ancien survivant ayant perdu la raison suite a une privation prolongee d'eau d'amande.
 * Aveugle et hagard, mais dote d'une sensibilite acoustique extreme.
 * Punit le bruit et la panique : patrouille lentement, mais fonce au sprint vers la source du son.
 * Contre-mesure : Discretion totale (ne pas courir) ou deploiement du leurre audio.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Wretch : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Wretch();

	virtual void Tick(float DeltaSeconds) override;

	void AlertToNoise(const FVector& NoiseLocation, float Loudness);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wretch|Movement", meta = (ClampMin = "50.0"))
	float WanderSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wretch|Movement", meta = (ClampMin = "200.0"))
	float FrenzySprintSpeed = 650.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wretch|State")
	bool bIsFrenzied = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wretch|State")
	float FrenzyDurationRemaining = 0.0f;

	FVector TargetNoiseLocation = FVector::ZeroVector;
};
