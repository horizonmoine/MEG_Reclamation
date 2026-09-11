#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalSafeZoneVolume.generated.h"

class UBoxComponent;

/**
 * Zone sure placable dans une map : coupe la pression de sanite, restaure lentement la sanite
 * et bloque les spawns d'entites. A poser sur l'ensemble du Hub Base Alpha, l'infirmerie,
 * l'interieur du sas B.R.C. et les ancres de realite fixes des biomes.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalSafeZoneVolume : public AActor
{
	GENERATED_BODY()

public:
	ALiminalSafeZoneVolume();

	UFUNCTION(BlueprintPure, Category = "SafeZone")
	bool ContainsLocation(const FVector& Location) const;

	UFUNCTION(BlueprintPure, Category = "SafeZone")
	float GetSanityRestorePerSecond() const { return SanityRestorePerSecond; }

	UFUNCTION(BlueprintPure, Category = "SafeZone")
	bool BlocksEntitySpawns() const { return bBlocksEntitySpawns; }

	UFUNCTION(BlueprintCallable, Category = "SafeZone")
	void SetBoxExtent(const FVector& NewExtent);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "SafeZone")
	TObjectPtr<UBoxComponent> Bounds;

	/** Sanite restauree par seconde aux joueurs a l'interieur (0 = aucune restauration). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone", meta = (ClampMin = "0.0"))
	float SanityRestorePerSecond = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone")
	bool bBlocksEntitySpawns = true;
};
