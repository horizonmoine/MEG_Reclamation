#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "AdrenalineInjectorTool.generated.h"

class AScavengerCharacter;

/**
 * Seringue autoinjectrice pneumatique M.E.G. (style R.E.P.O. / Escape Together).
 * - Injecte sur soi : declenche un rush d'adrenaline (+35% vitesse, endurance infinie, sanite verrouillee).
 * - Injecte sur un equipier au sol/mort : reanimation d'urgence defibrillatoire (+45% PV, +50% sanite).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API AAdrenalineInjectorTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AAdrenalineInjectorTool();

	virtual bool Activate() override;

	UFUNCTION(BlueprintPure, Category = "Tool|Adrenaline")
	int32 GetRemainingDoses() const { return RemainingDoses; }

	UFUNCTION(BlueprintPure, Category = "Tool|Adrenaline")
	int32 GetMaxDoses() const { return MaxDoses; }

	UFUNCTION(BlueprintCallable, Category = "Tool|Adrenaline")
	void SetRemainingDoses(int32 InDoses);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool|Adrenaline")
	int32 MaxDoses = 2;

	UPROPERTY(ReplicatedUsing = OnRep_RemainingDoses, VisibleAnywhere, BlueprintReadOnly, Category = "Tool|Adrenaline")
	int32 RemainingDoses = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool|Adrenaline", meta = (ClampMin = "50.0"))
	float InjectRange = 220.0f;

	UFUNCTION()
	void OnRep_RemainingDoses();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	AScavengerCharacter* TraceTeammateTarget() const;
};
