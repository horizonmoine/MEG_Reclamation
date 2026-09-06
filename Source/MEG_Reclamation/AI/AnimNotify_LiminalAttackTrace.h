#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_LiminalAttackTrace.generated.h"

/**
 * AnimNotify for liminal entity physical melee attack sweeps.
 * Executed server-side at the strike apex of attack animations/montages.
 * Performs a spherical sweep (SweepMultiByChannel) along ECC_Pawn from SocketName
 * (falling back to actor forward vector if unrigged or socket missing),
 * applying point damage to hit pawns and triggering impact sound and AI noise.
 */
UCLASS(meta = (DisplayName = "Liminal Entity Attack Trace"))
class MEG_RECLAMATION_API UAnimNotify_LiminalAttackTrace : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_LiminalAttackTrace();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	/** Bone/socket on the Skeletal Mesh from which the attack trace originates */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	FName SocketName = FName(TEXT("AttackSocket"));

	/** Sweep sphere radius in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (ClampMin = "5.0", ClampMax = "200.0"))
	float TraceRadius = 45.0f;

	/** Forward projection distance in centimeters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (ClampMin = "10.0", ClampMax = "400.0"))
	float TraceDistance = 90.0f;

	/** If true, override entity base damage with CustomDamage */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	bool bOverrideDamage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace", meta = (EditCondition = "bOverrideDamage", ClampMin = "0.0"))
	float CustomDamage = 35.0f;

	/** Optional custom DamageType class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace")
	TSubclassOf<UDamageType> DamageTypeClass;

	/** Draw debug sphere in non-shipping builds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attack Trace|Debug")
	bool bDrawDebugTrace = false;
};
