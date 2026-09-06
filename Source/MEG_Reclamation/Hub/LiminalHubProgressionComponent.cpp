#include "Hub/LiminalHubProgressionComponent.h"
#include "Data/LiminalGameInstance.h"
#include "Kismet/GameplayStatics.h"

ULiminalHubProgressionComponent::ULiminalHubProgressionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULiminalHubProgressionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		RefreshHubTier(GI->GetTotalCredits(), GI->GetActiveQuotaCycle());
	}
}

void ULiminalHubProgressionComponent::RefreshHubTier(int32 TotalBankCredits, int32 CompletedCycles)
{
	EHubTier NewTier = EHubTier::MakeshiftCamp;

	if (TotalBankCredits >= Tier3CreditsThreshold || CompletedCycles >= 4)
	{
		NewTier = EHubTier::ScientificLab;
	}
	else if (TotalBankCredits >= Tier2CreditsThreshold || CompletedCycles >= 2)
	{
		NewTier = EHubTier::ReinforcedOutpost;
	}

	if (NewTier != CurrentTier)
	{
		CurrentTier = NewTier;
		OnHubTierChanged.Broadcast(CurrentTier);
		UE_LOG(LogTemp, Log, TEXT("[HubProgression] Base Alpha upgraded to %s (Credits: %d, Cycle: %d)"),
			*GetTierDisplayName().ToString(), TotalBankCredits, CompletedCycles);
	}
}

FText ULiminalHubProgressionComponent::GetTierDisplayName() const
{
	switch (CurrentTier)
	{
	case EHubTier::ScientificLab:
		return FText::FromString(TEXT("Palier 3 : Laboratoire de Confinement M.E.G."));
	case EHubTier::ReinforcedOutpost:
		return FText::FromString(TEXT("Palier 2 : Avant-poste Fortifié"));
	case EHubTier::MakeshiftCamp:
	default:
		return FText::FromString(TEXT("Palier 1 : Campement de Fortune"));
	}
}
