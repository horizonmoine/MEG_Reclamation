#include "Crafting/LiminalCraftingComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Player/ScavengerCharacter.h"

ULiminalCraftingComponent::ULiminalCraftingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void ULiminalCraftingComponent::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InitializeDefaultRecipes();
	}
}

void ULiminalCraftingComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ULiminalCraftingComponent, UnlockedRecipeIds);
}

void ULiminalCraftingComponent::InitializeDefaultRecipes()
{
	RecipeDatabase.Empty();

	// Recette 1 : Bottes Silencieuses (Chantier 5 & 6)
	FCraftingRecipe StealthBoots;
	StealthBoots.RecipeId = FName(TEXT("Craft_StealthBoots"));
	StealthBoots.DisplayName = FText::FromString(TEXT("Semelles Amortissantes en Mousse"));
	StealthBoots.Description = FText::FromString(TEXT("Reduit le bruit des pas de 60%, ideal pour eviter les Hounds."));
	StealthBoots.RequiredCredits = 75;
	StealthBoots.RequiredScrap = 5;
	StealthBoots.bUnlockedByDefault = true;
	RecipeDatabase.Add(StealthBoots);

	// Recette 2 : Batterie Haute Capacite (Chantier 7)
	FCraftingRecipe HeavyBattery;
	HeavyBattery.RecipeId = FName(TEXT("Craft_HeavyBattery"));
	HeavyBattery.DisplayName = FText::FromString(TEXT("Accumulateur Lithium M.E.G."));
	HeavyBattery.Description = FText::FromString(TEXT("Double la duree d'utilisation de la lampe frontale et des outils electroniques."));
	HeavyBattery.RequiredCredits = 120;
	HeavyBattery.RequiredScrap = 8;
	HeavyBattery.bUnlockedByDefault = true;
	RecipeDatabase.Add(HeavyBattery);

	// Recette 3 : Filtre UV Stroboscopique (Anti-Smiler)
	FCraftingRecipe UVStrobe;
	UVStrobe.RecipeId = FName(TEXT("Craft_UVStrobe"));
	UVStrobe.DisplayName = FText::FromString(TEXT("Filtre Optique Ultraviolet"));
	UVStrobe.Description = FText::FromString(TEXT("Permet a la torche Flash d'eblouir et repousser les Smilers a courte portee."));
	UVStrobe.RequiredCredits = 200;
	UVStrobe.RequiredScrap = 12;
	UVStrobe.RequiredResearchTag = FName(TEXT("Intel_Entity_Smiler"));
	UVStrobe.bUnlockedByDefault = false;
	RecipeDatabase.Add(UVStrobe);

	// Recette 4 : Injecteur d'Adrenaline Purifiee
	FCraftingRecipe PurifiedAdrenaline;
	PurifiedAdrenaline.RecipeId = FName(TEXT("Craft_Adrenaline"));
	PurifiedAdrenaline.DisplayName = FText::FromString(TEXT("Seringue de Survie Synthetique"));
	PurifiedAdrenaline.Description = FText::FromString(TEXT("Restaure 100% de stamina et 40% de sanite mentale en situation de crise."));
	PurifiedAdrenaline.RequiredCredits = 150;
	PurifiedAdrenaline.RequiredScrap = 10;
	PurifiedAdrenaline.bUnlockedByDefault = true;
	RecipeDatabase.Add(PurifiedAdrenaline);

	for (const FCraftingRecipe& Recipe : RecipeDatabase)
	{
		if (Recipe.bUnlockedByDefault)
		{
			UnlockedRecipeIds.AddUnique(Recipe.RecipeId);
		}
	}
}

bool ULiminalCraftingComponent::IsRecipeUnlocked(const FName& RecipeId) const
{
	return UnlockedRecipeIds.Contains(RecipeId);
}

bool ULiminalCraftingComponent::UnlockRecipeByIntel(const FName& IntelTag)
{
	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		return false;
	}

	bool bUnlockedAny = false;
	for (const FCraftingRecipe& Recipe : RecipeDatabase)
	{
		if (Recipe.RequiredResearchTag == IntelTag && !UnlockedRecipeIds.Contains(Recipe.RecipeId))
		{
			UnlockedRecipeIds.Add(Recipe.RecipeId);
			OnRecipeUnlocked.Broadcast(Recipe.RecipeId);
			bUnlockedAny = true;
		}
	}

	return bUnlockedAny;
}

bool ULiminalCraftingComponent::TryCraftRecipe(AScavengerCharacter* Crafter, const FName& RecipeId)
{
	if (!Crafter)
	{
		return false;
	}

	if (GetOwner() && !GetOwner()->HasAuthority())
	{
		ServerCraftRecipe(Crafter, RecipeId);
		return true;
	}

	if (!IsRecipeUnlocked(RecipeId))
	{
		return false;
	}

	const FCraftingRecipe* FoundRecipe = RecipeDatabase.FindByPredicate([&](const FCraftingRecipe& R) {
		return R.RecipeId == RecipeId;
	});

	if (!FoundRecipe)
	{
		return false;
	}

	// Verification et deduction des credits portés par le joueur
	if (Crafter->GetCarriedCredits() < FoundRecipe->RequiredCredits)
	{
		return false;
	}

	// Deduction des couts
	Crafter->AddCarriedCredits(-FoundRecipe->RequiredCredits);

	// Spawn de l'acteur resultant si configure
	if (FoundRecipe->ResultActorClass && GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		const FVector SpawnLoc = Crafter->GetActorLocation() + Crafter->GetActorForwardVector() * 100.0f;
		GetWorld()->SpawnActor<AActor>(FoundRecipe->ResultActorClass, SpawnLoc, FRotator::ZeroRotator, SpawnParams);
	}

	OnItemCrafted.Broadcast(RecipeId);
	return true;
}

void ULiminalCraftingComponent::ServerCraftRecipe_Implementation(AScavengerCharacter* Crafter, const FName& RecipeId)
{
	TryCraftRecipe(Crafter, RecipeId);
}

void ULiminalCraftingComponent::OnRep_UnlockedRecipes()
{
}
