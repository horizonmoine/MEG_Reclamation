#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "AI/VoiceMimicryComponent.h"
#include "LiminalEntity_Skinwalker.generated.h"

class AScavengerCharacter;
class USpotLightComponent;

UENUM(BlueprintType)
enum class ESkinwalkerState : uint8
{
	Stalking,
	MimickingVoice,
	Ambushing,
	Retreating
};

/**
 * Entite Skinwalker (Etape 11 de la Roadmap).
 * Isole les joueurs en mimant la voix enregistree de leurs coequipiers.
 * Exploite le buffer vocal en RAM pour semer la confusion et la paranoïa.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Skinwalker : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Skinwalker();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Entity|Skinwalker")
	ESkinwalkerState GetSkinwalkerState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Entity|Skinwalker")
	UVoiceMimicryComponent* GetVoiceMimicryComponent() const { return VoiceMimicry; }

	UFUNCTION(BlueprintPure, Category = "Entity|Skinwalker")
	bool IsTargetIsolated(const AScavengerCharacter* Target) const;

	UFUNCTION(BlueprintCallable, Category = "Entity|Skinwalker")
	bool AttemptVoiceLure(AScavengerCharacter* Target);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Skinwalker|Visuals")
	TObjectPtr<USpotLightComponent> GlitchedHeadlamp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Voice", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UVoiceMimicryComponent> VoiceMimicry;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Skinwalker", meta = (ClampMin = "500.0"))
	float IsolationDistanceThreshold = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Skinwalker", meta = (ClampMin = "3.0"))
	float MimicryCooldownSeconds = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Skinwalker", meta = (ClampMin = "100.0"))
	float AmbushTriggerDistance = 350.0f;

private:
	void UpdateStalkingBehavior(float DeltaSeconds);
	AScavengerCharacter* FindBestStalkTarget() const;

	ESkinwalkerState CurrentState = ESkinwalkerState::Stalking;
	float MimicryTimer = 0.0f;
	TWeakObjectPtr<AScavengerCharacter> StalkedTarget;
};
