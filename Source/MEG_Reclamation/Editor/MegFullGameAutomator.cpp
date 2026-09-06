#if WITH_EDITOR

#include "Editor/MegFullGameAutomator.h"

#include "Data/ItemData.h"
#include "Engine/DataTable.h"
#include "HAL/FileManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace MEG_LoopInputBuilder
{
	void BuildLoopInputAssets();
}

namespace MEG_FullGameAutomator
{
	static bool SaveAsset(UObject* Asset)
	{
		UPackage* Package = Asset->GetOutermost();
		if (!Package)
		{
			return false;
		}

		Package->MarkPackageDirty();

		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		Args.Error = GError;

		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());

		return UPackage::SavePackage(Package, Asset, *Filename, Args);
	}

	static void BuildLootDataTable()
	{
		IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Data")), true);

		const FString Container(TEXT("/Game/Data"));
		const FString AssetName(TEXT("DT_LootItems"));
		const FString PackageName = Container / AssetName;

		const FString ExpectedFile = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
		if (FPaths::FileExists(ExpectedFile))
		{
			UE_LOG(LogTemp, Log, TEXT("MEG_FullGameAutomator : DT_LootItems existe deja sur le disque."));
			return;
		}

		UPackage* Package = CreatePackage(*PackageName);
		if (!Package)
		{
			return;
		}

		UDataTable* Table = NewObject<UDataTable>(Package, FName(*AssetName), RF_Public | RF_Standalone);
		Table->RowStruct = FLootItem::StaticStruct();

		auto AddItem = [&](FName Id, const FString& Name, float Weight, int32 Credits)
		{
			FLootItem Row;
			Row.ItemId = Id;
			Row.DisplayName = FText::FromString(Name);
			Row.WeightKg = Weight;
			Row.CreditsValue = Credits;
			Table->AddRow(Id, Row);
		};

		AddItem(TEXT("Scrap_Electronics"), TEXT("Composants Electroniques"), 5.0f, 80);
		AddItem(TEXT("Scrap_CopperCable"), TEXT("Bobine de Cuivre M.E.G."), 12.0f, 140);
		AddItem(TEXT("Item_MegRadio"), TEXT("Radio d'escouade endommagee"), 4.0f, 190);
		AddItem(TEXT("Item_AlmondWater"), TEXT("Bouteille d'Eau d'Amande (500ml)"), 1.0f, 90);
		AddItem(TEXT("Item_Battery9V"), TEXT("Pile 9V Industrielle"), 0.5f, 50);
		AddItem(TEXT("Item_RetroComputer"), TEXT("Terminal portable Mod. 4"), 18.0f, 350);
		AddItem(TEXT("Item_DeclassifiedTape"), TEXT("Cassette classee Secret M.E.G."), 0.2f, 220);
		AddItem(TEXT("Item_MilitaryMRE"), TEXT("Ration de survie scellee"), 1.5f, 60);
		AddItem(TEXT("Item_GeigerCounter"), TEXT("Compteur Geiger analogique"), 3.0f, 160);
		AddItem(TEXT("Item_AnomalousCore"), TEXT("Fragment de cristal non-euclidien"), 7.0f, 500);
		AddItem(TEXT("Item_FirstAidKit"), TEXT("Trousse de secours d'urgence"), 2.5f, 110);
		AddItem(TEXT("Item_HeavyLeadPlate"), TEXT("Blindage anti-radiations en plomb"), 35.0f, 280);

		if (SaveAsset(Table))
		{
			UE_LOG(LogTemp, Log, TEXT("MEG_FullGameAutomator : DataTable DT_LootItems creee avec succes."));
		}
	}

	static void BuildMaterials()
	{
		IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Materials")), true);

		const FString Container(TEXT("/Game/Materials"));
		TArray<FString> MatNames = {
			TEXT("M_LiminalCarpet"),
			TEXT("M_LiminalWall"),
			TEXT("M_LiminalCeiling"),
			TEXT("M_ConcreteHabitable"),
			TEXT("M_PipeMetal"),
			TEXT("M_ElectricCyan"),
			TEXT("M_OfficeFloor"),
			TEXT("M_PitchBlack"),
			TEXT("M_CaveRock"),
			TEXT("M_SuburbsAsphalt"),
			TEXT("M_WheatWood"),
			TEXT("M_PoolTile"),
			TEXT("M_HospitalRed")
		};

		for (const FString& MatName : MatNames)
		{
			const FString PackageName = Container / MatName;
			UPackage* Package = CreatePackage(*PackageName);
			if (!Package)
			{
				continue;
			}

			UMaterial* NewMat = NewObject<UMaterial>(Package, FName(*MatName), RF_Public | RF_Standalone);
			if (NewMat)
			{
				SaveAsset(NewMat);
			}
		}
		UE_LOG(LogTemp, Log, TEXT("MEG_FullGameAutomator : Materiaux generes pour les 11 biomes dans /Game/Materials."));
	}

	void BuildFullGame()
	{
		UE_LOG(LogTemp, Log, TEXT("=== MEG_FullGameAutomator : Construction integrale du jeu lancee ==="));
		BuildLootDataTable();
		MEG_LoopInputBuilder::BuildLoopInputAssets();
		UE_LOG(LogTemp, Log, TEXT("=== MEG_FullGameAutomator : Construction terminee avec succes. ==="));
	}
}

static FAutoConsoleCommand GMEGBuildFullGameCommand(
	TEXT("MEG.BuildFullGame"),
	TEXT("Construit et enregistre tous les assets du jeu (DataTable, Materiaux, IA)"),
	FConsoleCommandDelegate::CreateStatic(&MEG_FullGameAutomator::BuildFullGame));

#endif
