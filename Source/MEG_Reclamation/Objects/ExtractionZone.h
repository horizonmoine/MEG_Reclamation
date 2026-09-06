#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExtractionZone.generated.h"

class AScavengerCharacter;
class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class ALiminalBreakerActor;
class ALiminalKeypadActor;

/**
 * Zone d'extraction conditionnelle M.E.G. :
 * Livre le loot porte par un Recuperateur au QuotaManager (progression du quota).
 * Peut etre verrouillee jusqu'au retablissement du courant (Breaker) ou saisie du digicode (Keypad).
 * Server-authoritative : l'overlap et la validation ne sont traites que cote serveur.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API AExtractionZone : public AActor
{
	GENERATED_BODY()

public:
	AExtractionZone();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Extraction")
	bool IsExtractionUnlocked() const;

	UFUNCTION(BlueprintCallable, Category = "Extraction")
	void SetRequiresPower(bool bReq, ALiminalBreakerActor* InBreaker = nullptr);

	UFUNCTION(BlueprintCallable, Category = "Extraction")
	void SetRequiresKeypad(bool bReq, ALiminalKeypadActor* InKeypad = nullptr);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void DeliverLoot(AScavengerCharacter* Scavenger);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extraction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> ZoneBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extraction|Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> FrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extraction|Visuals", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ExitSignMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Extraction", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> MarkerLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Extraction", meta = (ClampMin = "0.0"))
	float CooldownSeconds = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extraction|Puzzle")
	bool bRequiresPower = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extraction|Puzzle")
	TObjectPtr<ALiminalBreakerActor> RequiredBreaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extraction|Puzzle")
	bool bRequiresKeypad = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Extraction|Puzzle")
	TObjectPtr<ALiminalKeypadActor> RequiredKeypad;

private:
	double LastDeliveryTime = -1.0e9;
};
