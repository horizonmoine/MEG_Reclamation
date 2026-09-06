#include "Tools/AdrenalineInjectorTool.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"
#include "Perception/AISense_Hearing.h"
#include "UObject/ConstructorHelpers.h"

AAdrenalineInjectorTool::AAdrenalineInjectorTool()
{
	MaxBatteryCharge = 100.0f;
	BatteryCharge = 100.0f;
	RemainingDoses = 2;
	MaxDoses = 2;
	InjectRange = 220.0f;
	ActiveBatteryDrainPerSecond = 0.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> InjectorFinder(
		TEXT("/Game/Meshes/Tools/SM_Adrenaline_Injector.SM_Adrenaline_Injector"));
	if (InjectorFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(InjectorFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f));
	}
}

void AAdrenalineInjectorTool::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAdrenalineInjectorTool, RemainingDoses);
}

void AAdrenalineInjectorTool::SetRemainingDoses(int32 InDoses)
{
	RemainingDoses = FMath::Clamp(InDoses, 0, MaxDoses);
	BatteryCharge = MaxDoses > 0 ? ((float)RemainingDoses / MaxDoses) * MaxBatteryCharge : 0.0f;
}

void AAdrenalineInjectorTool::OnRep_RemainingDoses()
{
	BatteryCharge = MaxDoses > 0 ? ((float)RemainingDoses / MaxDoses) * MaxBatteryCharge : 0.0f;
}

bool AAdrenalineInjectorTool::Activate()
{
	if (!Super::Activate())
	{
		return false;
	}

	if (RemainingDoses <= 0)
	{
		Deactivate();
		return false;
	}

	if (!HasAuthority())
	{
		Deactivate();
		return true;
	}

	AScavengerCharacter* OwnerChar = Cast<AScavengerCharacter>(GetOwner());
	AScavengerCharacter* TargetTeammate = TraceTeammateTarget();

	if (TargetTeammate && TargetTeammate != OwnerChar)
	{
		if (TargetTeammate->IsDead())
		{
			// Reanimation d'urgence defibrillatoire / adrenaline
			TargetTeammate->Revive(0.45f, 0.5f);
		}
		else
		{
			// Boost d'adrenaline sur l'equipier
			TargetTeammate->TriggerAdrenalineRush(14.0f);
		}

		UAISense_Hearing::ReportNoiseEvent(GetWorld(), TargetTeammate->GetActorLocation(), 0.5f, this);
	}
	else if (OwnerChar)
	{
		// Auto-injection de combat
		OwnerChar->TriggerAdrenalineRush(12.0f);
		UAISense_Hearing::ReportNoiseEvent(GetWorld(), OwnerChar->GetActorLocation(), 0.4f, this);
	}

	RemainingDoses = FMath::Max(RemainingDoses - 1, 0);
	BatteryCharge = MaxDoses > 0 ? ((float)RemainingDoses / MaxDoses) * MaxBatteryCharge : 0.0f;

	// Outil a usage instantane par dose
	Deactivate();
	return true;
}

AScavengerCharacter* AAdrenalineInjectorTool::TraceTeammateTarget() const
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * InjectRange;

	FCollisionQueryParams Params(TEXT("AdrenalineInjectTrace"), false);
	Params.AddIgnoredActor(this);
	if (GetOwner())
	{
		Params.AddIgnoredActor(GetOwner());
	}

	FHitResult HitResult;
	const bool bHit = World->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(35.0f), Params);

	if (bHit && HitResult.GetActor())
	{
		return Cast<AScavengerCharacter>(HitResult.GetActor());
	}

	return nullptr;
}
