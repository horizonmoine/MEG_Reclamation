#include "Tools/TetherTool.h"
#include "Player/ScavengerCharacter.h"
#include "Engine/World.h"
#include "EngineUtils.h"

ATetherTool::ATetherTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	ActiveBatteryDrainPerSecond = 0.0f;
}

bool ATetherTool::CanActivate() const
{
	return Super::CanActivate();
}

bool ATetherTool::Activate()
{
	if (IsTethered())
	{
		DisconnectTether();
		return true;
	}

	return ConnectToNearestPartner();
}

void ATetherTool::Deactivate()
{
	Super::Deactivate();
	DisconnectTether();
}

bool ATetherTool::ConnectToNearestPartner()
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor)
	{
		return false;
	}

	AScavengerCharacter* LocalScavenger = Cast<AScavengerCharacter>(OwnerActor);
	AScavengerCharacter* BestPartner = nullptr;
	float ClosestDist = ConnectRange;

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Candidate = *It;
		if (Candidate && Candidate != LocalScavenger && !Candidate->IsDead())
		{
			const float Dist = FVector::Dist(OwnerActor->GetActorLocation(), Candidate->GetActorLocation());
			if (Dist < ClosestDist)
			{
				ClosestDist = Dist;
				BestPartner = Candidate;
			}
		}
	}

	if (BestPartner)
	{
		ConnectedPartner = BestPartner;
		bIsActive = true;

		if (LocalScavenger)
		{
			LocalScavenger->SetTetherPartner(BestPartner);
		}
		BestPartner->SetTetherPartner(LocalScavenger);

		OnTetherConnectionChanged.Broadcast(true, BestPartner);
		UE_LOG(LogTemp, Log, TEXT("[Tether] Connected to partner '%s'"), *BestPartner->GetName());
		return true;
	}

	return false;
}

void ATetherTool::DisconnectTether()
{
	if (ConnectedPartner.IsValid())
	{
		AScavengerCharacter* OldPartner = ConnectedPartner.Get();
		ConnectedPartner = nullptr;
		bIsActive = false;

		if (AScavengerCharacter* LocalScavenger = Cast<AScavengerCharacter>(GetOwner()))
		{
			LocalScavenger->SetTetherPartner(nullptr);
		}
		if (OldPartner)
		{
			OldPartner->SetTetherPartner(nullptr);
		}

		OnTetherConnectionChanged.Broadcast(false, OldPartner);
		UE_LOG(LogTemp, Log, TEXT("[Tether] Disconnected."));
	}
}

float ATetherTool::GetCurrentTetherDistance() const
{
	if (ConnectedPartner.IsValid() && GetOwner())
	{
		return FVector::Dist(GetOwner()->GetActorLocation(), ConnectedPartner->GetActorLocation());
	}
	return 0.0f;
}

void ATetherTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (IsTethered())
	{
		const float Dist = GetCurrentTetherDistance();
		// If stretched too far past the maximum length, snap the connection
		if (Dist > MaxTetherLength * 1.35f)
		{
			DisconnectTether();
		}
	}
}
