#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalDoorActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UAudioComponent;
class USoundBase;
class AScavengerCharacter;
class ALiminalEntity;

UENUM(BlueprintType)
enum class EDoorState : uint8
{
	Closed,
	Opening,
	Open,
	Closing,
	LockedClosed,
	Broken
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDoorStateChanged, EDoorState, NewState, AActor*, InInstigator);

/**
 * Porte interactive physique des niveaux liminaux.
 * Supporte : ouverture/fermeture manuelle par le joueur, verrouillage par cle/keycard,
 * enfoncement par les entites IA, generation de bruit pour les Hounds,
 * et grincement optionnel basé sur la vitesse d'ouverture.
 * Server-authoritative : l'etat est replique, le client interpole visuellement.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalDoorActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalDoorActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Joueur tente d'interagir avec la porte (ouvrir/fermer). */
	UFUNCTION(BlueprintCallable, Category = "Door")
	void Interact(AScavengerCharacter* InInstigator);

	/** Une entite IA tente de casser la porte. */
	UFUNCTION(BlueprintCallable, Category = "Door|AI")
	void AIBreakDoor(ALiminalEntity* Entity);

	/** Une entite IA tente d'ouvrir normalement. */
	UFUNCTION(BlueprintCallable, Category = "Door|AI")
	void AIOpenDoor(ALiminalEntity* Entity);

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Lock();

	UFUNCTION(BlueprintCallable, Category = "Door")
	void Unlock();

	UFUNCTION(BlueprintPure, Category = "Door")
	EDoorState GetDoorState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsOpen() const { return CurrentState == EDoorState::Open || CurrentState == EDoorState::Opening; }

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsLocked() const { return CurrentState == EDoorState::LockedClosed; }

	UFUNCTION(BlueprintPure, Category = "Door")
	bool IsBroken() const { return CurrentState == EDoorState::Broken; }

	/** Tag de la cle requise pour deverrouiller (vide = pas de cle necessaire). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Door|Lock")
	FName RequiredKeyTag;

	UPROPERTY(BlueprintAssignable, Category = "Door")
	FOnDoorStateChanged OnDoorStateChanged;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_DoorState();

	UFUNCTION()
	void OnRep_DoorAngle();

	UFUNCTION()
	void OnInteractionOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void SetState(EDoorState NewState, AActor* InInstigator);
	void UpdateDoorSwing(float DeltaSeconds);
	void EmitDoorNoise(float Loudness);

	/** Composants visuels */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> DoorRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> DoorFrameMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> DoorPivot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> DoorPanelMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Door", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionZone;

	/** Parametres */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Motion", meta = (ClampMin = "30.0", ClampMax = "120.0"))
	float MaxOpenAngleDegrees = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Motion", meta = (ClampMin = "30.0"))
	float OpenSpeed = 180.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Motion", meta = (ClampMin = "30.0"))
	float CloseSpeed = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|AI", meta = (ClampMin = "1.0"))
	float AIBreakHitsRequired = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|AI", meta = (ClampMin = "0.5"))
	float AIBreakCooldown = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Noise", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float CreakLoudness = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Noise", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float SlamLoudness = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Noise", meta = (ClampMin = "0.0", ClampMax = "3.0"))
	float BreakLoudness = 2.5f;

	/** Auto-fermeture apres un delai (0 = jamais). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Door|Motion", meta = (ClampMin = "0.0"))
	float AutoCloseDelaySeconds = 0.0f;

	/** Sons */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Audio")
	TSoftObjectPtr<USoundBase> OpenSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Audio")
	TSoftObjectPtr<USoundBase> CloseSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Audio")
	TSoftObjectPtr<USoundBase> LockedSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Audio")
	TSoftObjectPtr<USoundBase> BreakSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Door|Audio")
	TSoftObjectPtr<USoundBase> CreakSound;

private:
	UPROPERTY(ReplicatedUsing = OnRep_DoorState)
	EDoorState CurrentState = EDoorState::Closed;

	UPROPERTY(ReplicatedUsing = OnRep_DoorAngle)
	float CurrentAngle = 0.0f;

	float TargetAngle = 0.0f;
	float AIBreakProgress = 0.0f;
	float AIBreakCooldownTimer = 0.0f;
	float AutoCloseTimer = 0.0f;
	float OpenDirection = 1.0f;
};
