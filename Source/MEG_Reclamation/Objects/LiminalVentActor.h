#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalVentActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USplineComponent;
class AScavengerCharacter;

UENUM(BlueprintType)
enum class EVentState : uint8
{
	Sealed,
	Open,
	Occupied
};

/**
 * Conduit de ventilation traversable.
 * Le joueur peut ouvrir la grille, ramper a l'interieur (camera contrainte),
 * et ressortir de l'autre cote. Les entites ne peuvent pas y entrer
 * (sauf certaines petites entites comme les Clumps).
 *
 * Le crawl genere du bruit metallique qui attire les Hounds.
 * Server-authoritative : l'etat et l'occupation sont repliques.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalVentActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalVentActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Le joueur entre dans le conduit depuis le point d'entree le plus proche. */
	UFUNCTION(BlueprintCallable, Category = "Vent")
	void TryEnter(AScavengerCharacter* Scavenger);

	/** Le joueur sort du conduit a l'autre extremite. */
	UFUNCTION(BlueprintCallable, Category = "Vent")
	void ExitVent();

	UFUNCTION(BlueprintPure, Category = "Vent")
	EVentState GetVentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "Vent")
	bool IsOccupied() const { return CurrentState == EVentState::Occupied; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_VentState();

	void UpdateCrawlProgress(float DeltaSeconds);
	void EmitCrawlNoise();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vent", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> VentRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vent", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> VentMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vent", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> GrilleMesh;

	/** Point d'entree A (un cote du conduit). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vent", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> EntryZoneA;

	/** Point d'entree B (l'autre cote du conduit). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vent", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> EntryZoneB;

	/** Spline definissant le chemin de crawl a travers le conduit. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Vent", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USplineComponent> CrawlPath;

	/** Vitesse de crawl (unites/seconde). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent|Motion", meta = (ClampMin = "50.0"))
	float CrawlSpeed = 150.0f;

	/** Intervalle entre les bruits metalliques de crawl (secondes). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent|Audio", meta = (ClampMin = "0.1"))
	float CrawlNoiseInterval = 0.8f;

	/** Loudness du bruit de crawl pour l'IA. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent|Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float CrawlNoiseLoudness = 0.5f;

	/** Les petites entites (Clumps) peuvent entrer dans le conduit. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vent|AI")
	bool bSmallEntitiesCanEnter = true;

private:
	UPROPERTY(ReplicatedUsing = OnRep_VentState)
	EVentState CurrentState = EVentState::Sealed;

	TWeakObjectPtr<AScavengerCharacter> OccupantCharacter;
	float CrawlProgress = 0.0f; // 0.0 = entree, 1.0 = sortie
	float CrawlNoiseTimer = 0.0f;
	bool bCrawlForward = true; // Direction du crawl (A->B ou B->A)

	FVector SavedPlayerLocation;
	FRotator SavedPlayerRotation;
};
