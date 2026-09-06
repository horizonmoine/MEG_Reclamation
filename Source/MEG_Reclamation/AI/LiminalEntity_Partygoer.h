#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Partygoer.generated.h"

class AScavengerCharacter;
class UPointLightComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScavengerInfected, AScavengerCharacter*, InfectedScavenger);

/**
 * Entite Partygoer (Etape 11 de la Roadmap).
 * Punit le contact social : transforme le joueur touche en traitre infecte.
 * Déplacement rapide et saccadé, recherche activement le contact physique direct.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Partygoer : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Partygoer();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Entity|Partygoer")
	bool TryInfectScavenger(AScavengerCharacter* Target);

	UFUNCTION(BlueprintPure, Category = "Entity|Partygoer")
	float GetInfectionRadius() const { return InfectionRadius; }

	UPROPERTY(BlueprintAssignable, Category = "Entity|Partygoer")
	FOnScavengerInfected OnScavengerInfected;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Partygoer|Visuals")
	TObjectPtr<UPointLightComponent> BalloonLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Partygoer|Visuals")
	TObjectPtr<UStaticMeshComponent> BalloonMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Partygoer", meta = (ClampMin = "50.0"))
	float InfectionRadius = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Partygoer", meta = (ClampMin = "0.0"))
	float SanityDrainNearPresencePerSecond = 4.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Partygoer", meta = (ClampMin = "100.0"))
	float PresenceRadius = 600.0f;

private:
	void UpdatePursuitAndInfection(float DeltaSeconds);
	AScavengerCharacter* FindNearestLivingScavenger() const;

	float InfectionCooldownTimer = 0.0f;
	float TargetCheckTimer = 0.0f;
	TWeakObjectPtr<AScavengerCharacter> CachedTarget;
};
