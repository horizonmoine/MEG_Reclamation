#include "Tools/AudioDecoyTool.h"

#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AISense_Hearing.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AAudioDecoyTool::AAudioDecoyTool()
{
	ActiveBatteryDrainPerSecond = 10.0f;
	SetReplicateMovement(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DecoyFinder(
		TEXT("/Game/Meshes/Tools/SM_AudioDecoy.SM_AudioDecoy"));
	if (DecoyFinder.Succeeded() && ToolMesh)
	{
		ToolMesh->SetStaticMesh(DecoyFinder.Object);
		ToolMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
	else
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> FallbackFinder(
			TEXT("/Game/Meshes/Tools/SM_WalkieTalkie.SM_WalkieTalkie"));
		if (FallbackFinder.Succeeded() && ToolMesh)
		{
			ToolMesh->SetStaticMesh(FallbackFinder.Object);
			ToolMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
		}
	}

	DecoyAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("DecoyAudio"));
	DecoyAudio->SetupAttachment(ToolMesh);
	DecoyAudio->bAutoActivate = false;

	DecoyStatusLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("DecoyStatusLight"));
	DecoyStatusLight->SetupAttachment(ToolMesh);
	DecoyStatusLight->SetIntensity(0.0f);
	DecoyStatusLight->SetLightColor(FLinearColor(1.0f, 0.4f, 0.05f));
	DecoyStatusLight->SetAttenuationRadius(500.0f);
	DecoyStatusLight->SetCastShadows(false);
}

void AAudioDecoyTool::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AAudioDecoyTool, bIsDeployed);
}

void AAudioDecoyTool::OnRep_IsDeployed()
{
	if (bIsDeployed)
	{
		DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		if (ToolMesh)
		{
			ToolMesh->SetSimulatePhysics(true);
		}
		if (DecoyStatusLight)
		{
			DecoyStatusLight->SetIntensity(300.0f);
		}
		if (NoiseSound && DecoyAudio)
		{
			DecoyAudio->SetSound(NoiseSound);
			DecoyAudio->Play();
		}
	}
	else
	{
		if (ToolMesh)
		{
			ToolMesh->SetSimulatePhysics(false);
		}
		if (DecoyStatusLight)
		{
			DecoyStatusLight->SetIntensity(0.0f);
		}
		if (DecoyAudio)
		{
			DecoyAudio->Stop();
		}
	}
}

bool AAudioDecoyTool::Activate()
{
	if (!Super::Activate())
	{
		return false;
	}

	bIsDeployed = true;

	DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));

	if (ToolMesh)
	{
		ToolMesh->SetSimulatePhysics(true);
	}

	if (NoiseSound)
	{
		DecoyAudio->SetSound(NoiseSound);
		DecoyAudio->Play();
	}

	if (DecoyStatusLight)
	{
		DecoyStatusLight->SetIntensity(300.0f);
	}

	EmitNoise();
	GetWorldTimerManager().SetTimer(NoiseTimerHandle, this, &AAudioDecoyTool::EmitNoise, NoiseIntervalSeconds, true);

	return true;
}

void AAudioDecoyTool::Deactivate()
{
	Super::Deactivate();

	GetWorldTimerManager().ClearTimer(NoiseTimerHandle);

	if (DecoyAudio)
	{
		DecoyAudio->Stop();
	}

	if (DecoyStatusLight)
	{
		DecoyStatusLight->SetIntensity(0.0f);
	}

	bIsDeployed = false;
}

void AAudioDecoyTool::EmitNoise()
{
	UWorld* World = GetWorld();
	if (!World || !bIsDeployed)
	{
		return;
	}

	UAISense_Hearing::ReportNoiseEvent(World, GetActorLocation(), Loudness, this);
	MakeNoise(Loudness, nullptr, GetActorLocation());

	// Pulsation lumineuse stroboscopique de la balise
	if (DecoyStatusLight)
	{
		const float NewIntensity = (DecoyStatusLight->Intensity > 150.0f) ? 60.0f : 450.0f;
		DecoyStatusLight->SetIntensity(NewIntensity);
	}
}
