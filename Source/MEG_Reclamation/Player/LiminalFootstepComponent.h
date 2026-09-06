#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalFootstepComponent.generated.h"

class USoundBase;

/**
 * Sons de pas dynamiques du Recuperateur : joue un son spatialisé à la position
 * du porteur. Le variant par PhysicalMaterial arrivera avec les biomes (étape 11).
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalFootstepComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalFootstepComponent();

	void PlayFootstep(float VolumeScale = 1.0f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Footsteps")
	TObjectPtr<USoundBase> FootstepSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Footsteps", meta = (ClampMin = "0.1", ClampMax = "2.0"))
	float PitchJitter = 0.15f;
};
