#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_LiminalFootstep.generated.h"

class USoundBase;

/**
 * AnimNotify for entity footstep events.
 * Traces downwards to find ground surface and plays spatialized footstep audio
 * while reporting noise event to AI perception.
 */
UCLASS(meta = (DisplayName = "Liminal Entity Footstep"))
class MEG_RECLAMATION_API UAnimNotify_LiminalFootstep : public UAnimNotify
{
	GENERATED_BODY()

public:
	UAnimNotify_LiminalFootstep();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference) override;

	virtual FString GetNotifyName_Implementation() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	FName FootSocketName = FName(TEXT("foot_r"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	float Loudness = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
	TObjectPtr<USoundBase> OverrideFootstepSound;
};
