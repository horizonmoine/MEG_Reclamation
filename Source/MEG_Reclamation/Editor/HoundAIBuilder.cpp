#if WITH_EDITOR

#include "Editor/HoundAIBuilder.h"

#include "AI/LiminalAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "HAL/FileManager.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace MEG_HoundAIBuilder
{
	template <typename T>
	static T* FindOrCreateAsset(const FString& ContainerPath, const FString& AssetName)
	{
		const FString PackageName = ContainerPath / AssetName;
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *AssetName);

		if (T* Existing = FindObject<T>(nullptr, *ObjectPath))
		{
			return Existing;
		}

		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (!Package)
		{
			Package = CreatePackage(*PackageName);
		}

		return NewObject<T>(Package, FName(*AssetName), RF_Public | RF_Standalone);
	}

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

	void BuildHoundAssets()
	{
		const FString Container(TEXT("/Game/AI"));
		const FString BtExpectedFile = FPackageName::LongPackageNameToFilename(Container / TEXT("BT_Hound"), FPackageName::GetAssetPackageExtension());
		if (FPaths::FileExists(BtExpectedFile))
		{
			UE_LOG(LogTemp, Log, TEXT("MEG.BuildHoundAI : BT_Hound existe deja sur le disque."));
			return;
		}

		UBlackboardData* Blackboard = FindOrCreateAsset<UBlackboardData>(Container, TEXT("BB_Hound"));
		if (!Blackboard)
		{
			UE_LOG(LogTemp, Error, TEXT("MEG.BuildHoundAI : impossible de creer BB_Hound"));
			return;
		}

		Blackboard->Modify();
		Blackboard->Keys.Empty();

		FBlackboardEntry TargetEntry;
		TargetEntry.EntryName = ALiminalAIController::PriorityTargetKey;
		TargetEntry.KeyType = NewObject<UBlackboardKeyType_Vector>(Blackboard, NAME_None, RF_Transactional);
		Blackboard->Keys.Add(TargetEntry);

		UBehaviorTree* BehaviorTree = FindOrCreateAsset<UBehaviorTree>(Container, TEXT("BT_Hound"));
		if (!BehaviorTree)
		{
			UE_LOG(LogTemp, Error, TEXT("MEG.BuildHoundAI : impossible de creer BT_Hound"));
			return;
		}

		BehaviorTree->Modify();

		if (BehaviorTree->RootNode)
		{
			BehaviorTree->RootNode->Rename(nullptr, GetTransientPackage(),
				REN_DontCreateRedirectors | REN_DoNotDirty | REN_AllowPackageLinkerMismatch);
			BehaviorTree->RootNode = nullptr;
		}

		UBehaviorTree* BTAsset = BehaviorTree;

		UBTComposite_Sequence* SequenceRoot = NewObject<UBTComposite_Sequence>(BTAsset);
		SequenceRoot->InitializeNode(nullptr, MAX_uint16, 0, 0);

		UBTTask_MoveTo* MoveTask = NewObject<UBTTask_MoveTo>(BTAsset);

		FBTCompositeChild& ChildInfo = SequenceRoot->Children.AddDefaulted_GetRef();
		ChildInfo.ChildComposite = nullptr;
		ChildInfo.ChildTask = MoveTask;

		MoveTask->InitializeNode(SequenceRoot, 0, 0, 1);

		if (FStructProperty* SelectorProp = FindFProperty<FStructProperty>(
			UBTTask_MoveTo::StaticClass(), FName(TEXT("BlackboardKey"))))
		{
			FBlackboardKeySelector* Selector = SelectorProp->ContainerPtrToValuePtr<FBlackboardKeySelector>(MoveTask);
			Selector->SelectedKeyName = ALiminalAIController::PriorityTargetKey;
			Selector->SelectedKeyType = UBlackboardKeyType_Vector::StaticClass();
		}

		MoveTask->bObserveBlackboardValue = true;

		BehaviorTree->RootNode = SequenceRoot;
		BehaviorTree->BlackboardAsset = Blackboard;

		IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("AI")), true);

		const bool bSavedBB = SaveAsset(Blackboard);
		const bool bSavedBT = SaveAsset(BehaviorTree);

		UE_LOG(LogTemp, Log,
			TEXT("MEG.BuildHoundAI : BB=%s BT=%s"),
			bSavedBB ? TEXT("OK") : TEXT("ECHEC"),
			bSavedBT ? TEXT("OK") : TEXT("ECHEC"));
	}
}

static FAutoConsoleCommand GMEGBuildHoundAICommand(
	TEXT("MEG.BuildHoundAI"),
	TEXT("Genere /Game/AI/BB_Hound et /Game/AI/BT_Hound pour l'entite Hound."),
	FConsoleCommandDelegate::CreateStatic(&MEG_HoundAIBuilder::BuildHoundAssets));

#endif
