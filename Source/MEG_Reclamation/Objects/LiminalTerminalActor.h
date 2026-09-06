#pragma once

#include "CoreMinimal.h"
#include "Data/LiminalGameInstance.h"
#include "GameFramework/Actor.h"
#include "LiminalTerminalActor.generated.h"

class UBoxComponent;
class UPointLightComponent;
class UStaticMeshComponent;
class AScavengerCharacter;
class ULiminalCraftingComponent;

USTRUCT(BlueprintType)
struct FTerminalStoreItem
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Store")
	int32 CostCredits = 100;
};

UCLASS(Blueprintable)
class MEG_RECLAMATION_API ALiminalTerminalActor : public AActor
{
	GENERATED_BODY()

public:
	ALiminalTerminalActor();

	UFUNCTION(BlueprintCallable, Category = "Terminal")
	void Interact(AScavengerCharacter* InteractingPlayer);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Terminal")
	void ServerPurchaseStoreItem(FName ItemId, AScavengerCharacter* Buyer);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Terminal")
	void ServerSelectBiome(ELevelBiome Biome);

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Terminal")
	void ServerLaunchIncursion();

	UFUNCTION(BlueprintPure, Category = "Terminal")
	const TArray<FTerminalStoreItem>& GetStoreCatalog() const { return StoreCatalog; }

	UFUNCTION(BlueprintPure, Category = "Terminal")
	ELevelBiome GetCurrentlySelectedBiome() const { return SelectedBiome; }

	UFUNCTION(BlueprintPure, Category = "Terminal")
	ULiminalCraftingComponent* GetCraftingComponent() const { return CraftingComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terminal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULiminalCraftingComponent> CraftingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terminal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UBoxComponent> InteractionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terminal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> TerminalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terminal", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPointLightComponent> ScreenLight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terminal")
	TArray<FTerminalStoreItem> StoreCatalog;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Terminal")
	ELevelBiome SelectedBiome = ELevelBiome::Level0_YellowLobby;
};
