#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LiminalPortalComponent.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UMaterialInstanceDynamic;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPortalTraversed, AActor*, TraversedActor, ULiminalPortalComponent*, SourcePortal);

/**
 * Composant de portail non-euclidien pour les espaces impossibles et les boucles infinies.
 * Permet un rendu seamless via SceneCapture2D et une teleportation instantanee
 * avec conservation de momentum physique et de rotation de vue.
 */
UCLASS(ClassGroup = (ProcGen), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalPortalComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	ULiminalPortalComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Associe ce portail a son jumeau de sortie */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Portal")
	void LinkTargetPortal(ULiminalPortalComponent* InTargetPortal);

	/** Assigne le mesh d'affichage visuel du portail (plan/quad) */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Portal")
	void SetDisplayMesh(UStaticMeshComponent* InMesh);

	/** Teleporte un acteur traversant le portail vers la sortie */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Portal")
	bool TeleportActor(AActor* InActor);

	UFUNCTION(BlueprintPure, Category = "Liminal|Portal")
	ULiminalPortalComponent* GetTargetPortal() const { return TargetPortal; }

	UFUNCTION(BlueprintPure, Category = "Liminal|Portal")
	bool IsLinked() const { return TargetPortal != nullptr; }

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Portal")
	FOnPortalTraversed OnPortalTraversed;

protected:
	void UpdateCaptureCamera();
	bool CheckActorCrossing(AActor* InActor, const FVector& PreviousLocation, const FVector& CurrentLocation);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Portal|Render")
	int32 RenderTargetWidth = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Portal|Render")
	int32 RenderTargetHeight = 1024;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Portal|Render")
	bool bEnableSceneCapture = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Liminal|Portal|Teleport")
	float TeleportThresholdDistance = 150.0f;

	UPROPERTY(Transient)
	TObjectPtr<ULiminalPortalComponent> TargetPortal;

	UPROPERTY(Transient)
	TObjectPtr<USceneCaptureComponent2D> PortalCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> PortalRenderTarget;

	UPROPERTY(Transient)
	TObjectPtr<UStaticMeshComponent> DisplayMeshComponent;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> PortalMaterialInstance;

	TMap<TWeakObjectPtr<AActor>, FVector> TrackedActorPositions;
};

/**
 * Acteur portail autonome pour les anomalies spatiales non-euclidiennes.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalPortalActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalPortalActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<ULiminalPortalComponent> PortalComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UStaticMeshComponent> DisplayMesh;
};

