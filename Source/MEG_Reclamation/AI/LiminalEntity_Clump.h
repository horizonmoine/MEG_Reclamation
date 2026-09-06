#pragma once

#include "CoreMinimal.h"
#include "AI/LiminalEntity.h"
#include "LiminalEntity_Clump.generated.h"

class USphereComponent;
class AScavengerCharacter;

/**
 * Clump : Entite tentaculaire statique embusquee dans les recoins.
 * Capture les joueurs distraits qui passent trop pres, les immobilise et draine leur sante.
 * Contre-mesure : frapper en melee ou declencher le micro-ondes sonique pour forcer le relachement.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalEntity_Clump : public ALiminalEntity
{
	GENERATED_BODY()

public:
	ALiminalEntity_Clump();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Clump")
	void ReleaseGrabbedPlayer();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnGrabTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clump", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USphereComponent> GrabTrigger;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clump", meta = (ClampMin = "50.0"))
	float GrabRadius = 140.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clump", meta = (ClampMin = "1.0"))
	float ConstrictionDamagePerSecond = 18.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Clump")
	TWeakObjectPtr<AScavengerCharacter> GrabbedPlayer;
};
