#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ScavengerCharacter.generated.h"

class UPhysicsHandleComponent;
class ALootActor;
class UInputAction;
class UInputMappingContext;
struct FInputActionValue;
class UCameraComponent;
class USkeletalMeshComponent;
class ULiminalFootstepComponent;
class ABaseTool;
class ALiminalDoorActor;
class ALiminalHidingSpot;
class ALiminalVentActor;
class ULiminalBodycamComponent;
class ULiminalTetrisInventoryComponent;
class ULiminalProximityVoiceComponent;
class ALiminalTerminalActor;
enum class ELevelBiome : uint8;

/**
 * Personnage Recuperateur : Stamina, poids d'inventaire et Sanite.
 *
 * Server-authoritative : toute mutation de gameplay passe par un Server RPC ;
 * les etats sont repliques au client possesseur uniquement (COND_OwnerOnly),
 * pose les bases de la sante mentale asymetrique client-side (etape 10).
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API AScavengerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	using ALiminalSurvivor = AScavengerCharacter;


public:
	AScavengerCharacter();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDrainStamina(float Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void ServerAddInventoryWeight(float WeightKg);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void ServerRemoveInventoryWeight(float WeightKg);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void ServerDrainSanity(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Sanity")
	void AuthDrainSanity(float Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Authority")
	void ServerRestoreSanity(float Amount);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Infection")
	void ServerSetInfected(bool bInfected);

	UFUNCTION(BlueprintPure, Category = "Scavenger|Infection")
	bool IsInfectedPartygoer() const { return bIsInfectedPartygoer; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Anchor")
	void SetInsideRealityAnchor(bool bInside) { bInsideRealityAnchor = bInside; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Anchor")
	bool IsInsideRealityAnchor() const { return bInsideRealityAnchor; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Tether")
	void SetTetherPartner(AScavengerCharacter* Partner) { TetheredPartner = Partner; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Tether")
	AScavengerCharacter* GetTetherPartner() const { return TetheredPartner.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|State")
	void SetHypnotized(bool bHypno) { bIsHypnotized = bHypno; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|State")
	bool IsHypnotized() const { return bIsHypnotized; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|Health")
	bool IsDead() const { return bIsDead; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Stamina")
	float GetStaminaPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|Weight")
	float GetWeightRatio() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|Sanity")
	float GetSanityPercent() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|Health")
	float GetCurrentHealth() const { return CurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Sanity")
	float GetCurrentSanity() const { return CurrentSanity; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Inventory")
	void AuthSetInventoryWeight(float NewWeightKg);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Stasis")
	void EnterStasis();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Stasis")
	void ExitStasis();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Stasis")
	bool IsInStasis() const { return bIsInStasis; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Carry")
	void InputGrab();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Carry")
	void InputRelease();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Carry")
	void InputThrowLoot();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Survival")
	void TriggerAdrenalineRush(float DurationSeconds = 12.0f);

	UFUNCTION(BlueprintPure, Category = "Scavenger|Survival")
	bool HasAdrenalineRush() const { return bHasAdrenalineRush; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Inventory")
	ULiminalTetrisInventoryComponent* GetTetrisInventory() const { return TetrisInventory; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Audio")
	ULiminalProximityVoiceComponent* GetProximityVoice() const { return ProximityVoice; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Survival")
	void Revive(float HealthPercent = 0.4f, float SanityPercent = 0.5f);

	UFUNCTION(BlueprintPure, Category = "Scavenger|Survival")
	bool IsDowned() const { return bIsDowned; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Survival")
	float GetDownedTimeRemaining() const { return DownedTimeRemaining; }

	// Legacy server-only entry point. Clients request on their own possessed pawn.
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "Scavenger|Survival")
	void ServerRevivePlayer(AScavengerCharacter* Reviver);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Scavenger|Survival")
	void ServerRequestRevive(AScavengerCharacter* Target);

	bool CanReviveTarget(const AScavengerCharacter* Target) const;



	UFUNCTION(BlueprintCallable, Category = "Scavenger|Survival")
	void EnterDownedState();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Carry")
	void DropCarriedLootOnGround();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Feedback")
	float GetDamageFlashIntensity() const { return DamageFlashIntensity; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Sanity")
	FString GetActiveFakeAlert() const { return ActiveFakeAlert; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Carry")
	int32 GetCarriedCredits() const;

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Carry")
	void AddCarriedCredits(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Carry")
	void SetCarriedCredits(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Scavenger|Tools")
	FName GetCurrentToolName() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|Tools")
	float GetToolBatteryCharge() const;

	UFUNCTION(BlueprintPure, Category = "Scavenger|Carry")
	ALootActor* GetHighlightedLoot() const { return HighlightedLoot.Get(); }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Carry")
	ALootActor* GetHeldLoot() const { return HeldLoot.Get(); }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Weight")
	float GetCurrentInventoryWeightKg() const { return CurrentInventoryWeightKg; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Weight")
	float GetMaxCarryWeightKg() const { return MaxCarryWeightKg; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Lamp")
	void ToggleHeadlamp();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Lamp")
	bool IsHeadlampOn() const;

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Camera")
	void ToggleNightVision();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Camera")
	ULiminalBodycamComponent* GetBodycamComponent() const { return BodycamComponent; }

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Tools")
	bool AddOwnedTool(TSubclassOf<ABaseTool> ToolClass);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Tools")
	void RechargeCurrentToolBattery(float PercentAmount);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Health")
	void HealAndRestoreSanity(float HealthAmount, float SanityAmount);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Health")
	void AuthSetHealthAndSanity(float AbsoluteHealth, float AbsoluteSanity);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Tools")
	void DeployEquippedTool();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Camera")
	UStaticMeshComponent* GetFirstPersonToolMesh() const { return FirstPersonToolMesh; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Mesh")
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	void DeliverCarriedLoot();

	virtual bool ShouldTakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) const override;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent,
		AController* EventInstigator, AActor* DamageCauser) override;

	void Die(AController* Killer);

	UFUNCTION(Client, Reliable)
	void ClientOnDamaged(float HealthPercent);

	UFUNCTION(Client, Reliable)
	void ClientOnSanityRestored(float SanityPercent);

	UFUNCTION(Client, Reliable)
	void ClientOnLootCollected(int32 Credits, float WeightKg);

	UFUNCTION(BlueprintCallable, Category = "Scavenger|HUD")
	void ToggleFieldManual();

	void UpdateSanityPressure(float DeltaSeconds);
	void UpdateLocalEffects(float DeltaSeconds);
	void SpawnHallucination();

	// --- Crouch System ---

	/** Accroupissement : mouvement silencieux, passage sous les obstacles. */
	UFUNCTION(BlueprintCallable, Category = "Scavenger|Movement")
	void StartCrouch();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Movement")
	void StopCrouch();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Movement")
	bool GetIsCrouching() const { return bIsCrouching; }

	// --- Lean/Peek System ---

	/** Se pencher a gauche/droite pour regarder autour des coins. */
	UFUNCTION(BlueprintCallable, Category = "Scavenger|Movement")
	void StartLeanLeft();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Movement")
	void StartLeanRight();

	UFUNCTION(BlueprintCallable, Category = "Scavenger|Movement")
	void StopLean();

	UFUNCTION(BlueprintPure, Category = "Scavenger|Movement")
	float GetLeanAmount() const { return CurrentLeanAmount; }

	// --- World Interaction ---

	/** Interaction contextuelle avec les objets du monde (portes, cachettes, vents). */
	UFUNCTION(BlueprintCallable, Category = "Scavenger|Interaction")
	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract();

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Terminal|Network")
	void ServerTerminalPurchaseItem(ALiminalTerminalActor* Terminal, FName ItemId);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Terminal|Network")
	void ServerTerminalSelectBiome(ALiminalTerminalActor* Terminal, ELevelBiome Biome);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Terminal|Network")
	void ServerTerminalLaunchIncursion(ALiminalTerminalActor* Terminal);

	UFUNCTION(BlueprintPure, Category = "Scavenger|Interaction")
	bool IsHiddenInSpot() const { return bIsHiddenInSpot; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Interaction")
	bool IsInVent() const { return bIsInVent; }

	// --- Breathing System ---

	UFUNCTION(BlueprintPure, Category = "Scavenger|Breathing")
	float GetBreathingRate() const { return CurrentBreathingRate; }

	UFUNCTION(BlueprintPure, Category = "Scavenger|Breathing")
	float GetHeartbeatRate() const { return CurrentHeartbeatRate; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void NotifyControllerChanged() override;
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(Server, Reliable)
	void ServerTryGrab();

	UFUNCTION(Server, Reliable)
	void ServerRelease();

	UFUNCTION(Server, Reliable)
	void ServerThrowLoot(float ForceMultiplier = 1.0f);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetSprinting(bool bNewSprinting);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerUseTool();

	UFUNCTION(Server, Reliable)
	void ServerCycleTool();

	UFUNCTION(Server, Reliable)
	void ServerDeployTool();

	void HandleMove(const FInputActionValue& Value);
	void HandleLook(const FInputActionValue& Value);
	void HandleSprintStarted();
	void HandleSprintStopped();
	void HandleGrabPressed();
	void HandleUseToolPressed();
	void HandleCycleToolPressed();
	void HandleCrouchStarted();
	void HandleCrouchStopped();
	void HandleLeanLeft(const FInputActionValue& Value);
	void HandleLeanRight(const FInputActionValue& Value);
	void HandleLeanStop();
	void HandleInteractPressed();
	void HandleDeployToolPressed();
	void HandleToggleHeadlamp();

	void FallbackMoveForward(float Val);
	void FallbackMoveBackward(float Val);
	void FallbackMoveLeft(float Val);
	void FallbackMoveRight(float Val);
	void FallbackTurn(float Val);
	void FallbackLookUp(float Val);

	UFUNCTION()
	void OnRep_CurrentHealth();

	UFUNCTION()
	void OnRep_IsDead();

	UFUNCTION()
	void OnRep_CurrentToolIndex();

	UFUNCTION()
	void OnRep_CurrentStamina();

	UFUNCTION()
	void OnRep_CurrentInventoryWeightKg();

	UFUNCTION()
	void OnRep_CurrentSanity();

	UFUNCTION()
	void OnRep_IsInStasis();

	UFUNCTION()
	void OnRep_CarriedCredits();

	UFUNCTION()
	void OnRep_IsSprinting();

	void UpdateStamina(float DeltaSeconds);
	void UpdateMovementFromWeight();
	void UpdateCarriedObjectTarget();
	bool TraceForGrabbable(ALootActor*& OutLoot, UPrimitiveComponent*& OutPrimitive) const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Health", meta = (ClampMin = "1.0"))
	float MaxHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Health", ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Health", ReplicatedUsing = OnRep_IsDead)
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Health", ReplicatedUsing = OnRep_IsDowned)
	bool bIsDowned = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Health", Replicated)
	float DownedTimeRemaining = 45.0f;

	UFUNCTION()
	void OnRep_IsDowned();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Stamina", meta = (ClampMin = "0.0"))
	float MaxStamina = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Stamina", ReplicatedUsing = OnRep_CurrentStamina)
	float CurrentStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenPerSecond = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Stamina", meta = (ClampMin = "0.0"))
	float StaminaRegenDelaySeconds = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Weight", meta = (ClampMin = "0.0"))
	float MaxCarryWeightKg = 60.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Weight", ReplicatedUsing = OnRep_CurrentInventoryWeightKg)
	float CurrentInventoryWeightKg = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Weight", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float WalkSpeedAtMaxWeightFactor = 0.35f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0"))
	float MaxSanity = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Carry", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|View", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> FirstPersonMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|View", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USpotLightComponent> HeadlampLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULiminalFootstepComponent> FootstepAudio;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class ULiminalSanityPostProcessComponent> SanityPostProcess;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Camera", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULiminalBodycamComponent> BodycamComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Inventory", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULiminalTetrisInventoryComponent> TetrisInventory;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Audio", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULiminalProximityVoiceComponent> ProximityVoice;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Carry", meta = (ClampMin = "50.0"))
	float GrabRange = 250.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Carry", meta = (ClampMin = "40.0"))
	float CarryDistance = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Carry", ReplicatedUsing = OnRep_CarriedCredits)
	int32 CarriedCredits = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Sanity", ReplicatedUsing = OnRep_CurrentSanity)
	float CurrentSanity = 100.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Infection", ReplicatedUsing = OnRep_IsInfectedPartygoer)
	bool bIsInfectedPartygoer = false;

	UFUNCTION()
	void OnRep_IsInfectedPartygoer();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputMappingContext> ScavengerMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionLook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionGrab;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionSprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionUseTool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionCycleTool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionDeployTool;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionToggleHeadlamp;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionInteract;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionDropLoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionThrowLoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionNightVision;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Input")
	TSoftObjectPtr<UInputAction> InputActionFieldManual;

	bool bEnhancedInputBound = false;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scavenger|Camera")
	TObjectPtr<UStaticMeshComponent> FirstPersonToolMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Camera")
	TSoftObjectPtr<UStaticMesh> DefaultToolMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Tools")
	TArray<TSubclassOf<ABaseTool>> DefaultToolClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sprint", meta = (ClampMin = "1.0"))
	float SprintSpeedMultiplier = 1.6f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sprint", meta = (ClampMin = "0.0"))
	float SprintStaminaDrainPerSecond = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0"))
	float SanityIsolationDrainPerSecond = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0"))
	float SanityDarknessDrainPerSecond = 0.75f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0"))
	float SanityHoundProximityDrainPerSecond = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0"))
	float HoundProximityRange = 1200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0"))
	float AllySupportRange = 1500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HallucinationSanityThresholdPercent = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "1.0"))
	float HallucinationMinIntervalSeconds = 6.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scavenger|Sanity", meta = (ClampMin = "2.0"))
	float HallucinationMaxIntervalSeconds = 12.0f;

private:
	TWeakObjectPtr<AScavengerCharacter> PendingReviveTarget;
	float ReviveElapsedSeconds = 0.0f;
	double LastDoorInteractionTime = -1.0;
	void UpdateRevive(float DeltaSeconds);
	UPROPERTY(ReplicatedUsing = OnRep_IsSprinting)
	bool bIsSprinting = false;

	void EmitFootstepNoise(float DeltaSeconds);
	void UpdateDebugHud();
	void UpdateLootGaze();
	ABaseTool* GetCurrentTool() const;
	void EquipLoadout();

	// --- Crouch/Lean internals ---
	void UpdateCrouchState(float DeltaSeconds);
	void UpdateLeanState(float DeltaSeconds);
	void UpdateBreathing(float DeltaSeconds);
	bool TraceForInteractable(AActor*& OutActor) const;

	float BaseWalkSpeed = 450.0f;
	float TimeSinceStaminaDrain = 0.0f;
	float FootstepNoiseTimer = 0.0f;
	float TimeToNextHallucination = 8.0f;
	float HeadBobPhase = 0.0f;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentToolIndex)
	int32 CurrentToolIndex = 0;

	UPROPERTY(Replicated)
	TArray<TObjectPtr<ABaseTool>> OwnedTools;

	TWeakObjectPtr<ALootActor> HeldLoot;
	TWeakObjectPtr<ALootActor> HighlightedLoot;
	TWeakObjectPtr<AScavengerCharacter> TetheredPartner;
	bool bInsideRealityAnchor = false;
	bool bIsHypnotized = false;
	bool bHasAdrenalineRush = false;
	float AdrenalineRushTimer = 0.0f;
	float DamageFlashIntensity = 0.0f;
	FString ActiveFakeAlert;
	float FakeAlertTimer = 0.0f;
	float GhostFootstepTimer = 0.0f;

	// --- Crouch ---
	bool bIsCrouching = false;
	bool bWantsToCrouch = false;
	float CrouchAlpha = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Crouch", meta = (ClampMin = "0.1", ClampMax = "1.0"))
	float CrouchSpeedMultiplier = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Crouch", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float CrouchNoiseReduction = 0.7f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Crouch", meta = (ClampMin = "20.0"))
	float CrouchedCapsuleHalfHeight = 44.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Crouch", meta = (ClampMin = "0.1"))
	float CrouchTransitionSpeed = 8.0f;

	float StandingCapsuleHalfHeight = 0.0f;
	float StandingCameraZ = 60.0f;
	float CrouchedCameraZ = 20.0f;

	// --- Lean/Peek ---
	float TargetLeanAmount = 0.0f;
	float CurrentLeanAmount = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Lean", meta = (ClampMin = "5.0", ClampMax = "60.0"))
	float MaxLeanOffset = 35.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Lean", meta = (ClampMin = "1.0", ClampMax = "30.0"))
	float MaxLeanAngle = 15.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Lean", meta = (ClampMin = "1.0"))
	float LeanSpeed = 10.0f;

	// --- World Interaction ---
	UPROPERTY(Replicated)
	bool bIsHiddenInSpot = false;
	UPROPERTY(Replicated)
	bool bIsInVent = false;
	UPROPERTY(ReplicatedUsing = OnRep_IsInStasis)
	bool bIsInStasis = false;
	TWeakObjectPtr<ALiminalHidingSpot> CurrentHidingSpot;
	TWeakObjectPtr<ALiminalVentActor> CurrentVent;

	// --- Breathing ---
	float CurrentBreathingRate = 1.0f;
	float CurrentHeartbeatRate = 1.0f;
	float BreathingTimer = 0.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Noise", meta = (ClampMin = "0.0"))
	float FootstepLoudness = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Noise", meta = (ClampMin = "0.05"))
	float FootstepNoiseIntervalSeconds = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Scavenger|Noise", meta = (ClampMin = "0.0"))
	float MinimumNoiseSpeed = 50.0f;
};
