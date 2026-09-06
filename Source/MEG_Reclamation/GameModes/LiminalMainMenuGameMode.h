#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LiminalMainMenuGameMode.generated.h"

/**
 * GameMode pour la carte du menu principal.
 * Definit ALiminalMainMenuHUD comme HUD par defaut.
 * Aucun spawn de personnage, pas d'IA, pas de logique de jeu.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALiminalMainMenuGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
