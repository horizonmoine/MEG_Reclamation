#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LiminalZoneRulesSubsystem.generated.h"

class ALiminalSafeZoneVolume;

/**
 * Regles de zone : ou la pression de sanite, les hallucinations et les spawns d'entites
 * sont autorises. Source unique de verite pour "suis-je en danger ici ?".
 *
 * - Hostile : un ALiminalGameState existe et sa phase est Incursion ou Collapsing.
 *   Le Hub, le menu, le lobby et le sas non scelle ne sont jamais hostiles.
 * - Zone sure : interieur d'un ALiminalSafeZoneVolume (infirmerie, sas, ancre fixe).
 */
UCLASS()
class MEG_RECLAMATION_API ULiminalZoneRulesSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Zone")
	bool IsMissionHostile() const;

	UFUNCTION(BlueprintPure, Category = "Zone")
	bool IsLocationInSafeZone(const FVector& Location) const;

	/** Pression de sanite (isolement, obscurite, proximite) autorisee pour cet acteur ? */
	UFUNCTION(BlueprintPure, Category = "Zone")
	bool IsSanityPressureActiveFor(const AActor* Actor) const;

	/** Les entites peuvent-elles apparaitre a cet endroit ? */
	UFUNCTION(BlueprintPure, Category = "Zone")
	bool IsEntitySpawnAllowedAt(const FVector& Location) const;

	/** Taux de restauration de sanite (par seconde) de la zone sure contenant l'acteur, 0 sinon. */
	UFUNCTION(BlueprintPure, Category = "Zone")
	float GetSafeZoneSanityRestoreRateFor(const AActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "Zone")
	int32 GetSafeZoneCount() const { return SafeZones.Num(); }

	/** Helpers statiques : resolvent le subsystem depuis l'acteur. Sans subsystem, comportement legacy (pression active). */
	static bool IsSanityPressureActive(const AActor* Actor);
	static float GetSafeZoneSanityRestoreRate(const AActor* Actor);

	void RegisterSafeZone(ALiminalSafeZoneVolume* Zone);
	void UnregisterSafeZone(ALiminalSafeZoneVolume* Zone);

private:
	const ALiminalSafeZoneVolume* FindSafeZoneContaining(const FVector& Location) const;

	TArray<TWeakObjectPtr<ALiminalSafeZoneVolume>> SafeZones;
};
