#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "ChalkMarkerTool.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChalkMarkPlaced, const FVector&, Location, int32, RemainingCount);

/**
 * Craie & Balises de repérage (Outil Phase 2 - Etape 11 de la Roadmap).
 * Permet de marquer le sol ou les cloisons pour naviguer sans boussole
 * dans la géométrie labyrinthique et non-euclidienne des Backrooms.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API AChalkMarkerTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AChalkMarkerTool();

	virtual bool Activate() override;
	virtual bool CanActivate() const override;

	UFUNCTION(BlueprintCallable, Category = "Chalk")
	bool PlaceMarkOnSurface();

	UFUNCTION(BlueprintPure, Category = "Chalk")
	int32 GetRemainingMarks() const { return RemainingMarks; }

	UFUNCTION(BlueprintPure, Category = "Chalk")
	int32 GetMaxMarks() const { return MaxMarks; }

	UPROPERTY(BlueprintAssignable, Category = "Chalk")
	FOnChalkMarkPlaced OnChalkMarkPlaced;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chalk", meta = (ClampMin = "1"))
	int32 MaxMarks = 16;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chalk")
	int32 RemainingMarks = 16;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chalk", meta = (ClampMin = "50.0"))
	float PlacementTraceDistance = 250.0f;
};
