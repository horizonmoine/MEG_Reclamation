#include "AI/AnimNotify_LiminalAttackTrace.h"
#include "AI/LiminalEntity.h"
#include "Components/SkeletalMeshComponent.h"

UAnimNotify_LiminalAttackTrace::UAnimNotify_LiminalAttackTrace()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(255, 30, 30, 255);
#endif
}

void UAnimNotify_LiminalAttackTrace::Notify(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	if (ALiminalEntity* Entity = Cast<ALiminalEntity>(Owner))
	{
		const float DamageToApply = bOverrideDamage ? CustomDamage : -1.0f;
		Entity->OnAttackNotify(SocketName, TraceRadius, TraceDistance, DamageToApply, DamageTypeClass, bDrawDebugTrace);
	}
}

FString UAnimNotify_LiminalAttackTrace::GetNotifyName_Implementation() const
{
	return FString::Printf(TEXT("Liminal Attack Trace [%s]"), *SocketName.ToString());
}
