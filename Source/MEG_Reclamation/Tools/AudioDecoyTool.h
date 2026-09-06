#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "AudioDecoyTool.generated.h"

class UAudioComponent;
class USoundBase;
class UPointLightComponent;

UCLASS(Blueprintable)
class MEG_RECLAMATION_API AAudioDecoyTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AAudioDecoyTool();

	virtual bool Activate() override;
	virtual void Deactivate() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_IsDeployed();

protected:
	UFUNCTION()
	void EmitNoise();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Decoy", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAudioComponent> DecoyAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Decoy|Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> DecoyStatusLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoy")
	TObjectPtr<USoundBase> NoiseSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoy|Noise")
	float Loudness = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Decoy|Noise", meta = (ClampMin = "0.05"))
	float NoiseIntervalSeconds = 0.5f;

private:
	FTimerHandle NoiseTimerHandle;

	UPROPERTY(ReplicatedUsing = OnRep_IsDeployed)
	bool bIsDeployed = false;
};
