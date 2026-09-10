#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "LiminalEntity.generated.h"

class UAIPerceptionComponent;
class ALiminalAIController;
class UBehaviorTree;
class UStaticMeshComponent;
class USkeletalMesh;
class UAnimInstance;
class UAnimMontage;
class USoundBase;

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	Standard,
	Smiler,
	Hound,
	Duller,
	Clump,
	Deathmoth,
	Skinwalker,
	Partygoer,
	Jerry,
	Watcher,
	Wretch,
	Hydrolitis
};

/**
 * Classe mere de toutes les entites hostiles des niveaux liminaux.
 * Server-authoritative : l'entite est possedee et simulee par le serveur uniquement.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalEntity : public ACharacter
{
	GENERATED_BODY()

public:
	ALiminalEntity();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintPure, Category = "Entity")
	EMonsterType GetMonsterType() const;

	UAIPerceptionComponent* GetPerceptionComponent() const;

	UBehaviorTree* GetInitialBehaviorTree() const;

	UFUNCTION(BlueprintCallable, Category = "Entity|Combat")
	void ApplyStun(float DurationSeconds);

	UFUNCTION(BlueprintCallable, Category = "Entity|Combat")
	void ApplyCalm(float DurationSeconds);

	UFUNCTION(BlueprintPure, Category = "Entity|Combat")
	bool IsStunned() const;

	UFUNCTION(BlueprintPure, Category = "Entity|Combat")
	bool IsCalmed() const;

	UFUNCTION(BlueprintPure, Category = "Entity|Combat")
	bool CanAttack() const;

	UFUNCTION(BlueprintCallable, Category = "Entity|Combat")
	bool PerformMeleeAttack(AActor* Target);

	UFUNCTION(BlueprintPure, Category = "Entity|Combat")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure, Category = "Entity|Audio")
	USoundBase* GetAttackSound() const { return AttackSound; }

	UFUNCTION(BlueprintPure, Category = "Entity|Audio")
	USoundBase* GetAggroSound() const { return AggroSound; }

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Entity|Visual")
	void SetEntityVisualScale(const FVector& Scale3D);

	UFUNCTION(BlueprintCallable, Category = "Entity|Visual")
	void SetEntityVisualVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "Entity|Combat")
	int32 OnAttackNotify(FName SocketName = NAME_None, float Radius = -1.0f, float ForwardDistance = -1.0f,
		float DamageOverride = -1.0f, TSubclassOf<UDamageType> DamageType = nullptr, bool bDrawDebug = false);

	UFUNCTION(BlueprintPure, Category = "Entity|Mesh")
	USkeletalMeshComponent* GetSkeletalMesh() const { return GetMesh(); }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity")
	EMonsterType MonsterType = EMonsterType::Standard;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Audio")
	TObjectPtr<USoundBase> AttackSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Audio")
	TObjectPtr<USoundBase> AggroSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|AI")
	TObjectPtr<UBehaviorTree> InitialBehaviorTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAIPerceptionComponent> AIPerception;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> BodyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity")
	TSoftObjectPtr<UStaticMesh> DefaultBodyMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Mesh")
	TSoftObjectPtr<USkeletalMesh> DefaultSkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
	TSubclassOf<UAnimInstance> DefaultAnimClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
	TObjectPtr<UAnimMontage> HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Combat")
	FName DefaultAttackSocket = FName(TEXT("AttackSocket"));

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "5.0"))
	float DefaultAttackTraceRadius = 45.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "10.0"))
	float DefaultAttackTraceDistance = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Audio")
	TObjectPtr<USoundBase> AttackImpactSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "0.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Entity|Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage = 35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "50.0"))
	float AttackRange = 160.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity|Combat", meta = (ClampMin = "0.1"))
	float AttackCooldownSeconds = 1.5f;

private:
	// One accepted attack opens one impact window, consumed by its first notify.
	bool bAttackImpactPending = false;
	float StunTimer = 0.0f;
	float CalmTimer = 0.0f;
	float AttackCooldownTimer = 0.0f;
	float OriginalWalkSpeed = 500.0f;
};
