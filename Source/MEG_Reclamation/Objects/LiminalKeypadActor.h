#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiminalKeypadActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UTextRenderComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeypadUnlockedSignature, ALiminalKeypadActor*, Keypad);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKeypadCodeFailedSignature, ALiminalKeypadActor*, Keypad);

/**
 * Clavier de securite diegetique a 4 chiffres (style Escape the Backrooms).
 * Permet de deverrouiller des portes blindees, des sas de quarantaine ou des armoires de loot.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalKeypadActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalKeypadActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Keypad")
	bool EnterDigit(int32 Digit);

	UFUNCTION(BlueprintCallable, Category = "Keypad")
	void ClearInput();

	UFUNCTION(BlueprintCallable, Category = "Keypad")
	bool SubmitCode();

	UFUNCTION(BlueprintCallable, Category = "Keypad")
	void SetTargetCode(const FString& InCode) { TargetCode = InCode; }

	UFUNCTION(BlueprintPure, Category = "Keypad")
	FString GetTargetCode() const { return TargetCode; }

	UFUNCTION(BlueprintPure, Category = "Keypad")
	FString GetCurrentInput() const { return CurrentInput; }

	UFUNCTION(BlueprintPure, Category = "Keypad")
	bool IsUnlocked() const { return bIsUnlocked; }

	UFUNCTION(BlueprintPure, Category = "Keypad")
	FString GetDisplayString() const;

	UPROPERTY(BlueprintAssignable, Category = "Keypad|Events")
	FOnKeypadUnlockedSignature OnKeypadUnlocked;

	UPROPERTY(BlueprintAssignable, Category = "Keypad|Events")
	FOnKeypadCodeFailedSignature OnKeypadCodeFailed;

	// Acteur cible optionnel a activer / deverrouiller (ex: porte, sas)
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Keypad")
	TWeakObjectPtr<AActor> LinkedTargetActor;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> KeypadMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPointLightComponent> StatusLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UTextRenderComponent> DisplayText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Keypad")
	FString TargetCode = TEXT("4821");

	UPROPERTY(ReplicatedUsing = OnRep_CurrentInput, VisibleAnywhere, BlueprintReadOnly, Category = "Keypad")
	FString CurrentInput;

	UPROPERTY(ReplicatedUsing = OnRep_IsUnlocked, VisibleAnywhere, BlueprintReadOnly, Category = "Keypad")
	bool bIsUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Keypad")
	int32 MaxDigits = 4;

	UFUNCTION()
	void OnRep_CurrentInput();

	UFUNCTION()
	void OnRep_IsUnlocked();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	float ErrorFlashTimer = 0.0f;
	void UpdateVisualState();
};
