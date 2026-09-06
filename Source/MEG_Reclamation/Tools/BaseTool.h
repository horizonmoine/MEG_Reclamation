#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseTool.generated.h"

class UStaticMeshComponent;

/**
 * Classe de base des outils de maintenance (Flash Strobe, Micro-Onde Sonique, Spray Eau d'Amande, Leurre Audio).
 *
 * Server-authoritative : Activate()/Deactivate() sont le point d'entree autoritaire.
 * Le layering RPC (Server Reliable) sera branche sur le character possesseur a l'etape 3 de la roadmap.
 */
UCLASS(Abstract, Blueprintable)
class MEG_RECLAMATION_API ABaseTool : public AActor
{
	GENERATED_BODY()

public:
	ABaseTool();

	UFUNCTION(BlueprintCallable, Category = "Tool")
	virtual bool Activate();

	UFUNCTION(BlueprintCallable, Category = "Tool")
	virtual void Deactivate();

	UFUNCTION(BlueprintPure, Category = "Tool")
	bool IsActive() const;

	UFUNCTION(BlueprintPure, Category = "Tool")
	virtual bool CanActivate() const;

	UFUNCTION(BlueprintPure, Category = "Tool|Battery")
	float GetBatteryCharge() const;

	UFUNCTION(BlueprintPure, Category = "Tool|Battery")
	float GetMaxBatteryCharge() const;

	UFUNCTION(BlueprintPure, Category = "Tool|Battery")
	float GetBatteryNormalized() const { return (MaxBatteryCharge > 0.0f) ? FMath::Clamp(BatteryCharge / MaxBatteryCharge, 0.0f, 1.0f) : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Tool")
	UStaticMeshComponent* GetToolMesh() const { return ToolMesh; }

	UFUNCTION(BlueprintCallable, Category = "Tool|Battery")
	void SetBatteryCharge(float NewCharge);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnRep_IsActive();

	UFUNCTION()
	virtual void OnRep_BatteryCharge();

protected:
	virtual void ConsumeBattery(float DeltaSeconds);

	void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tool", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> ToolMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool|Battery", meta = (ClampMin = "0.0"))
	float MaxBatteryCharge = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool|Battery", ReplicatedUsing = OnRep_BatteryCharge, meta = (ClampMin = "0.0"))
	float BatteryCharge = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool|Battery", meta = (ClampMin = "0.0"))
	float ActiveBatteryDrainPerSecond = 5.0f;

	UPROPERTY(ReplicatedUsing = OnRep_IsActive, BlueprintReadOnly, Category = "Tool")
	bool bIsActive = false;
};
