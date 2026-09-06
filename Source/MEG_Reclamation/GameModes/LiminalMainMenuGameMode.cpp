#include "GameModes/LiminalMainMenuGameMode.h"
#include "UI/LiminalMainMenuHUD.h"
#include "Camera/CameraActor.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"

ALiminalMainMenuGameMode::ALiminalMainMenuGameMode()
{
	HUDClass = ALiminalMainMenuHUD::StaticClass();
	DefaultPawnClass = nullptr;
	bStartPlayersAsSpectators = false;
}

void ALiminalMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// Trouver ou creer une camera cinematique pour le menu
	ACameraActor* MenuCamera = nullptr;
	for (TActorIterator<ACameraActor> It(World); It; ++It)
	{
		MenuCamera = *It;
		break;
	}

	if (!MenuCamera)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		MenuCamera = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(),
			FVector(-300.0f, 0.0f, 160.0f), FRotator(-10.0f, 0.0f, 0.0f), SpawnParams);
	}

	if (APlayerController* PC = World->GetFirstPlayerController())
	{
		if (MenuCamera)
		{
			PC->SetViewTargetWithBlend(MenuCamera);
		}
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
		PC->bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}
}

void ALiminalMainMenuGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		NewPlayer->bShowMouseCursor = true;
		NewPlayer->bEnableClickEvents = true;
		NewPlayer->bEnableMouseOverEvents = true;

		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		NewPlayer->SetInputMode(InputMode);

		for (TActorIterator<ACameraActor> It(GetWorld()); It; ++It)
		{
			NewPlayer->SetViewTargetWithBlend(*It);
			break;
		}
	}
}

