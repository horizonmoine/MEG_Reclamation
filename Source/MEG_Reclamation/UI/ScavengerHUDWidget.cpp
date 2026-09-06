#include "UI/ScavengerHUDWidget.h"

#include "Player/ScavengerCharacter.h"
#include "Tools/BaseTool.h"

AScavengerCharacter* UScavengerHUDWidget::GetOwningScavenger() const
{
	return Cast<AScavengerCharacter>(GetOwningPlayerPawn());
}

float UScavengerHUDWidget::GetHealthPercent() const
{
	if (const AScavengerCharacter* Scavenger = GetOwningScavenger())
	{
		return FMath::Clamp(Scavenger->GetHealthPercent(), 0.0f, 1.0f);
	}
	return 1.0f;
}

float UScavengerHUDWidget::GetSanityPercent() const
{
	if (const AScavengerCharacter* Scavenger = GetOwningScavenger())
	{
		return FMath::Clamp(Scavenger->GetSanityPercent(), 0.0f, 1.0f);
	}
	return 1.0f;
}

ESanityTier UScavengerHUDWidget::GetSanityTier() const
{
	return LiminalSanity::GetTierFromPercent(GetSanityPercent());
}

FText UScavengerHUDWidget::GetCorruptedAlertText() const
{
	const ESanityTier Tier = GetSanityTier();
	switch (Tier)
	{
	case ESanityTier::Psychotic:
	{
		static const TCHAR* PsychoticAlerts[] = {
			TEXT("ERREUR SYNCHRO // IL EST DERRIÈRE TOI"),
			TEXT("SIGNAL CRITIQUE // NE REGARDE PAS LA PORTE"),
			TEXT("ALERTE : COÉQUIPIER DÉCÉDÉ (FAUX SIGNAL)"),
			TEXT("SURCHARGE NEURONALE // FUYEZ MAINTENANT")
		};
		const int32 Index = FMath::RandRange(0, 3);
		return FText::FromString(PsychoticAlerts[Index]);
	}
	case ESanityTier::Paranoid:
	{
		static const TCHAR* ParanoidAlerts[] = {
			TEXT("ATTENTION : ANOMALIE SPATIALE PROCHE"),
			TEXT("MICRO-RUPTURE DE RÉALITÉ DÉTECTÉE"),
			TEXT("ÉCHO VOCAL NON IDENTIFIÉ DANS LE SECTEUR")
		};
		const int32 Index = FMath::RandRange(0, 2);
		return FText::FromString(ParanoidAlerts[Index]);
	}
	case ESanityTier::Uneasy:
		return FText::FromString(TEXT("AVERTISSEMENT : STABILITÉ PSYCHIQUE EN BAISSE"));
	case ESanityTier::Stable:
	default:
		return FText::GetEmpty();
	}
}

bool UScavengerHUDWidget::HasActiveHallucinationAlert() const
{
	const ESanityTier Tier = GetSanityTier();
	return (Tier == ESanityTier::Paranoid || Tier == ESanityTier::Psychotic);
}

float UScavengerHUDWidget::GetStaminaPercent() const
{
	if (const AScavengerCharacter* Scavenger = GetOwningScavenger())
	{
		return FMath::Clamp(Scavenger->GetStaminaPercent(), 0.0f, 1.0f);
	}
	return 1.0f;
}

float UScavengerHUDWidget::GetWeightRatio() const
{
	if (const AScavengerCharacter* Scavenger = GetOwningScavenger())
	{
		return FMath::Clamp(Scavenger->GetWeightRatio(), 0.0f, 2.0f);
	}
	return 0.0f;
}

int32 UScavengerHUDWidget::GetCarriedCredits() const
{
	if (const AScavengerCharacter* Scavenger = GetOwningScavenger())
	{
		return Scavenger->GetCarriedCredits();
	}
	return 0;
}

FName UScavengerHUDWidget::GetCurrentToolName() const
{
	if (const AScavengerCharacter* Scavenger = GetOwningScavenger())
	{
		return Scavenger->GetCurrentToolName();
	}
	return NAME_None;
}

float UScavengerHUDWidget::GetToolBatteryCharge() const
{
	return 1.0f;
}

bool UScavengerHUDWidget::IsCriticallyLowSanity() const
{
	return GetSanityPercent() < 0.3f;
}

bool UScavengerHUDWidget::IsCriticallyLowHealth() const
{
	return GetHealthPercent() < 0.25f;
}
