#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "LiminalPlayerState.generated.h"

UENUM(BlueprintType)
enum class EScavengerStatus : uint8
{
	Alive,
	Downed,
	Dead,
	Extracted,
	Spectating
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScavengerSanityChanged, float, OldSanity, float, NewSanity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScavengerHealthChanged, float, OldHealth, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScavengerStatusChanged, EScavengerStatus, OldStatus, EScavengerStatus, NewStatus);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnScavengerCreditsChanged, int32, NewCredits);

/**
 * Source de verite repliquee de l'etat de session d'un Recuperateur.
 * Survit a la destruction du pawn (mort, spectateur, SeamlessTravel).
 * Toute mutation passe par les methodes Auth* (serveur uniquement).
 * Le pawn (AScavengerCharacter) lit et affiche ; il ne stocke plus ces valeurs.
 */
UCLASS()
class MEG_RECLAMATION_API ALiminalPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ALiminalPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OverrideWith(APlayerState* PlayerState) override;

	// --- Mutations : serveur uniquement (HasAuthority) ---

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthApplySanityDelta(float Delta);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthApplyHealthDelta(float Delta);

	/** Retourne false si la transition est invalide (ex : Downed -> Spectating). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	bool AuthSetStatus(EScavengerStatus NewStatus);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthAddCarriedCredits(int32 Delta);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthSetCarriedCredits(int32 NewCredits);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthSetInfectedPartygoer(bool bInfected);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthSetReady(bool bReady);

	/** Reanimation : uniquement depuis Downed. */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthRevive(float HealthPercent = 0.4f, float SanityPercent = 0.5f);

	/** Remise a zero complete pour une nouvelle incursion (appele par le GameMode). */
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void AuthResetForNewRun();

	// --- Lecture (tous les clients) ---

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	float GetSanity() const { return Sanity; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	float GetSanityPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	EScavengerStatus GetStatus() const { return Status; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	int32 GetCarriedCredits() const { return CarriedCredits; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	bool IsInfectedPartygoer() const { return bIsInfectedPartygoer; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	bool IsAlive() const { return Status == EScavengerStatus::Alive; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	bool IsDowned() const { return Status == EScavengerStatus::Downed; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	bool IsDead() const { return Status == EScavengerStatus::Dead; }

	/** Table des transitions autorisees. Statique pour etre testable sans monde. */
	static bool IsStatusTransitionValid(EScavengerStatus From, EScavengerStatus To);

	// --- Evenements (declenches sur serveur et clients) ---

	UPROPERTY(BlueprintAssignable, Category = "Scavenger|Events")
	FOnScavengerSanityChanged OnSanityChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scavenger|Events")
	FOnScavengerHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scavenger|Events")
	FOnScavengerStatusChanged OnStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Scavenger|Events")
	FOnScavengerCreditsChanged OnCarriedCreditsChanged;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Sanity, VisibleInstanceOnly, Category = "Scavenger|State")
	float Sanity;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleInstanceOnly, Category = "Scavenger|State")
	float Health;

	UPROPERTY(ReplicatedUsing = OnRep_Status, VisibleInstanceOnly, Category = "Scavenger|State")
	EScavengerStatus Status;

	UPROPERTY(ReplicatedUsing = OnRep_CarriedCredits, VisibleInstanceOnly, Category = "Scavenger|State")
	int32 CarriedCredits;

	UPROPERTY(ReplicatedUsing = OnRep_InfectedPartygoer, VisibleInstanceOnly, Category = "Scavenger|State")
	bool bIsInfectedPartygoer;

	UPROPERTY(ReplicatedUsing = OnRep_Ready, VisibleInstanceOnly, Category = "Scavenger|State")
	bool bIsReady;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Tuning", meta = (ClampMin = "1.0"))
	float MaxSanity;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Tuning", meta = (ClampMin = "1.0"))
	float MaxHealth;

	UFUNCTION()
	void OnRep_Sanity(float OldSanity);

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UFUNCTION()
	void OnRep_Status(EScavengerStatus OldStatus);

	UFUNCTION()
	void OnRep_CarriedCredits();

	UFUNCTION()
	void OnRep_InfectedPartygoer();

	UFUNCTION()
	void OnRep_Ready();

private:
	void HandleIncapacitation();
	void SetSanityInternal(float NewSanity);
	void SetHealthInternal(float NewHealth);
};
