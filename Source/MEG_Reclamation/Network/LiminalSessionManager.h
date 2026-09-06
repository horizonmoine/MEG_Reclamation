#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LiminalSessionManager.generated.h"

class AScavengerCharacter;
class AController;

USTRUCT(BlueprintType)
struct MEG_RECLAMATION_API FPlayerStasisRecord
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	FString PlayerUniqueId;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	float Health = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	float Sanity = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	int32 CarriedCredits = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	float CarriedWeightKg = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	FVector SavedLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	FRotator SavedRotation = FRotator::ZeroRotator;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	TArray<FName> CarriedTags;

	UPROPERTY(BlueprintReadOnly, Category = "Session|Stasis")
	double DisconnectTimestamp = 0.0;

	UPROPERTY()
	TWeakObjectPtr<AScavengerCharacter> StasisPawn;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerEnteredStasis, const FString&, PlayerId, const FVector&, StasisLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPlayerRestoredFromStasis, const FString&, PlayerId, AScavengerCharacter*, RestoredCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHostMigrationTriggered, const FString&, NewHostAddress);

/**
 * ULiminalSessionManager - Subsystem de session multijoueur persistant (Chantier 9).
 * Gere la stase de reconnexion en cours d'expedition, la validation anti-speedhack
 * et les evenements de migration d'hote.
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalSessionManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Enregistre la deconnexion imprevue d'un joueur et place son corps/donnees en stase. */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Session")
	bool RegisterPlayerDisconnect(AController* ExitingController);

	/** Restaure les donnees et l'inventaire d'un joueur qui se reconnecte. */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Session")
	bool TryRestorePlayer(AController* JoiningController);

	/** Verifie si un joueur a des donnees en stase. */
	UFUNCTION(BlueprintPure, Category = "Liminal|Session")
	bool HasStasisRecord(const FString& PlayerUniqueId) const;

	/** Validation anti-triche server-side (anti-teleport et check de vitesse maximale). */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Session")
	bool ValidatePlayerMovement(APawn* InPawn, const FVector& OldLocation, const FVector& NewLocation, float DeltaTime);

	/** Declenche la procedure de repli / migration d'hote. */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Session")
	void TriggerHostMigration(const FString& NewHostAddress);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Session")
	FOnPlayerEnteredStasis OnPlayerEnteredStasis;

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Session")
	FOnPlayerRestoredFromStasis OnPlayerRestoredFromStasis;

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Session")
	FOnHostMigrationTriggered OnHostMigrationTriggered;

protected:
	UPROPERTY()
	TMap<FString, FPlayerStasisRecord> ActiveStasisRecords;

	/** Vitesse max toleree en units/sec avant flag de hack (comprenant sprint et boost d'adrenaline). */
	UPROPERTY(EditDefaultsOnly, Category = "Liminal|Security")
	float MaxAllowedPawnSpeed = 1600.0f;

	FString GetUniquePlayerIdFromController(AController* Controller) const;
};
