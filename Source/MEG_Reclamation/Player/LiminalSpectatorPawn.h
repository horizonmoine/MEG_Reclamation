#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SpectatorPawn.h"
#include "LiminalSpectatorPawn.generated.h"

class AScavengerCharacter;

UCLASS()
class MEG_RECLAMATION_API ALiminalSpectatorPawn : public ASpectatorPawn
{
	GENERATED_BODY()

public:
	ALiminalSpectatorPawn();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void ViewNextPlayer();

	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void ViewPreviousPlayer();

	UFUNCTION(BlueprintCallable, Category = "Spectator")
	void ToggleFreeCam();

	UFUNCTION(BlueprintPure, Category = "Spectator")
	AScavengerCharacter* GetTargetSpectatedPlayer() const { return TargetSpectatedPlayer.Get(); }

	UFUNCTION(BlueprintPure, Category = "Spectator")
	bool IsFollowingPlayer() const { return bFollowPlayer; }

protected:
	virtual void BeginPlay() override;

	UPROPERTY(BlueprintReadOnly, Category = "Spectator")
	TWeakObjectPtr<AScavengerCharacter> TargetSpectatedPlayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
	bool bFollowPlayer = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spectator")
	FVector FollowOffset = FVector(-180.0f, 0.0f, 70.0f);

private:
	void UpdateSpectateTarget(int32 Step);
	TArray<AScavengerCharacter*> GetAlivePlayers() const;
};
