#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "HarmonicResonatorTool.generated.h"

/**
 * Résonateur Harmonique (No-Clip Tool) - GDD Canonique 1.6.
 * Traverse un mur pour une fuite d'urgence.
 * 70% succes (traversee propre) / 30% echec critique (teleportation aleatoire / degats de desynchronisation).
 * Trouvable en loot uniquement, jamais achetable, usage unique.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API AHarmonicResonatorTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AHarmonicResonatorTool();

	virtual bool Activate() override;
	virtual bool CanActivate() const override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NoClip")
	float MaxWallThickness = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NoClip")
	float SuccessChance = 0.70f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NoClip")
	bool bIsExpended = false;
};
