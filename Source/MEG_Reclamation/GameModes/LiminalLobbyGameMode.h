#pragma once

#include "CoreMinimal.h"
#include "Data/LiminalGameInstance.h"
#include "GameFramework/GameModeBase.h"
#include "LiminalLobbyGameMode.generated.h"

class AScavengerCharacter;
class ULiminalHubProgressionComponent;

/**
 * Mode de jeu du Hub de repos (M.E.G. Safe Zone).
 * Pas d'ennemis, pas de pression de sanite, acces au terminal de mission et magasin.
 */
UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALiminalLobbyGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void LaunchSquadMission(ELevelBiome Biome);

	UFUNCTION(BlueprintPure, Category = "Hub")
	ULiminalHubProgressionComponent* GetHubProgression() const { return HubProgression; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Hub")
	TObjectPtr<ULiminalHubProgressionComponent> HubProgression;
};
