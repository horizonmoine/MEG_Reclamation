#include "GameModes/LiminalMainMenuGameMode.h"
#include "UI/LiminalMainMenuHUD.h"
#include "GameFramework/DefaultPawn.h"

ALiminalMainMenuGameMode::ALiminalMainMenuGameMode()
{
	HUDClass = ALiminalMainMenuHUD::StaticClass();

	// Pas de pawn de jeu dans le menu principal
	DefaultPawnClass = ADefaultPawn::StaticClass();

	// Pas de spectateur
	bStartPlayersAsSpectators = true;
}
