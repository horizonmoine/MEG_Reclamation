#include "GameModes/LiminalLobbyGameMode.h"

#include "Components/PointLightComponent.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Objects/LiminalAirlockActor.h"
#include "Objects/LiminalTerminalActor.h"
#include "Player/ScavengerCharacter.h"
#include "Hub/LiminalHubProgressionComponent.h"
#include "UI/LiminalScavengerHUD.h"

ALiminalLobbyGameMode::ALiminalLobbyGameMode()
{
	DefaultPawnClass = AScavengerCharacter::StaticClass();
	HUDClass = ALiminalScavengerHUD::StaticClass();
	HubProgression = CreateDefaultSubobject<ULiminalHubProgressionComponent>(TEXT("HubProgression"));
}

void ALiminalLobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
	{
		const int32 CompletedCycles = GI->GetSaveData().ActiveQuotaCycle;
		const int32 Credits = GI->GetTotalCredits();
		if (HubProgression)
		{
			HubProgression->RefreshHubTier(Credits, CompletedCycles);
		}
	}
	const EHubTier CurrentTier = HubProgression ? HubProgression->GetCurrentTier() : EHubTier::MakeshiftCamp;

	UWorld* World = GetWorld();
	if (World && HasAuthority())
	{
		// Zone de securite absolue du M.E.G. : elimination stricte de toute entite hostile
		for (TActorIterator<ACharacter> CharIt(World); CharIt; ++CharIt)
		{
			ACharacter* Char = *CharIt;
			if (Char && !Char->IsA(AScavengerCharacter::StaticClass()))
			{
				Char->Destroy();
			}
		}

		// S'assurer de la presence du decor de la Base Alpha
		bool bHasGeometry = false;
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			bHasGeometry = true;
			break;
		}

		if (!bHasGeometry)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
			UStaticMesh* FloorMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Modular/SM_Floor_Tile_400x400.SM_Floor_Tile_400x400"));
			UStaticMesh* WallMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Modular/SM_Wall_Modular_400x300.SM_Wall_Modular_400x300"));
			UStaticMesh* CeilingMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Modular/SM_Ceiling_Tile_400x400.SM_Ceiling_Tile_400x400"));
			UStaticMesh* PillarMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Modular/SM_Pillar_Industrial.SM_Pillar_Industrial"));
			UStaticMesh* CrateMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Props/SM_SupplyCrate_MEG.SM_SupplyCrate_MEG"));
			UStaticMesh* LockerMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Props/SM_HidingLocker.SM_HidingLocker"));
			UStaticMesh* DeskMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Props/SM_Office_Desk.SM_Office_Desk"));
			UStaticMesh* ChairMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Props/SM_Office_Chair.SM_Office_Chair"));
			UStaticMesh* ExitSignMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Props/SM_Exit_Sign.SM_Exit_Sign"));
			UStaticMesh* LightHousingMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/Modular/SM_Ceiling_FluorescentLight.SM_Ceiling_FluorescentLight"));

			UMaterialInterface* BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/LevelPrototyping/Materials/M_FlatCol.M_FlatCol"));
			if (!BaseMat)
			{
				BaseMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
			}

			auto CreateHubMat = [&](const FLinearColor& Col) -> UMaterialInstanceDynamic*
			{
				if (!BaseMat) return nullptr;
				UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(BaseMat, this);
				if (Dyn)
				{
					Dyn->SetVectorParameterValue(FName("Color"), Col);
					Dyn->SetVectorParameterValue(FName("BaseColor"), Col);
				}
				return Dyn;
			};

			UMaterialInstanceDynamic* FloorMat = CreateHubMat(FLinearColor(0.20f, 0.22f, 0.24f));
			UMaterialInstanceDynamic* WallMat = CreateHubMat(FLinearColor(0.28f, 0.30f, 0.32f));
			UMaterialInstanceDynamic* CeilingMat = CreateHubMat(FLinearColor(0.16f, 0.16f, 0.18f));
			UMaterialInstanceDynamic* CrateMat = CreateHubMat(FLinearColor(0.40f, 0.28f, 0.16f));

			// Sol : Dalles modulaires ou cube
			if (FloorMesh)
			{
				for (int32 X = -1; X <= 3; ++X)
				{
					for (int32 Y = -3; Y <= 3; ++Y)
					{
						AStaticMeshActor* Tile = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
							FVector(X * 400.0f, Y * 400.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
						if (Tile && Tile->GetStaticMeshComponent())
						{
							Tile->GetStaticMeshComponent()->SetStaticMesh(FloorMesh);
							Tile->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
						}
					}
				}
			}
			else if (CubeMesh)
			{
				AStaticMeshActor* FloorActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(500.0f, 0.0f, -25.0f), FRotator::ZeroRotator, SpawnParams);
				if (FloorActor && FloorActor->GetStaticMeshComponent())
				{
					FloorActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
					FloorActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(30.0f, 30.0f, 0.5f));
					FloorActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					if (FloorMat) FloorActor->GetStaticMeshComponent()->SetMaterial(0, FloorMat);
				}
			}

			// Plafond modulaire ou cube
			if (CeilingMesh)
			{
				for (int32 X = -1; X <= 3; ++X)
				{
					for (int32 Y = -3; Y <= 3; ++Y)
					{
						AStaticMeshActor* Tile = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
							FVector(X * 400.0f, Y * 400.0f, 300.0f), FRotator(180.0f, 0.0f, 0.0f), SpawnParams);
						if (Tile && Tile->GetStaticMeshComponent())
						{
							Tile->GetStaticMeshComponent()->SetStaticMesh(CeilingMesh);
							Tile->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
						}
					}
				}
			}
			else if (CubeMesh)
			{
				AStaticMeshActor* CeilingActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(500.0f, 0.0f, 425.0f), FRotator::ZeroRotator, SpawnParams);
				if (CeilingActor && CeilingActor->GetStaticMeshComponent())
				{
					CeilingActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
					CeilingActor->GetStaticMeshComponent()->SetWorldScale3D(FVector(30.0f, 30.0f, 0.5f));
					CeilingActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					if (CeilingMat) CeilingActor->GetStaticMeshComponent()->SetMaterial(0, CeilingMat);
				}
			}

			// Murs d'enceinte modulaires
			if (WallMesh)
			{
				// Mur Nord (Y = 1400) et Sud (Y = -1400)
				for (int32 X = -1; X <= 3; ++X)
				{
					AStaticMeshActor* WallN = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
						FVector(X * 400.0f, 1400.0f, 0.0f), FRotator(0.0f, 180.0f, 0.0f), SpawnParams);
					if (WallN && WallN->GetStaticMeshComponent())
					{
						WallN->GetStaticMeshComponent()->SetStaticMesh(WallMesh);
						WallN->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					}

					AStaticMeshActor* WallS = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
						FVector(X * 400.0f, -1400.0f, 0.0f), FRotator::ZeroRotator, SpawnParams);
					if (WallS && WallS->GetStaticMeshComponent())
					{
						WallS->GetStaticMeshComponent()->SetStaticMesh(WallMesh);
						WallS->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					}
				}

				// Mur Est (X = 1400) et Ouest (X = -600)
				for (int32 Y = -3; Y <= 3; ++Y)
				{
					AStaticMeshActor* WallE = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
						FVector(1400.0f, Y * 400.0f, 0.0f), FRotator(0.0f, -90.0f, 0.0f), SpawnParams);
					if (WallE && WallE->GetStaticMeshComponent())
					{
						WallE->GetStaticMeshComponent()->SetStaticMesh(WallMesh);
						WallE->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					}

					AStaticMeshActor* WallW = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
						FVector(-600.0f, Y * 400.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f), SpawnParams);
					if (WallW && WallW->GetStaticMeshComponent())
					{
						WallW->GetStaticMeshComponent()->SetStaticMesh(WallMesh);
						WallW->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					}
				}
			}
			else if (CubeMesh)
			{
				auto SpawnWall = [&](const FVector& Loc, const FVector& Scale)
				{
					AStaticMeshActor* WallActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Loc, FRotator::ZeroRotator, SpawnParams);
					if (WallActor && WallActor->GetStaticMeshComponent())
					{
						WallActor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
						WallActor->GetStaticMeshComponent()->SetWorldScale3D(Scale);
						WallActor->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
						if (WallMat) WallActor->GetStaticMeshComponent()->SetMaterial(0, WallMat);
					}
				};

				SpawnWall(FVector(500.0f, 1500.0f, 200.0f), FVector(30.0f, 0.5f, 4.0f));
				SpawnWall(FVector(500.0f, -1500.0f, 200.0f), FVector(30.0f, 0.5f, 4.0f));
				SpawnWall(FVector(2000.0f, 0.0f, 200.0f), FVector(0.5f, 30.0f, 4.0f));
				SpawnWall(FVector(-1000.0f, 0.0f, 200.0f), FVector(0.5f, 30.0f, 4.0f));
			}

			// Piliers industriels de structure
			if (PillarMesh)
			{
				TArray<FVector> PillarLocations = {
					FVector(-200.0f, -600.0f, 0.0f),
					FVector(-200.0f, 600.0f, 0.0f),
					FVector(1000.0f, -600.0f, 0.0f),
					FVector(1000.0f, 600.0f, 0.0f)
				};
				for (const FVector& PLoc : PillarLocations)
				{
					AStaticMeshActor* Pillar = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), PLoc, FRotator::ZeroRotator, SpawnParams);
					if (Pillar && Pillar->GetStaticMeshComponent())
					{
						Pillar->GetStaticMeshComponent()->SetStaticMesh(PillarMesh);
						Pillar->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					}
				}
			}

			// Eclairage de la Base
			FLinearColor LightColor = FLinearColor(1.0f, 0.85f, 0.65f);
			float LightIntensity = 2200.0f;
			if (CurrentTier == EHubTier::ReinforcedOutpost)
			{
				LightColor = FLinearColor(1.0f, 1.0f, 0.95f);
				LightIntensity = 3500.0f;
			}
			else if (CurrentTier == EHubTier::ScientificLab)
			{
				LightColor = FLinearColor(0.80f, 0.95f, 1.0f);
				LightIntensity = 4500.0f;
			}

			auto SpawnHubLight = [&](const FVector& Pos)
			{
				if (LightHousingMesh)
				{
					AStaticMeshActor* LightHousing = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
						Pos + FVector(0.0f, 0.0f, 10.0f), FRotator::ZeroRotator, SpawnParams);
					if (LightHousing && LightHousing->GetStaticMeshComponent())
					{
						LightHousing->GetStaticMeshComponent()->SetStaticMesh(LightHousingMesh);
						LightHousing->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("NoCollision"));
					}
				}

				APointLight* LightActor = World->SpawnActor<APointLight>(APointLight::StaticClass(), Pos, FRotator::ZeroRotator, SpawnParams);
				if (LightActor && LightActor->GetLightComponent())
				{
					LightActor->GetLightComponent()->SetMobility(EComponentMobility::Movable);
					if (UPointLightComponent* PLC = Cast<UPointLightComponent>(LightActor->GetLightComponent()))
					{
						PLC->SetLightColor(LightColor);
						PLC->SetIntensity(LightIntensity);
						PLC->SetAttenuationRadius(1900.0f);
					}
				}
			};

			SpawnHubLight(FVector(0.0f, 400.0f, 280.0f));
			SpawnHubLight(FVector(0.0f, -400.0f, 280.0f));
			SpawnHubLight(FVector(800.0f, 400.0f, 280.0f));
			SpawnHubLight(FVector(800.0f, -400.0f, 280.0f));

			// Caisses M.E.G.
			UStaticMesh* UsableCrateMesh = CrateMesh ? CrateMesh : CubeMesh;
			auto SpawnCrate = [&](const FVector& Pos, const FRotator& Rot, const FVector& Scale)
			{
				AStaticMeshActor* Crate = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Pos, Rot, SpawnParams);
				if (Crate && Crate->GetStaticMeshComponent())
				{
					Crate->GetStaticMeshComponent()->SetStaticMesh(UsableCrateMesh);
					Crate->GetStaticMeshComponent()->SetWorldScale3D(Scale);
					Crate->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
					if (!CrateMesh && CrateMat) Crate->GetStaticMeshComponent()->SetMaterial(0, CrateMat);
				}
			};

			SpawnCrate(FVector(200.0f, 800.0f, 20.0f), FRotator(0.0f, 15.0f, 0.0f), FVector(1.0f));
			SpawnCrate(FVector(200.0f, -800.0f, 20.0f), FRotator(0.0f, -25.0f, 0.0f), FVector(1.0f));
			SpawnCrate(FVector(600.0f, 800.0f, 20.0f), FRotator(0.0f, 45.0f, 0.0f), FVector(1.0f));

			if (CurrentTier >= EHubTier::ReinforcedOutpost)
			{
				SpawnCrate(FVector(800.0f, -800.0f, 20.0f), FRotator(0.0f, 10.0f, 0.0f), FVector(1.2f));
				SpawnCrate(FVector(800.0f, -800.0f, 100.0f), FRotator(0.0f, 75.0f, 0.0f), FVector(0.9f));
			}
			if (CurrentTier == EHubTier::ScientificLab)
			{
				SpawnCrate(FVector(1100.0f, 800.0f, 20.0f), FRotator::ZeroRotator, FVector(1.4f));
				SpawnCrate(FVector(1100.0f, -800.0f, 20.0f), FRotator::ZeroRotator, FVector(1.4f));
			}

			// Casiers vestiaires M.E.G.
			if (LockerMesh)
			{
				AStaticMeshActor* Locker1 = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(-500.0f, -200.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f), SpawnParams);
				if (Locker1 && Locker1->GetStaticMeshComponent())
				{
					Locker1->GetStaticMeshComponent()->SetStaticMesh(LockerMesh);
					Locker1->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
				}

				AStaticMeshActor* Locker2 = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(-500.0f, 200.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f), SpawnParams);
				if (Locker2 && Locker2->GetStaticMeshComponent())
				{
					Locker2->GetStaticMeshComponent()->SetStaticMesh(LockerMesh);
					Locker2->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
				}
			}

			// Bureau et chaise M.E.G.
			if (DeskMesh)
			{
				AStaticMeshActor* Desk = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(300.0f, -250.0f, 0.0f), FRotator(0.0f, -90.0f, 0.0f), SpawnParams);
				if (Desk && Desk->GetStaticMeshComponent())
				{
					Desk->GetStaticMeshComponent()->SetStaticMesh(DeskMesh);
					Desk->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
				}
			}
			if (ChairMesh)
			{
				AStaticMeshActor* Chair = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(300.0f, -320.0f, 0.0f), FRotator(0.0f, 90.0f, 0.0f), SpawnParams);
				if (Chair && Chair->GetStaticMeshComponent())
				{
					Chair->GetStaticMeshComponent()->SetStaticMesh(ChairMesh);
					Chair->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
				}
			}

			// Panneau EXIT au-dessus du sas
			if (ExitSignMesh)
			{
				AStaticMeshActor* ExitSign = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
					FVector(1000.0f, 0.0f, 240.0f), FRotator(0.0f, 180.0f, 0.0f), SpawnParams);
				if (ExitSign && ExitSign->GetStaticMeshComponent())
				{
					ExitSign->GetStaticMeshComponent()->SetStaticMesh(ExitSignMesh);
					ExitSign->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("NoCollision"));
				}
			}

			// Ambiance sonore : Bourdonnement fluorescent M.E.G.
			USoundBase* HumSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Audio/S_Fluorescent_Hum.S_Fluorescent_Hum"));
			if (HumSound)
			{
				UGameplayStatics::PlaySoundAtLocation(World, HumSound, FVector(500.0f, 0.0f, 200.0f), 0.40f, 1.0f);
			}
		}
		// S'assurer de la presence du Terminal de mission M.E.G.
		bool bHasTerminal = false;
		for (TActorIterator<ALiminalTerminalActor> It(World); It; ++It)
		{
			bHasTerminal = true;
			break;
		}
		if (!bHasTerminal)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<ALiminalTerminalActor>(ALiminalTerminalActor::StaticClass(),
				FVector(450.0f, 0.0f, 80.0f), FRotator(0.0f, 180.0f, 0.0f), Params);
		}

		// S'assurer de la presence du Sas d'incursion
		bool bHasAirlock = false;
		for (TActorIterator<ALiminalAirlockActor> It(World); It; ++It)
		{
			bHasAirlock = true;
			break;
		}
		if (!bHasAirlock)
		{
			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			World->SpawnActor<ALiminalAirlockActor>(ALiminalAirlockActor::StaticClass(),
				FVector(1000.0f, 0.0f, 80.0f), FRotator(0.0f, 180.0f, 0.0f), Params);
		}
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Cyan,
			TEXT("[BASE ALPHA M.E.G.] Zone securisee active. Utilisez le terminal [E] pour choisir un biome, puis le sas [E] pour embarquer."));
	}
}

void ALiminalLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer && GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green,
			FString::Printf(TEXT("Agent connecte au Hub : %s"), *NewPlayer->GetName()));
	}
}

void ALiminalLobbyGameMode::LaunchSquadMission(ELevelBiome Biome)
{
	if (!HasAuthority())
	{
		return;
	}

	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
	{
		GI->SetSelectedBiome(Biome);
		GI->TravelToMission();
	}
}
