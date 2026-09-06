#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LiminalCraftingComponent.generated.h"

class AScavengerCharacter;
class ABaseTool;

USTRUCT(BlueprintType)
struct FCraftingRecipe
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FName RecipeId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 RequiredScrap = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	int32 RequiredCredits = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	FName RequiredResearchTag = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	TSubclassOf<AActor> ResultActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	bool bUnlockedByDefault = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemCrafted, const FName&, RecipeId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRecipeUnlocked, const FName&, RecipeId);

/**
 * Composant d'etabli de fabrication et d'amelioration d'equipement M.E.G.
 * Server-authoritative : validation des recettes, debit des ressources et application des ameliorations.
 */
UCLASS(ClassGroup = (Progression), meta = (BlueprintSpawnableComponent))
class MEG_RECLAMATION_API ULiminalCraftingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULiminalCraftingComponent();

	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Tente de fabriquer une recette pour le scavenger specifie */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Crafting")
	bool TryCraftRecipe(AScavengerCharacter* Crafter, const FName& RecipeId);

	/** Deverrouille une recette suite a la decouverte d'un rapport Intel M.E.G. */
	UFUNCTION(BlueprintCallable, Category = "Liminal|Crafting")
	bool UnlockRecipeByIntel(const FName& IntelTag);

	UFUNCTION(BlueprintPure, Category = "Liminal|Crafting")
	bool IsRecipeUnlocked(const FName& RecipeId) const;

	UFUNCTION(BlueprintPure, Category = "Liminal|Crafting")
	const TArray<FCraftingRecipe>& GetAvailableRecipes() const { return RecipeDatabase; }

	// Server RPC
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Liminal|Crafting")
	void ServerCraftRecipe(AScavengerCharacter* Crafter, const FName& RecipeId);

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Crafting")
	FOnItemCrafted OnItemCrafted;

	UPROPERTY(BlueprintAssignable, Category = "Liminal|Crafting")
	FOnRecipeUnlocked OnRecipeUnlocked;

protected:
	void InitializeDefaultRecipes();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Liminal|Crafting")
	TArray<FCraftingRecipe> RecipeDatabase;

	UPROPERTY(ReplicatedUsing = OnRep_UnlockedRecipes)
	TArray<FName> UnlockedRecipeIds;

	UFUNCTION()
	void OnRep_UnlockedRecipes();
};
