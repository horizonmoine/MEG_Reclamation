#if WITH_EDITOR

#include "HAL/FileManager.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace MEG_LoopInputBuilder
{
	static UInputAction* FindOrCreateInputAction(const FString& Container, const FString& Name,
		EInputActionValueType ValueType)
	{
		const FString PackageName = Container / Name;
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *PackageName, *Name);

		if (UInputAction* Existing = LoadObject<UInputAction>(nullptr, *ObjectPath))
		{
			return Existing;
		}

		UPackage* Package = FindPackage(nullptr, *PackageName);
		if (!Package)
		{
			Package = CreatePackage(*PackageName);
		}

		UInputAction* Action = NewObject<UInputAction>(Package, FName(*Name), RF_Public | RF_Standalone);
		Action->ValueType = ValueType;
		return Action;
	}

	static bool SaveAsset(UObject* Asset)
	{
		UPackage* Package = Asset->GetOutermost();
		if (!Package)
		{
			return false;
		}

		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(), FPackageName::GetAssetPackageExtension());

		if (FPaths::FileExists(Filename))
		{
			return true;
		}

		Package->MarkPackageDirty();

		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		Args.Error = GError;

		return UPackage::SavePackage(Package, Asset, *Filename, Args);
	}

	template <typename T>
	static T* NewModifier(UInputMappingContext* Context)
	{
		return NewObject<T>(Context, NAME_None, RF_Transactional);
	}

	void BuildLoopInputAssets()
	{
		const FString ImcExpectedFile = FPaths::ProjectContentDir() / TEXT("Input/Scavenger/IMC_Scavenger.uasset");
		if (FPaths::FileExists(ImcExpectedFile))
		{
			UE_LOG(LogTemp, Log, TEXT("MEG_LoopInputBuilder : IMC_Scavenger existe deja sur le disque."));
			return;
		}

		const FString Container(TEXT("/Game/Input/Scavenger"));


		UInputAction* MoveAction = FindOrCreateInputAction(Container, TEXT("IA_Move"), EInputActionValueType::Axis2D);
		UInputAction* LookAction = FindOrCreateInputAction(Container, TEXT("IA_Look"), EInputActionValueType::Axis2D);
		UInputAction* JumpAction = FindOrCreateInputAction(Container, TEXT("IA_Jump"), EInputActionValueType::Boolean);
		UInputAction* GrabAction = FindOrCreateInputAction(Container, TEXT("IA_Grab"), EInputActionValueType::Boolean);
		UInputAction* SprintAction = FindOrCreateInputAction(Container, TEXT("IA_Sprint"), EInputActionValueType::Boolean);
		UInputAction* UseToolAction = FindOrCreateInputAction(Container, TEXT("IA_UseTool"), EInputActionValueType::Boolean);
		UInputAction* CycleToolAction = FindOrCreateInputAction(Container, TEXT("IA_CycleTool"), EInputActionValueType::Boolean);
		UInputAction* DeployToolAction = FindOrCreateInputAction(Container, TEXT("IA_DeployTool"), EInputActionValueType::Boolean);
		UInputAction* ToggleHeadlampAction = FindOrCreateInputAction(Container, TEXT("IA_ToggleHeadlamp"), EInputActionValueType::Boolean);
		UInputAction* InteractAction = FindOrCreateInputAction(Container, TEXT("IA_Interact"), EInputActionValueType::Boolean);
		UInputAction* CrouchAction = FindOrCreateInputAction(Container, TEXT("IA_Crouch"), EInputActionValueType::Boolean);
		UInputAction* DropLootAction = FindOrCreateInputAction(Container, TEXT("IA_DropLoot"), EInputActionValueType::Boolean);
		UInputAction* ThrowLootAction = FindOrCreateInputAction(Container, TEXT("IA_ThrowLoot"), EInputActionValueType::Boolean);
		UInputAction* NightVisionAction = FindOrCreateInputAction(Container, TEXT("IA_NightVision"), EInputActionValueType::Boolean);
		UInputAction* FieldManualAction = FindOrCreateInputAction(Container, TEXT("IA_FieldManual"), EInputActionValueType::Boolean);

		if (!MoveAction || !LookAction || !JumpAction || !GrabAction || !SprintAction || !UseToolAction || !CycleToolAction || !DeployToolAction || !ToggleHeadlampAction || !InteractAction || !CrouchAction || !DropLootAction || !ThrowLootAction || !NightVisionAction || !FieldManualAction)
		{
			UE_LOG(LogTemp, Error, TEXT("MEG.BuildLoopInput : echec de creation des InputActions"));
			return;
		}

		const FString PackageName = Container / TEXT("IMC_Scavenger");
		UPackage* ImcPackage = FindPackage(nullptr, *PackageName);
		if (!ImcPackage)
		{
			ImcPackage = CreatePackage(*PackageName);
		}

		UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(nullptr,
			*FString::Printf(TEXT("%s.IMC_Scavenger"), *PackageName));
		if (!MappingContext)
		{
			MappingContext = NewObject<UInputMappingContext>(ImcPackage, FName(TEXT("IMC_Scavenger")),
				RF_Public | RF_Standalone);
		}

		MappingContext->Modify();

		auto AddMapping = [MappingContext](UInputAction* Action, FKey Key,
			const TArray<UInputModifier*>& Modifiers)
		{
			MappingContext->UnmapKey(Action, Key);

			FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(Action, Key);
			for (UInputModifier* Modifier : Modifiers)
			{
				Mapping.Modifiers.Add(Modifier);
			}
		};

		// --- DEPLACEMENT OMNIDIRECTIONNEL (AZERTY ZQSD + QWERTY WASD + FLECHES) ---
		UInputModifierSwizzleAxis* SwizzleForward = NewModifier<UInputModifierSwizzleAxis>(MappingContext);
		SwizzleForward->Order = EInputAxisSwizzle::YXZ;
		AddMapping(MoveAction, EKeys::W, { SwizzleForward });
		AddMapping(MoveAction, EKeys::Z, { SwizzleForward });
		AddMapping(MoveAction, EKeys::Up, { SwizzleForward });

		UInputModifierSwizzleAxis* SwizzleBack = NewModifier<UInputModifierSwizzleAxis>(MappingContext);
		SwizzleBack->Order = EInputAxisSwizzle::YXZ;
		UInputModifierNegate* NegateBack = NewModifier<UInputModifierNegate>(MappingContext);
		AddMapping(MoveAction, EKeys::S, { SwizzleBack, NegateBack });
		AddMapping(MoveAction, EKeys::Down, { SwizzleBack, NegateBack });

		AddMapping(MoveAction, EKeys::D, {});
		AddMapping(MoveAction, EKeys::Right, {});

		UInputModifierNegate* NegateLeft = NewModifier<UInputModifierNegate>(MappingContext);
		AddMapping(MoveAction, EKeys::A, { NegateLeft });
		AddMapping(MoveAction, EKeys::Q, { NegateLeft });
		AddMapping(MoveAction, EKeys::Left, { NegateLeft });

		// --- REGARD SOURIS ---
		AddMapping(LookAction, EKeys::MouseX, {});
		UInputModifierSwizzleAxis* SwizzleMouseY = NewModifier<UInputModifierSwizzleAxis>(MappingContext);
		SwizzleMouseY->Order = EInputAxisSwizzle::YXZ;
		UInputModifierNegate* NegateMouseY = NewModifier<UInputModifierNegate>(MappingContext);
		AddMapping(LookAction, EKeys::MouseY, { SwizzleMouseY, NegateMouseY });

		// --- ACTIONS DE SURVIE, INTERACTION & PHYSIQUE ---
		AddMapping(JumpAction, EKeys::SpaceBar, {});
		AddMapping(InteractAction, EKeys::E, {});
		AddMapping(GrabAction, EKeys::RightMouseButton, {});
		AddMapping(SprintAction, EKeys::LeftShift, {});
		AddMapping(CrouchAction, EKeys::C, {});
		AddMapping(CrouchAction, EKeys::LeftControl, {});
		AddMapping(DropLootAction, EKeys::X, {});
		AddMapping(ThrowLootAction, EKeys::R, {});
		AddMapping(NightVisionAction, EKeys::N, {});
		AddMapping(FieldManualAction, EKeys::M, {});
		AddMapping(FieldManualAction, EKeys::J, {});

		// --- GESTION DE L'ARSENAL ---
		AddMapping(UseToolAction, EKeys::LeftMouseButton, {});
		AddMapping(CycleToolAction, EKeys::MouseScrollUp, {});
		AddMapping(CycleToolAction, EKeys::MouseScrollDown, {});
		AddMapping(CycleToolAction, EKeys::One, {});
		AddMapping(CycleToolAction, EKeys::Two, {});
		AddMapping(CycleToolAction, EKeys::Three, {});
		AddMapping(CycleToolAction, EKeys::Four, {});
		AddMapping(CycleToolAction, EKeys::Tab, {});

		AddMapping(DeployToolAction, EKeys::G, {});
		AddMapping(ToggleHeadlampAction, EKeys::F, {});
		AddMapping(ToggleHeadlampAction, EKeys::T, {});
		AddMapping(ToggleHeadlampAction, EKeys::L, {});

		IFileManager::Get().MakeDirectory(*(FPaths::ProjectContentDir() / TEXT("Input/Scavenger")), true);

		const bool bSavedIMC = SaveAsset(MappingContext);
		bool bAllSaved = bSavedIMC;

		TArray<UInputAction*> AllActions = {
			MoveAction, LookAction, JumpAction, GrabAction, SprintAction,
			UseToolAction, CycleToolAction, DeployToolAction, ToggleHeadlampAction,
			InteractAction, CrouchAction, DropLootAction, ThrowLootAction,
			NightVisionAction, FieldManualAction
		};
		for (UInputAction* Action : AllActions)
		{
			bAllSaved = SaveAsset(Action) && bAllSaved;
		}


		UE_LOG(LogTemp, Log, TEXT("MEG.BuildLoopInput : %s (AZERTY, QWERTY, Souris omnidirectionnelle et Arsenal configures)"),
			bAllSaved ? TEXT("assets input sauvegardes") : TEXT("ECHEC DE SAUVEGARDE"));
	}
}

static FAutoConsoleCommand GMEGBuildLoopInputCommand(
	TEXT("MEG.BuildLoopInput"),
	TEXT("Genere les assets Enhanced Input du Scavenger dans /Game/Input/Scavenger."),
	FConsoleCommandDelegate::CreateStatic(&MEG_LoopInputBuilder::BuildLoopInputAssets));

#endif
