#pragma once

#include "CoreMinimal.h"
#include "LiminalSanityTypes.generated.h"

UENUM(BlueprintType)
enum class ESanityTier : uint8
{
	Stable UMETA(DisplayName = "Stable (>70%)"),
	Uneasy UMETA(DisplayName = "Inquiet (50-70%)"),
	Paranoid UMETA(DisplayName = "Paranoïaque (25-50%)"),
	Psychotic UMETA(DisplayName = "Psychotique (<25%)")
};

UENUM(BlueprintType)
enum class EHallucinationType : uint8
{
	ShadowSilhouette,
	FakeDoor,
	FakeLoot,
	PhantomAuditory
};

namespace LiminalSanity
{
	FORCEINLINE ESanityTier GetTierFromPercent(float SanityPercent)
	{
		if (SanityPercent >= 0.70f)
		{
			return ESanityTier::Stable;
		}
		if (SanityPercent >= 0.50f)
		{
			return ESanityTier::Uneasy;
		}
		if (SanityPercent >= 0.25f)
		{
			return ESanityTier::Paranoid;
		}
		return ESanityTier::Psychotic;
	}
}
