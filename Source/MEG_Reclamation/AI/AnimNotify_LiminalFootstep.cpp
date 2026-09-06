#include "AI/AnimNotify_LiminalFootstep.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"
#include "UObject/ConstructorHelpers.h"

UAnimNotify_LiminalFootstep::UAnimNotify_LiminalFootstep()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 180, 255, 255);
#endif
}

void UAnimNotify_LiminalFootstep::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (!World)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	const FVector TraceStart = (FootSocketName != NAME_None && MeshComp->DoesSocketExist(FootSocketName))
		? MeshComp->GetSocketLocation(FootSocketName)
		: MeshComp->GetComponentLocation();

	const FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, 60.0f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LiminalFootstepTrace), false, Owner);
	QueryParams.bReturnPhysicalMaterial = true;

	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
	const FVector SoundLocation = bHit ? HitResult.ImpactPoint : TraceStart;

	USoundBase* SoundToPlay = OverrideFootstepSound;
	if (!SoundToPlay)
	{
		static ConstructorHelpers::FObjectFinderOptional<USoundBase> DefaultFootstep(
			TEXT("/Game/Audio/S_Footstep_Carpet_01.S_Footstep_Carpet_01"));
		if (DefaultFootstep.Succeeded())
		{
			SoundToPlay = DefaultFootstep.Get();
		}
	}

	if (SoundToPlay)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SoundToPlay, SoundLocation, Loudness,
			FMath::FRandRange(0.9f, 1.1f));
	}

	if (Owner)
	{
		UAISense_Hearing::ReportNoiseEvent(World, SoundLocation, Loudness, Owner);
	}
}

FString UAnimNotify_LiminalFootstep::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Liminal Footstep [%s]"), *FootSocketName.ToString());
}
