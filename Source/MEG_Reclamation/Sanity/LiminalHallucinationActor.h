#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sanity/LiminalSanityTypes.h"
#include "LiminalHallucinationActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Acteur d'hallucination asymetrique (client-side uniquement).
 * N'existe que dans le monde du joueur souffrant de basse sanite.
 * S'evapore ou disparait brutalement des que le joueur s'en approche.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalHallucinationActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalHallucinationActor();

	virtual void Tick(float DeltaSeconds) override;

	void ConfigureHallucination(EHallucinationType InType, float InLifespan = 12.0f);

	UFUNCTION(BlueprintPure, Category = "Sanity|Hallucination")
	EHallucinationType GetHallucinationType() const { return HallucinationType; }

	UFUNCTION(BlueprintCallable, Category = "Sanity|Hallucination")
	void Vanish();

	UFUNCTION(BlueprintPure, Category = "Sanity|Hallucination")
	bool IsVanishing() const { return bIsVanishing; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Sanity|Visual")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|Settings")
	float VanishDistance = 350.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|Settings")
	float VanishFadeDuration = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sanity|Settings")
	float JitterIntensity = 2.5f;

private:
	EHallucinationType HallucinationType = EHallucinationType::ShadowSilhouette;
	bool bIsVanishing = false;
	float VanishTimer = 0.0f;
	float LifeTimer = 0.0f;
	float MaxLifespan = 12.0f;
	FVector InitialLocation;
};
