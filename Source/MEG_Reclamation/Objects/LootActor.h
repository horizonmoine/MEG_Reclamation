#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LootActor.generated.h"

class UItemData;
class UStaticMeshComponent;
class USoundBase;
class AScavengerCharacter;

/**
 * Objet recuperable physique (Chaos) : mesh en simulation, poids issu de son UItemData.
 * Les collisions violentes emettent du bruit perceptible par l'IA (AISense_Hearing).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALootActor : public AActor
{
	GENERATED_BODY()

public:
	ALootActor();

	UFUNCTION(BlueprintPure, Category = "Loot")
	float GetWeightKg() const;

	UFUNCTION(BlueprintPure, Category = "Loot")
	int32 GetCreditsValue() const;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetRandomizedStats(float InWeight, int32 InCredits);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void HighlightLoot(bool bEnable);

	void SetItemData(UItemData* InItemData);

	UFUNCTION(BlueprintPure, Category = "Loot")
	bool IsClaimed() const;

	void SetClaimed(AScavengerCharacter* Claimant);

	UPrimitiveComponent* GetLootPrimitive() const;

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void SetCustomLoot(UStaticMesh* InMesh, USoundBase* InSound, float InWeight, int32 InCredits, FName InItemId = NAME_None);

	UFUNCTION(BlueprintPure, Category = "Loot")
	FName GetLootItemId() const { return LootItemId; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnLootHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		FVector NormalImpulse, const FHitResult& Hit);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ClaimedBy();

	UFUNCTION()
	void OnRep_ItemData();

	UFUNCTION()
	void OnRep_CustomMesh();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", ReplicatedUsing = OnRep_ItemData)
	TObjectPtr<UItemData> ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", ReplicatedUsing = OnRep_CustomMesh)
	TObjectPtr<UStaticMesh> CustomMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Loot")
	FName LootItemId = NAME_None;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_ClaimedBy, Category = "Loot")
	TObjectPtr<AScavengerCharacter> ClaimedBy;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.0"))
	float RandomizedWeightKg = 0.0f;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0"))
	int32 RandomizedCreditsValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot", meta = (ClampMin = "0.0"))
	float FallbackWeightKg = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Noise", meta = (ClampMin = "0.0"))
	float MinImpactNoiseThreshold = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Noise", meta = (ClampMin = "0.0"))
	float MaxImpactNoiseLoudness = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Audio")
	TObjectPtr<USoundBase> ImpactSound;
};
