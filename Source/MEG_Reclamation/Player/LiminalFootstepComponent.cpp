#include "Player/LiminalFootstepComponent.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"

ULiminalFootstepComponent::ULiminalFootstepComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<USoundBase> DefaultFootstep(
		TEXT("/Game/Audio/S_Footstep_Carpet_01.S_Footstep_Carpet_01"));
	if (DefaultFootstep.Succeeded())
	{
		FootstepSound = DefaultFootstep.Object;
	}
}

void ULiminalFootstepComponent::PlayFootstep(float VolumeScale)
{
	const AActor* Owner = GetOwner();
	UWorld* World = GetWorld();
	if (!Owner || !World || !FootstepSound)
	{
		return;
	}

	const float Pitch = 1.6f + FMath::FRandRange(-PitchJitter, PitchJitter);
	UGameplayStatics::PlaySoundAtLocation(World, FootstepSound, Owner->GetActorLocation(),
		VolumeScale * 0.35f, Pitch);
}
