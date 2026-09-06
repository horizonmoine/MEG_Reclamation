#pragma once

#include "CoreMinimal.h"
#include "Tools/BaseTool.h"
#include "WalkieTalkieTool.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWalkieChannelChanged, int32, NewChannel, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWalkieTransmitStateChanged, bool, bTransmitting, int32, Channel);

/**
 * Talkie-Walkie M.E.G. standard (Frequences UHF 400-470MHz).
 * Permet la communication vocale/radio a longue distance entre equipiers.
 * Supporte : 8 canaux selectionnables, bruit de squelch, perte de signal selon distance/epaisseur des murs,
 * et risque d'attirer les entites acoustiques (Hounds) lors de l'emission.
 */
UCLASS()
class MEG_RECLAMATION_API AWalkieTalkieTool : public ABaseTool
{
	GENERATED_BODY()

public:
	AWalkieTalkieTool();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual bool Activate() override;
	virtual void Deactivate() override;

	/** Change le canal radio actif (1 a 8) */
	UFUNCTION(BlueprintCallable, Category = "Tool|Walkie")
	void SetChannel(int32 NewChannel);

	/** Passe au canal suivant */
	UFUNCTION(BlueprintCallable, Category = "Tool|Walkie")
	void NextChannel();

	UFUNCTION(BlueprintPure, Category = "Tool|Walkie")
	int32 GetChannel() const { return CurrentChannel; }

	UFUNCTION(BlueprintPure, Category = "Tool|Walkie")
	bool IsTransmitting() const { return bIsActive; }

	/** Calcule la clarte du signal radio recu (0.0 = friture totale, 1.0 = reception cristalline) */
	UFUNCTION(BlueprintCallable, Category = "Tool|Walkie")
	float CalculateSignalClarity(const FVector& TransmitterLocation) const;

	UPROPERTY(BlueprintAssignable, Category = "Tool|Walkie")
	FOnWalkieChannelChanged OnChannelChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tool|Walkie")
	FOnWalkieTransmitStateChanged OnTransmitStateChanged;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnRep_CurrentChannel();

	virtual void OnRep_IsActive() override;

private:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentChannel, EditAnywhere, BlueprintReadOnly, Category = "Tool|Walkie", meta = (ClampMin = "1", ClampMax = "8", AllowPrivateAccess = "true"))
	int32 CurrentChannel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool|Walkie", meta = (AllowPrivateAccess = "true"))
	float MaxEffectiveRangeMeters = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tool|Walkie", meta = (AllowPrivateAccess = "true"))
	float AcousticNoiseRadiusOnTransmit = 600.0f;
};
