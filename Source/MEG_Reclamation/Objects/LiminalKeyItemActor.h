#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalKeyItemActor.generated.h"

class UStaticMeshComponent;
class USphereComponent;
class AScavengerCharacter;
class ALiminalDoorActor;

UENUM(BlueprintType)
enum class EKeycardLevel : uint8
{
	Level1_Standard UMETA(DisplayName = "Niveau 1 - Scientifique Standard"),
	Level2_Maintenance UMETA(DisplayName = "Niveau 2 - Maintenance & Tuyauterie"),
	Level3_Security UMETA(DisplayName = "Niveau 3 - Securite & Armurerie"),
	Master_Overseer UMETA(DisplayName = "M.E.G. Passe-Partout Superviseur")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnKeyItemCollected, AScavengerCharacter*, Collector, const FName&, KeyId);

/**
 * Item physique de clef / pass d'acces M.E.G.
 * Peut etre ramasse dans le monde et utilise pour deverrouiller les portes (ALiminalDoorActor).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalKeyItemActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalKeyItemActor();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Ramasse la clef */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Key")
	bool TryCollectKey(AScavengerCharacter* InScavenger);

	UFUNCTION(BlueprintPure, Category = "Liminal|Key")
	FName GetKeyId() const { return KeyId; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Key")
	EKeycardLevel GetKeycardLevel() const { return KeycardLevel; }

	UFUNCTION(BlueprintCallable, Category = "Liminal|Key")
	void SetKeyData(const FName& InKeyId, EKeycardLevel InLevel, const FText& InDisplayName);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Key")
	FOnKeyItemCollected OnKeyCollected;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liminal|Key")
	TObjectPtr<USphereComponent> CollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Liminal|Key")
	TObjectPtr<UStaticMeshComponent> KeyMesh;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Liminal|Key")
	FName KeyId = FName(TEXT("Key_SectorA"));

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Liminal|Key")
	EKeycardLevel KeycardLevel = EKeycardLevel::Level1_Standard;

	UPROPERTY(Replicated, EditAnywhere, BlueprintReadOnly, Category = "Liminal|Key")
	FText DisplayName;

	UPROPERTY(ReplicatedUsing = OnRep_IsCollected)
	bool bIsCollected = false;

	UFUNCTION()
	void OnRep_IsCollected();
};
