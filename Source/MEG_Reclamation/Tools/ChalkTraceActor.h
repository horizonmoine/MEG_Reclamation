#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChalkTraceActor.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;

/**
 * Marque persistante au sol ou sur les murs tracée à la craie phosphorescente.
 * Permet à l'escouade de baliser son chemin dans les labyrinthes massifs.
 */
UCLASS()
class MEG_RECLAMATION_API AChalkTraceActor : public AActor
{
	GENERATED_BODY()

public:
	AChalkTraceActor();

	UFUNCTION(BlueprintCallable, Category = "Chalk")
	void SetMarkColor(const FLinearColor& InColor);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Chalk")
	TObjectPtr<UStaticMeshComponent> MarkMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chalk")
	float LifespanSeconds = 600.0f;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;
};
