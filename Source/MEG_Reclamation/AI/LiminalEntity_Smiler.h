#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Smiler.generated.h"

class UPointLightComponent;

/**
 * Smiler : Entite insidieuse du Niveau 0.
 * Invisible dans l'obscurite totale (yeux lumineux visibles a courte portee).
 * S'il est eclaire directement par une lampe torche, il hurle et charge.
 * S'il est regarde fixement dans le noir sans lumiere, il est paralyse.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Smiler : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Smiler();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Smiler")
	bool IsCharging() const { return bIsCharging; }

	UFUNCTION(BlueprintPure, Category = "Smiler")
	bool IsParalyzedByStare() const { return bIsParalyzedByStare; }

	UFUNCTION(BlueprintPure, Category = "Smiler")
	float GetChargeSpeed() const { return ChargeSpeed; }

	UFUNCTION(BlueprintPure, Category = "Smiler")
	float GetNormalStalkSpeed() const { return NormalStalkSpeed; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smiler|Eyes")
	TObjectPtr<UPointLightComponent> EyeGlowLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smiler|Eyes")
	TObjectPtr<UPointLightComponent> EyeGlowRight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Smiler|Mouth")
	TObjectPtr<UPointLightComponent> SmileMouthGlow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smiler|Combat", meta = (ClampMin = "100.0"))
	float ChargeSpeed = 850.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smiler|Combat", meta = (ClampMin = "50.0"))
	float NormalStalkSpeed = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smiler|Detection", meta = (ClampMin = "100.0"))
	float StareDetectionRange = 1400.0f;

private:
	void UpdateSensoryReactions(float DeltaSeconds);

	bool bIsCharging = false;
	bool bIsParalyzedByStare = false;
	float StareTimer = 0.0f;
};
