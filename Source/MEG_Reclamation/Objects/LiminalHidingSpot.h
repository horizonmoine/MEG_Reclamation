#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalHidingSpot.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UCameraComponent;
class AScavengerCharacter;

UENUM(BlueprintType)
enum class EHidingSpotType : uint8
{
	Closet     UMETA(DisplayName = "Placard / Armoire"),
	UnderDesk  UMETA(DisplayName = "Sous un Bureau"),
	Locker     UMETA(DisplayName = "Casier"),
	Curtain    UMETA(DisplayName = "Derriere un Rideau"),
	Crate      UMETA(DisplayName = "Dans une Caisse")
};

/**
 * Cachette interactive des niveaux liminaux.
 * Le joueur entre dans la cachette, la camera se repositionne a l'interieur
 * (vue a travers des fentes/entrebaillements), et le personnage est invisible
 * pour les entites SAUF si elles l'ont vu entrer.
 *
 * Le rythme cardiaque du joueur est audible et s'accelere avec la peur.
 * Server-authoritative : l'etat de presence dans la cachette est replique.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalHidingSpot : public AActor
{
	GENERATED_BODY()

public:
	ALiminalHidingSpot();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Le joueur tente d'entrer/sortir de la cachette. */
	UFUNCTION(BlueprintCallable, Category = "Hiding")
	void TryEnter(AScavengerCharacter* Scavenger);

	UFUNCTION(BlueprintCallable, Category = "Hiding")
	void ForceExit();

	UFUNCTION(BlueprintPure, Category = "Hiding")
	bool IsOccupied() const { return OccupantCharacter.IsValid(); }

	UFUNCTION(BlueprintPure, Category = "Hiding")
	AScavengerCharacter* GetOccupant() const { return OccupantCharacter.Get(); }

	UFUNCTION(BlueprintPure, Category = "Hiding")
	EHidingSpotType GetSpotType() const { return SpotType; }

	/** Point de vue a l'interieur de la cachette (position + rotation de la camera cache). */
	UFUNCTION(BlueprintPure, Category = "Hiding")
	FTransform GetHiddenViewpoint() const;

	/** L'entite peut-elle fouiller cette cachette ? (si elle a vu le joueur entrer). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hiding|AI")
	bool bSearchedByEntity = false;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_OccupantId();

	UFUNCTION()
	void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void EnterHidingSpot(AScavengerCharacter* Scavenger);
	void ExitHidingSpot();
	void UpdateHeartbeatAudio(float DeltaSeconds);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hiding", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hiding", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> SpotMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hiding", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionZone;

	/** Position et rotation de la camera quand le joueur est cache (relative a l'acteur). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding|View")
	FVector HiddenCameraOffset = FVector(0.0f, 0.0f, 60.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding|View")
	FRotator HiddenCameraRotation = FRotator(0.0f, 0.0f, 0.0f);

	/** FOV restreint a l'interieur de la cachette (vue a travers des fentes). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hiding|View", meta = (ClampMin = "30.0", ClampMax = "90.0"))
	float HiddenFOV = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hiding")
	EHidingSpotType SpotType = EHidingSpotType::Closet;

	/** Duree d'animation d'entree/sortie (interpolation). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hiding|Motion", meta = (ClampMin = "0.1"))
	float EnterExitDuration = 0.6f;

	/** Volume du rythme cardiaque quand cache et en danger. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Hiding|Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float HeartbeatBaseVolume = 0.3f;

private:
	TWeakObjectPtr<AScavengerCharacter> OccupantCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_OccupantId)
	int32 OccupantNetId = 0;

	FVector SavedPlayerLocation;
	FRotator SavedPlayerRotation;
	float SavedFOV = 90.0f;
	bool bIsTransitioning = false;
	float TransitionAlpha = 0.0f;
	bool bEntering = false;

	float HeartbeatTimer = 0.0f;
	float HeartbeatRate = 1.0f;
};
