#include "Player/LiminalSpectatorPawn.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawnMovement.h"
#include "Player/ScavengerCharacter.h"

ALiminalSpectatorPawn::ALiminalSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	if (USpectatorPawnMovement* SpectatorMovement = Cast<USpectatorPawnMovement>(GetMovementComponent()))
	{
		SpectatorMovement->MaxSpeed = 1200.0f;
	}
}

void ALiminalSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();

	ViewNextPlayer();
}

void ALiminalSpectatorPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bFollowPlayer && TargetSpectatedPlayer.IsValid())
	{
		AScavengerCharacter* Target = TargetSpectatedPlayer.Get();
		if (Target->IsDead())
		{
			ViewNextPlayer();
		}
		else
		{
			const FVector DesiredLocation = Target->GetActorLocation() +
				Target->GetActorRotation().RotateVector(FollowOffset);
			SetActorLocation(FMath::VInterpTo(GetActorLocation(), DesiredLocation, DeltaSeconds, 8.0f));

			if (APlayerController* PC = Cast<APlayerController>(GetController()))
			{
				const FRotator DesiredRot = (Target->GetActorLocation() - GetActorLocation()).Rotation();
				PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), DesiredRot, DeltaSeconds, 10.0f));
			}
		}
	}

	if (IsLocallyControlled() && GEngine)
	{
		const FString Info = FString::Printf(
			TEXT("[MODE SPECTATEUR] %s (Clic: changer joueur | Espace: basculer vol libre)"),
			TargetSpectatedPlayer.IsValid() ? *TargetSpectatedPlayer->GetName() : TEXT("Vol libre"));
		GEngine->AddOnScreenDebugMessage(99991, -1.0f, FColor::Yellow, Info);
	}
}

void ALiminalSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		PlayerInputComponent->BindAction("SpectatorNext", IE_Pressed, this, &ALiminalSpectatorPawn::ViewNextPlayer);
		PlayerInputComponent->BindAction("SpectatorPrev", IE_Pressed, this, &ALiminalSpectatorPawn::ViewPreviousPlayer);
		PlayerInputComponent->BindAction("SpectatorToggleFree", IE_Pressed, this, &ALiminalSpectatorPawn::ToggleFreeCam);

		// Touches directes universelles (garantit le fonctionnement independamment des Input Mappings de projet)
		PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ALiminalSpectatorPawn::ViewNextPlayer);
		PlayerInputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ALiminalSpectatorPawn::ViewPreviousPlayer);
		PlayerInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &ALiminalSpectatorPawn::ToggleFreeCam);
		PlayerInputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ALiminalSpectatorPawn::ViewNextPlayer);
	}
}

TArray<AScavengerCharacter*> ALiminalSpectatorPawn::GetAlivePlayers() const
{
	TArray<AScavengerCharacter*> Alive;
	UWorld* World = GetWorld();
	if (!World)
	{
		return Alive;
	}

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		if (*It && !It->IsDead())
		{
			Alive.Add(*It);
		}
	}

	return Alive;
}

void ALiminalSpectatorPawn::UpdateSpectateTarget(int32 Step)
{
	TArray<AScavengerCharacter*> Alive = GetAlivePlayers();
	if (Alive.Num() == 0)
	{
		TargetSpectatedPlayer = nullptr;
		bFollowPlayer = false;
		return;
	}

	int32 CurrentIndex = -1;
	if (TargetSpectatedPlayer.IsValid())
	{
		CurrentIndex = Alive.IndexOfByKey(TargetSpectatedPlayer.Get());
	}

	int32 NextIndex = (CurrentIndex + Step + Alive.Num()) % Alive.Num();
	TargetSpectatedPlayer = Alive[NextIndex];
	bFollowPlayer = true;
}

void ALiminalSpectatorPawn::ViewNextPlayer()
{
	UpdateSpectateTarget(1);
}

void ALiminalSpectatorPawn::ViewPreviousPlayer()
{
	UpdateSpectateTarget(-1);
}

void ALiminalSpectatorPawn::ToggleFreeCam()
{
	bFollowPlayer = !bFollowPlayer;
	if (!bFollowPlayer)
	{
		TargetSpectatedPlayer = nullptr;
	}
	else
	{
		ViewNextPlayer();
	}
}
