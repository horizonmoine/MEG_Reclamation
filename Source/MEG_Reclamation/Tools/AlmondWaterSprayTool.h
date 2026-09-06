#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "AlmondWaterSprayTool.generated.h"

UCLASS(Blueprintable)
class MEG_RECLAMATION_API AAlmondWaterSprayTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AAlmondWaterSprayTool();

protected:
	void Tick(float DeltaSeconds) override;

	virtual void ApplyCalmingEffect(AActor* Target, float DeltaSeconds);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spray", meta = (ClampMin = "0.0"))
	float SprayRange = 150.0f;

private:
	AActor* TraceSprayTarget() const;
};
