#include "Data/QuotaManager.h"

void UQuotaManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (State.Required <= 0)
	{
		State.Required = FQuotaLogic::ClampTarget(DefaultQuotaTarget);
	}
}

void UQuotaManager::SetQuotaTarget(int32 NewRequiredValue)
{
	State.Required = FQuotaLogic::ClampTarget(NewRequiredValue);
	BroadcastQuotaUpdate();
}

void UQuotaManager::AddDeliveredValue(int32 ValueToAdd)
{
	FQuotaLogic::AddDelivered(State, ValueToAdd);
	BroadcastQuotaUpdate();
}

void UQuotaManager::ApplyQuotaFailure()
{
	FQuotaLogic::ApplyFailure(State);
	BroadcastQuotaUpdate();
}

bool UQuotaManager::IsQuotaMet() const
{
	return FQuotaLogic::IsMet(State);
}

int32 UQuotaManager::GetDeliveredValue() const
{
	return State.Delivered;
}

int32 UQuotaManager::GetRequiredValue() const
{
	return State.Required;
}

int32 UQuotaManager::GetOutstandingDebt() const
{
	return State.OutstandingDebt;
}

int32 UQuotaManager::GetTotalDue() const
{
	return FQuotaLogic::TotalDue(State);
}

void UQuotaManager::BroadcastQuotaUpdate()
{
	OnQuotaUpdated.Broadcast(State.Delivered, FQuotaLogic::TotalDue(State));
}
