#include "UI/LiminalScavengerHUD.h"

#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Objects/LootActor.h"
#include "Player/ScavengerCharacter.h"
#include "Player/LiminalSpectatorPawn.h"
#include "Sanity/LiminalSanityTypes.h"
#include "Tools/BaseTool.h"
#include "Data/LiminalGameInstance.h"
#include "GameModes/LiminalGameMode.h"
#include "Objects/LiminalAirlockActor.h"
#include "Objects/LiminalTerminalActor.h"
#include "Objects/LiminalKeypadActor.h"
#include "Objects/LiminalBreakerActor.h"
#include "Objects/LiminalDoorActor.h"
#include "Objects/LiminalHidingSpot.h"
#include "Objects/LiminalVentActor.h"
#include "Objects/LiminalKeyItemActor.h"
#include "Objects/LiminalValvePuzzleActor.h"
#include "Objects/LiminalFuseBoxActor.h"
#include "EngineUtils.h"

ALiminalScavengerHUD::ALiminalScavengerHUD()
{
}

AScavengerCharacter* ALiminalScavengerHUD::GetOwningScavenger() const
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		return Cast<AScavengerCharacter>(PC->GetPawn());
	}
	return nullptr;
}

ALiminalSpectatorPawn* ALiminalScavengerHUD::GetOwningSpectator() const
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		return Cast<ALiminalSpectatorPawn>(PC->GetPawn());
	}
	return nullptr;
}

void ALiminalScavengerHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float ScreenWidth = Canvas->ClipX;
	const float ScreenHeight = Canvas->ClipY;

	// Mise a jour des timers d'animation analogiques
	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	GlitchTimer += DeltaSeconds * 3.0f;
	ScanlineOffset = FMath::Fmod(ScanlineOffset + DeltaSeconds * 40.0f, ScreenHeight);

	// Si le Terminal M.E.G. est ouvert
	if (bShowTerminalUI)
	{
		HandleTerminalInput();
		DrawTerminalUI(ScreenWidth, ScreenHeight);
		return;
	}

	// Si le manuel de terrain tactique M.E.G. est ouvert
	if (bShowFieldManual)
	{
		DrawFieldManual(ScreenWidth, ScreenHeight);
		return;
	}

	// Si le joueur est en mode spectateur (elimine en mission ou observateur)
	if (ALiminalSpectatorPawn* Spectator = GetOwningSpectator())
	{
		DrawSpectatorHUD(Spectator, ScreenWidth, ScreenHeight);
		return;
	}

	AScavengerCharacter* Scavenger = GetOwningScavenger();
	if (!Scavenger)
	{
		return;
	}
	const float CenterX = ScreenWidth * 0.5f;
	const float CenterY = ScreenHeight * 0.5f;

	// 1. Dessine le reticule central et prompts d'interaction
	DrawReticle(Scavenger, CenterX, CenterY);

	// 2. Dessine les jauges vitales (Sante, Sanite, Stamina, Poids)
	DrawSurvivalGauges(Scavenger, CenterX, CenterY);

	// 3. Dessine le statut de l'outil actif et de la batterie
	DrawToolStatus(Scavenger, ScreenWidth, ScreenHeight);

	// 4. Dessine le statut de mission (Credits, Quota, Extraction)
	DrawMissionStatus(Scavenger, ScreenWidth, ScreenHeight);

	// 5. Effets diegetiques de folie / Sanite 2.0 (Glitches, fausses alertes)
	DrawSanityGlitches(Scavenger, ScreenWidth, ScreenHeight);

	// 5b. Feedback de degats rouge sang (Game Feel)
	if (Scavenger->GetDamageFlashIntensity() > 0.005f)
	{
		DrawDamageVignette(Scavenger->GetDamageFlashIntensity(), ScreenWidth, ScreenHeight);
	}

	// 5c. Alerte d'etat K.O. et equipiers a terre (Coop Survival)
	DrawDownedIndicator(Scavenger, ScreenWidth, ScreenHeight);

	// 6. Camera found-footage VHS 1998 / OSD Camescope (Escape Together)
	if (bShowVHSOverlay)
	{
		DrawVHSBodycamOSD(Scavenger, ScreenWidth, ScreenHeight);
	}

	// 7. Notification d'encaissement de butin fluide dans le sac (Toast)
	if (LootNotificationTimer > 0.0f)
	{
		LootNotificationTimer -= DeltaSeconds;
		const float Alpha = FMath::Clamp(LootNotificationTimer / 0.4f, 0.0f, 1.0f);
		const float BannerWidth = 340.0f;
		const float BannerHeight = 28.0f;
		const float BannerX = CenterX - (BannerWidth * 0.5f);
		const float BannerY = CenterY - 60.0f;

		DrawRect(FLinearColor(0.02f, 0.12f, 0.04f, 0.85f * Alpha), BannerX, BannerY, BannerWidth, BannerHeight);
		Canvas->K2_DrawLine(FVector2D(BannerX, BannerY + BannerHeight), FVector2D(BannerX + BannerWidth, BannerY + BannerHeight), 1.5f, FLinearColor(0.1f, 1.0f, 0.4f, Alpha));

		FCanvasTextItem Toast(FVector2D(BannerX + 10.0f, BannerY + 6.0f),
			FText::FromString(LootNotificationText), GEngine->GetSmallFont(), FLinearColor(0.3f, 1.0f, 0.5f, Alpha));
		Toast.EnableShadow(FLinearColor(0.0f, 0.0f, 0.0f, Alpha));
		Canvas->DrawItem(Toast);
	}
}

void ALiminalScavengerHUD::ShowLootPickupNotification(int32 Credits, float WeightKg)
{
	LootNotificationTimer = 2.5f;
	LootNotificationText = FString::Printf(TEXT("+%d CR  RECOLTES DANS LE SAC  (Poids: +%.1f kg)"), Credits, WeightKg);
}

void ALiminalScavengerHUD::DrawReticle(AScavengerCharacter* Scavenger, float CenterX, float CenterY)
{
	const float ReticleSize = 4.0f;
	FLinearColor ReticleColor = FLinearColor(0.8f, 0.85f, 0.9f, 0.75f);

	ALootActor* Loot = Scavenger->GetHighlightedLoot();
	if (Loot)
	{
		ReticleColor = FLinearColor(0.1f, 1.0f, 0.4f, 1.0f); // Vert interactif

		const FString PromptText = FString::Printf(TEXT("[E] RAMASSER DANS LE SAC (+%d CR  |  %.1f kg)"),
			Loot->GetCreditsValue(), Loot->GetWeightKg());
		
		FCanvasTextItem PromptItem(FVector2D(CenterX - 105.0f, CenterY + 20.0f),
			FText::FromString(PromptText), GEngine->GetSmallFont(), FLinearColor(0.3f, 1.0f, 0.6f));
		PromptItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(PromptItem);
	}
	else if (Scavenger->GetHeldLoot())
	{
		ReticleColor = FLinearColor(1.0f, 0.8f, 0.1f, 1.0f); // Ambre maintien
		FCanvasTextItem HoldItem(FVector2D(CenterX - 110.0f, CenterY + 20.0f),
			FText::FromString(TEXT("[X] Deposer au sol  |  [R / Clic-Droit] Lancer")), GEngine->GetSmallFont(), FLinearColor::Yellow);
		HoldItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(HoldItem);
	}
	else
	{
		if (UWorld* World = GetWorld())
		{
			FHitResult Hit;
			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(Scavenger);
			const FVector CamLoc = Scavenger->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
			const FVector CamForward = Scavenger->GetControlRotation().Vector();
			if (World->LineTraceSingleByChannel(Hit, CamLoc, CamLoc + CamForward * 320.0f, ECC_Visibility, QueryParams))
			{
				if (AActor* TargetActor = Hit.GetActor())
				{
					if (TargetActor->IsA(ALiminalTerminalActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.2f, 0.8f, 1.0f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 95.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Terminal M.E.G. (Missions & Magasin)")), GEngine->GetSmallFont(), FLinearColor(0.0f, 1.0f, 1.0f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalAirlockActor::StaticClass()))
					{
						ReticleColor = FLinearColor(1.0f, 0.7f, 0.2f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 75.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Sas d'Incursion M.E.G. (Depart)")), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.85f, 0.3f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalDoorActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.3f, 0.9f, 0.5f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 70.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Ouvrir / Fermer la Porte")), GEngine->GetSmallFont(), FLinearColor(0.4f, 1.0f, 0.6f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalHidingSpot::StaticClass()))
					{
						ReticleColor = FLinearColor(0.2f, 0.7f, 1.0f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 80.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Se Cacher dans le Casier")), GEngine->GetSmallFont(), FLinearColor(0.3f, 0.9f, 1.0f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalVentActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.8f, 0.8f, 0.3f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 85.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Ramper dans le Conduit")), GEngine->GetSmallFont(), FLinearColor(0.9f, 0.9f, 0.4f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalKeypadActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.9f, 0.3f, 1.0f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 85.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Clavier de Securite [Code 4 Chiffres]")), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.5f, 1.0f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalBreakerActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.2f, 1.0f, 0.7f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 85.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Disjoncteur Mural [Basculer Alimentation]")), GEngine->GetSmallFont(), FLinearColor(0.4f, 1.0f, 0.8f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalValvePuzzleActor::StaticClass()))
					{
						ReticleColor = FLinearColor(1.0f, 0.5f, 0.2f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 75.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Tourner la Vanne a Pression")), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.6f, 0.3f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalFuseBoxActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.9f, 0.9f, 0.2f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 85.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Boitier de Fusibles [Manipuler]")), GEngine->GetSmallFont(), FLinearColor(1.0f, 1.0f, 0.3f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
					else if (TargetActor->IsA(ALiminalKeyItemActor::StaticClass()))
					{
						ReticleColor = FLinearColor(0.3f, 1.0f, 0.8f, 1.0f);
						FCanvasTextItem PromptItem(FVector2D(CenterX - 70.0f, CenterY + 20.0f),
							FText::FromString(TEXT("[E] Ramasser le Badge d'Acces")), GEngine->GetSmallFont(), FLinearColor(0.4f, 1.0f, 0.8f));
						PromptItem.EnableShadow(FLinearColor::Black);
						Canvas->DrawItem(PromptItem);
					}
				}
			}
		}
	}

	// Croix centrale discrete
	Canvas->K2_DrawLine(FVector2D(CenterX - ReticleSize, CenterY), FVector2D(CenterX + ReticleSize, CenterY), 1.5f, ReticleColor);
	Canvas->K2_DrawLine(FVector2D(CenterX, CenterY - ReticleSize), FVector2D(CenterX, CenterY + ReticleSize), 1.5f, ReticleColor);
}

void ALiminalScavengerHUD::DrawSurvivalGauges(AScavengerCharacter* Scavenger, float CenterX, float CenterY)
{
	const float ScreenHeight = Canvas->ClipY;

	// --- BARRE D'ENDURANCE (Bas-Centre) ---
	const float StaminaPercent = Scavenger->GetStaminaPercent();
	const float StaminaWidth = 240.0f;
	const float StaminaHeight = 8.0f;
	const float StaminaX = CenterX - (StaminaWidth * 0.5f);
	const float StaminaY = ScreenHeight - 55.0f;

	FLinearColor StaminaColor = (StaminaPercent > 0.35f) ?
		FLinearColor(0.2f, 0.8f, 1.0f, 0.85f) : FLinearColor(1.0f, 0.3f, 0.1f, 0.95f);

	DrawProgressBar(StaminaX, StaminaY, StaminaWidth, StaminaHeight, StaminaPercent,
		StaminaColor, FLinearColor(0.05f, 0.08f, 0.12f, 0.6f), TEXT("VIGUEUR"));

	// --- MONITEUR VITAL & SANITE (Bas-Gauche) ---
	const float GaugeLeftX = 40.0f;
	const float HealthY = ScreenHeight - 95.0f;
	const float SanityY = ScreenHeight - 65.0f;
	const float GaugeWidth = 180.0f;
	const float GaugeHeight = 10.0f;

	const float HealthPercent = Scavenger->GetHealthPercent();
	FLinearColor HealthColor = (HealthPercent > 0.4f) ?
		FLinearColor(0.15f, 0.9f, 0.3f, 0.9f) : FLinearColor(0.95f, 0.15f, 0.15f, 0.95f);
	DrawProgressBar(GaugeLeftX, HealthY, GaugeWidth, GaugeHeight, HealthPercent,
		HealthColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.6f), TEXT("INTEGRITE"));

	const float SanityPercent = Scavenger->GetSanityPercent();
	ESanityTier Tier = LiminalSanity::GetTierFromPercent(SanityPercent);

	FLinearColor SanityColor = FLinearColor(0.2f, 0.7f, 1.0f, 0.9f);
	FString TierName = TEXT("STABLE");
	if (Tier == ESanityTier::Uneasy)
	{
		SanityColor = FLinearColor(1.0f, 0.85f, 0.2f, 0.9f);
		TierName = TEXT("INQUIET");
	}
	else if (Tier == ESanityTier::Paranoid)
	{
		SanityColor = FLinearColor(1.0f, 0.5f, 0.1f, 0.95f);
		TierName = TEXT("PARANOIAQUE");
	}
	else if (Tier == ESanityTier::Psychotic)
	{
		SanityColor = FLinearColor(0.95f, 0.1f, 0.3f, 1.0f);
		TierName = TEXT("PSYCHOTIQUE");
	}

	DrawProgressBar(GaugeLeftX, SanityY, GaugeWidth, GaugeHeight, SanityPercent,
		SanityColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.6f), FString::Printf(TEXT("SANITE [%s]"), *TierName));

	// --- JAUGE DE CHARGE / POIDS ---
	const float WeightRatio = Scavenger->GetWeightRatio();
	const float CurWeight = Scavenger->GetCurrentInventoryWeightKg();
	const float MaxWeight = Scavenger->GetMaxCarryWeightKg();
	const float WeightY = ScreenHeight - 35.0f;

	FLinearColor WeightColor = (WeightRatio < 0.7f) ?
		FLinearColor(0.7f, 0.8f, 0.85f, 0.8f) : FLinearColor(1.0f, 0.4f, 0.1f, 0.95f);

	DrawProgressBar(GaugeLeftX, WeightY, GaugeWidth, GaugeHeight, WeightRatio,
		WeightColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.6f),
		FString::Printf(TEXT("CHARGE : %.1f / %.0f kg"), CurWeight, MaxWeight));
}

void ALiminalScavengerHUD::DrawToolStatus(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight)
{
	const float PanelWidth = 240.0f;
	const float PanelHeight = 70.0f;
	const float PanelX = ScreenWidth - PanelWidth - 40.0f;
	const float PanelY = ScreenHeight - PanelHeight - 35.0f;

	// Fond de panneau semi-transparent
	Canvas->K2_DrawBox(FVector2D(PanelX, PanelY), FVector2D(PanelWidth, PanelHeight), 1.0f, FLinearColor(0.0f, 0.8f, 0.6f, 0.4f));

	const FName ToolName = Scavenger->GetCurrentToolName();
	const float Battery = Scavenger->GetToolBatteryCharge();

	FCanvasTextItem ToolItem(FVector2D(PanelX + 10.0f, PanelY + 8.0f),
		FText::FromString(FString::Printf(TEXT("EQUIPEMENT : %s"), *ToolName.ToString())),
		GEngine->GetSmallFont(), FLinearColor::White);
	ToolItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(ToolItem);

	// Barre de batterie de l'outil
	const float BatteryBarWidth = PanelWidth - 20.0f;
	DrawProgressBar(PanelX + 10.0f, PanelY + 30.0f, BatteryBarWidth, 8.0f, Battery,
		FLinearColor(0.1f, 0.9f, 0.7f, 0.9f), FLinearColor(0.08f, 0.1f, 0.1f, 0.7f),
		FString::Printf(TEXT("BATTERIE : %.0f%%"), Battery * 100.0f));

	FCanvasTextItem HintItem(FVector2D(PanelX + 10.0f, PanelY + 48.0f),
		FText::FromString(TEXT("[Molette] Changer | [Clic G] Activer | [G] Poser")),
		GEngine->GetSmallFont(), FLinearColor(0.7f, 0.7f, 0.7f, 0.7f));
	HintItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(HintItem);
}

void ALiminalScavengerHUD::DrawMissionStatus(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight)
{
	const float PanelX = ScreenWidth - 300.0f;
	const float PanelY = 30.0f;

	FCanvasTextItem HeaderItem(FVector2D(PanelX, PanelY),
		FText::FromString(TEXT("== TRANSMISSION M.E.G. ==")), GEngine->GetSmallFont(),
		FLinearColor(0.3f, 0.9f, 1.0f, 0.9f));
	HeaderItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(HeaderItem);

	const int32 Credits = Scavenger->GetCarriedCredits();
	FCanvasTextItem CreditItem(FVector2D(PanelX, PanelY + 18.0f),
		FText::FromString(FString::Printf(TEXT("Butin secouru : %d Credits"), Credits)),
		GEngine->GetSmallFont(), FLinearColor::White);
	CreditItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(CreditItem);

	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance()))
	{
		FCanvasTextItem BankItem(FVector2D(PanelX, PanelY + 34.0f),
			FText::FromString(FString::Printf(TEXT("Solde Banque : %d Credits"), GI->GetTotalCredits())),
			GEngine->GetSmallFont(), FLinearColor(0.7f, 0.95f, 0.7f));
		BankItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(BankItem);
	}

	float CurrentOffsetY = PanelY + 52.0f;

	ALiminalGameMode* GM = Cast<ALiminalGameMode>(GetWorld() ? GetWorld()->GetAuthGameMode() : nullptr);
	if (!GM)
	{
		// Nous sommes dans la Base Alpha (Lobby) : Affichage d'un panneau d'instructions clair
		const float CenterX = ScreenWidth * 0.5f;
		const float BannerWidth = 560.0f;
		const float BannerHeight = 85.0f;
		const float BannerX = CenterX - BannerWidth * 0.5f;
		const float BannerY = 22.0f;

		Canvas->K2_DrawBox(FVector2D(BannerX, BannerY), FVector2D(BannerWidth, BannerHeight), 1.0f, FLinearColor(0.2f, 0.7f, 1.0f, 0.4f));

		FCanvasTextItem TitleItem(FVector2D(BannerX + 16.0f, BannerY + 8.0f),
			FText::FromString(TEXT("POSTE AVANCE M.E.G. - BASE ALPHA")),
			GEngine->GetMediumFont(), FLinearColor(0.3f, 0.9f, 1.0f));
		TitleItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TitleItem);

		FCanvasTextItem Step1(FVector2D(BannerX + 16.0f, BannerY + 34.0f),
			FText::FromString(TEXT("[1] Terminal [E] : Choisir le biome d'incursion et acheter des outils")),
			GEngine->GetSmallFont(), FLinearColor::White);
		Step1.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(Step1);

		FCanvasTextItem Step2(FVector2D(BannerX + 16.0f, BannerY + 54.0f),
			FText::FromString(TEXT("[2] Sas Motorise [E] : Activer le levier pour demarrer l'expedition")),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.85f, 0.2f));
		Step2.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(Step2);
	}
	else
	{
		// En mission dans les Backrooms : Rappel d'objectif d'incursion
		const float CenterX = ScreenWidth * 0.5f;
		const float BannerWidth = 500.0f;
		const float BannerX = CenterX - BannerWidth * 0.5f;

		FCanvasTextItem MissionObj(FVector2D(BannerX, 15.0f),
			FText::FromString(TEXT("OBJECTIF : RECOLTEZ DU BUTIN ET EVACUEZ AVANT L'EFFONDREMENT")),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.85f, 0.2f));
		MissionObj.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(MissionObj);

		const float Remaining = GM->GetRealityCollapseRemainingSeconds();
		const int32 Mins = FMath::FloorToInt(Remaining / 60.0f);
		const int32 Secs = FMath::FloorToInt(FMath::Fmod(Remaining, 60.0f));

		FLinearColor TimerColor = FLinearColor(0.2f, 1.0f, 0.4f);
		if (Remaining <= 60.0f)
		{
			TimerColor = (FMath::Fmod(GlitchTimer, 2.0f) > 1.0f) ? FLinearColor::Red : FLinearColor::Yellow;
		}
		else if (Remaining <= 180.0f)
		{
			TimerColor = FLinearColor::Yellow;
		}

		FCanvasTextItem CollapseItem(FVector2D(PanelX, CurrentOffsetY),
			FText::FromString(FString::Printf(TEXT("STABILITE : %02d:%02d"), Mins, Secs)),
			GEngine->GetSmallFont(), TimerColor);
		CollapseItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(CollapseItem);
		CurrentOffsetY += 18.0f;

		if (GM->GetMatchState() == EExtractionMatchState::ExtractionPending)
		{
			FCanvasTextItem ExtractingItem(FVector2D(PanelX, CurrentOffsetY),
				FText::FromString(TEXT("EXTRACTION EN COURS...")),
				GEngine->GetSmallFont(), FLinearColor::Green);
			ExtractingItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(ExtractingItem);
			CurrentOffsetY += 18.0f;
		}
		else if (GM->GetMatchState() == EExtractionMatchState::SquadWiped)
		{
			FCanvasTextItem WipedItem(FVector2D(PanelX, CurrentOffsetY),
				FText::FromString(TEXT("ESCOUADE PERDUE... EVACUATION")),
				GEngine->GetSmallFont(), FLinearColor::Red);
			WipedItem.EnableShadow(FLinearColor::Black);
			Canvas->DrawItem(WipedItem);
			CurrentOffsetY += 18.0f;
		}
	}

	if (Scavenger->IsInfectedPartygoer())
	{
		FCanvasTextItem InfectItem(FVector2D(PanelX, CurrentOffsetY),
			FText::FromString(TEXT("! INFECTION SOCIALE DETECTEE =)")),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.2f, 0.2f, 1.0f));
		InfectItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(InfectItem);
	}
	else if (Scavenger->IsInsideRealityAnchor())
	{
		FCanvasTextItem AnchorItem(FVector2D(PanelX, CurrentOffsetY),
			FText::FromString(TEXT("Champ d'Ancre de Realite Actif")),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.85f, 0.2f, 0.9f));
		AnchorItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(AnchorItem);
	}
}

void ALiminalScavengerHUD::DrawSanityGlitches(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight)
{
	const float Sanity = Scavenger->GetSanityPercent();
	if (Sanity >= 0.5f)
	{
		return;
	}

	// Lignes de balayage CRT glitchées proportionnelles au déficit
	const float Deficit = 1.0f - (Sanity / 0.5f);
	if (FMath::FRand() < Deficit * 0.4f)
	{
		const float LineY = FMath::FRandRange(0.0f, ScreenHeight);
		Canvas->K2_DrawLine(FVector2D(0.0f, LineY), FVector2D(ScreenWidth, LineY), 2.0f,
			FLinearColor(1.0f, 0.2f, 0.2f, 0.4f * Deficit));
	}

	// Alertes corrompues en phase psychotique
	if (Sanity < 0.25f)
	{
		const TCHAR* GlitchPhrases[] = {
			TEXT("NE REGARDE PAS DERRIERE TOI"),
			TEXT("ILS ENTENDENT TES PAS"),
			TEXT("CE COULOIR N'A PAS DE FIN"),
			TEXT("REJOINS LA FETE =)"),
			TEXT("SIGNAL CORROMPU - FUYEZ")
		};
		const int32 Index = FMath::Abs(FMath::FloorToInt(GlitchTimer)) % 5;
		FCanvasTextItem AlertItem(FVector2D(ScreenWidth * 0.35f, 80.0f),
			FText::FromString(GlitchPhrases[Index]), GEngine->GetMediumFont(),
			FLinearColor(1.0f, 0.1f, 0.1f, 0.85f));
		AlertItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(AlertItem);
	}

	// Fausses alertes systeme diegetiques (Doute paranoïaque M.E.G.)
	const FString FakeAlert = Scavenger->GetActiveFakeAlert();
	if (!FakeAlert.IsEmpty())
	{
		const float BannerWidth = 480.0f;
		const float BannerHeight = 32.0f;
		const float BannerX = (ScreenWidth - BannerWidth) * 0.5f;
		const float BannerY = 55.0f;

		DrawRect(FLinearColor(0.2f, 0.02f, 0.02f, 0.9f), BannerX, BannerY, BannerWidth, BannerHeight);
		Canvas->K2_DrawBox(FVector2D(BannerX, BannerY), FVector2D(BannerWidth, BannerHeight), 1.5f, FLinearColor(1.0f, 0.2f, 0.2f, 0.95f));

		FCanvasTextItem GlitchItem(FVector2D(BannerX + 15.0f, BannerY + 8.0f),
			FText::FromString(FakeAlert), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.35f, 0.35f, 0.95f));
		GlitchItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(GlitchItem);
	}
}

void ALiminalScavengerHUD::DrawProgressBar(float X, float Y, float Width, float Height, float Percent,
	const FLinearColor& FillColor, const FLinearColor& BackColor, const FString& Label)
{
	Percent = FMath::Clamp(Percent, 0.0f, 1.0f);

	// Fond
	Canvas->K2_DrawBox(FVector2D(X, Y), FVector2D(Width, Height), 1.0f, FLinearColor(BackColor.R, BackColor.G, BackColor.B, 0.8f));

	// Remplissage
	if (Percent > 0.001f)
	{
		Canvas->K2_DrawLine(FVector2D(X + 1.0f, Y + (Height * 0.5f)),
			FVector2D(X + (Width * Percent) - 1.0f, Y + (Height * 0.5f)),
			Height - 2.0f, FillColor);
	}

	// Libelle
	if (!Label.IsEmpty())
	{
		FCanvasTextItem TextItem(FVector2D(X + 4.0f, Y - 14.0f),
			FText::FromString(Label), GEngine->GetSmallFont(), FLinearColor(0.85f, 0.9f, 0.95f, 0.85f));
		TextItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TextItem);
	}
}

void ALiminalScavengerHUD::DrawSpectatorHUD(ALiminalSpectatorPawn* Spectator, float ScreenWidth, float ScreenHeight)
{
	if (!Canvas || !Spectator)
	{
		return;
	}

	const float CenterX = ScreenWidth * 0.5f;

	// 1. Cadre CRT et bandeau supérieur de surveillance M.E.G.
	const float AlphaPulse = 0.85f + 0.15f * FMath::Sin(GlitchTimer * 2.0f);
	FCanvasTextItem HeaderItem(FVector2D(40.0f, 35.0f),
		FText::FromString(TEXT("● REC // M.E.G. CCTV SURVEILLANCE FEED [TÉLÉMÉTRIE SATELLITE CO-OP]")),
		GEngine->GetMediumFont(), FLinearColor(1.0f, 0.25f, 0.25f, AlphaPulse));
	HeaderItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(HeaderItem);

	Canvas->K2_DrawLine(FVector2D(40.0f, 65.0f), FVector2D(ScreenWidth - 40.0f, 65.0f), 1.5f, FLinearColor(0.8f, 0.15f, 0.15f, 0.5f));

	AScavengerCharacter* Target = Spectator->GetTargetSpectatedPlayer();
	if (Target && !Target->IsDead())
	{
		// 2. Télémétrie vitale de l'opérateur suivi
		const FString OpName = FString::Printf(TEXT("SUJET SUIVI : %s"), *Target->GetName());
		FCanvasTextItem TargetNameItem(FVector2D(40.0f, 85.0f),
			FText::FromString(OpName), GEngine->GetMediumFont(), FLinearColor(0.2f, 0.85f, 1.0f, 0.95f));
		TargetNameItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(TargetNameItem);

		const float HealthPercent = Target->GetHealthPercent();
		const float SanityPercent = Target->GetSanityPercent();
		const float StaminaPercent = Target->GetStaminaPercent();
		const float GaugeWidth = 260.0f;
		const float GaugeHeight = 12.0f;

		FLinearColor HealthColor = (HealthPercent > 0.4f) ?
			FLinearColor(0.2f, 0.9f, 0.3f, 0.9f) : FLinearColor(0.95f, 0.15f, 0.15f, 0.95f);
		DrawProgressBar(40.0f, 135.0f, GaugeWidth, GaugeHeight, HealthPercent,
			HealthColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.6f),
			FString::Printf(TEXT("INTÉGRITÉ SUJET : %.0f%%"), HealthPercent * 100.0f));

		ESanityTier Tier = LiminalSanity::GetTierFromPercent(SanityPercent);
		FLinearColor SanityColor = (Tier == ESanityTier::Stable) ? FLinearColor(0.2f, 0.7f, 1.0f) :
			(Tier == ESanityTier::Uneasy) ? FLinearColor(1.0f, 0.85f, 0.2f) :
			(Tier == ESanityTier::Paranoid) ? FLinearColor(1.0f, 0.5f, 0.1f) : FLinearColor(0.95f, 0.1f, 0.3f);
		DrawProgressBar(40.0f, 175.0f, GaugeWidth, GaugeHeight, SanityPercent,
			SanityColor, FLinearColor(0.1f, 0.1f, 0.1f, 0.6f),
			FString::Printf(TEXT("SANIGRAMME : %.0f%%"), SanityPercent * 100.0f));

		DrawProgressBar(40.0f, 215.0f, GaugeWidth, GaugeHeight, StaminaPercent,
			FLinearColor(0.2f, 0.8f, 1.0f, 0.85f), FLinearColor(0.1f, 0.1f, 0.1f, 0.6f),
			FString::Printf(TEXT("VIGUEUR : %.0f%%"), StaminaPercent * 100.0f));

		// Données de chargement et outil
		const FString CargoInfo = FString::Printf(
			TEXT("CHARGEMENT : %.1f kg  |  BUTIN TRANSPORTÉ : %d CR  |  OUTIL ÉQUIPÉ : %s"),
			Target->GetCurrentInventoryWeightKg(),
			Target->GetCarriedCredits(),
			*Target->GetCurrentToolName().ToString());
		FCanvasTextItem CargoItem(FVector2D(40.0f, 245.0f),
			FText::FromString(CargoInfo), GEngine->GetSmallFont(), FLinearColor(0.9f, 0.85f, 0.4f, 0.9f));
		CargoItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(CargoItem);
	}
	else
	{
		// Mode vol libre ou attente de coéquipiers
		FCanvasTextItem FreeCamItem(FVector2D(CenterX - 180.0f, ScreenHeight * 0.45f),
			FText::FromString(TEXT("[MODE OBSERVATEUR LIBRE // AUCUN SUJET ACTIF]")),
			GEngine->GetMediumFont(), FLinearColor(1.0f, 0.8f, 0.2f, 0.8f));
		FreeCamItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(FreeCamItem);
	}

	// 3. Bas de page : Guide des commandes spectateur
	Canvas->K2_DrawLine(FVector2D(40.0f, ScreenHeight - 55.0f), FVector2D(ScreenWidth - 40.0f, ScreenHeight - 55.0f), 1.0f, FLinearColor(0.4f, 0.4f, 0.4f, 0.5f));
	FCanvasTextItem FooterItem(FVector2D(40.0f, ScreenHeight - 42.0f),
		FText::FromString(TEXT("[CLIC GAUCHE / TAB] SUIVANT  |  [CLIC DROIT] PRÉCÉDENT  |  [ESPACE] BASCULER VOL LIBRE")),
		GEngine->GetSmallFont(), FLinearColor(0.7f, 0.75f, 0.8f, 0.8f));
	FooterItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(FooterItem);
}

void ALiminalScavengerHUD::DrawFieldManual(float ScreenWidth, float ScreenHeight)
{
	if (!Canvas)
	{
		return;
	}

	// 1. Fond sombre et bordure CRT
	DrawRect(FLinearColor(0.01f, 0.025f, 0.015f, 0.96f), 0.0f, 0.0f, ScreenWidth, ScreenHeight);

	// Lignes de balayage CRT
	for (float Y = 0.0f; Y < ScreenHeight; Y += 6.0f)
	{
		Canvas->K2_DrawLine(FVector2D(0.0f, Y), FVector2D(ScreenWidth, Y), 1.0f, FLinearColor(0.0f, 0.1f, 0.04f, 0.25f));
	}

	// 2. En-tête M.E.G.
	DrawRect(FLinearColor(0.04f, 0.2f, 0.08f, 0.9f), 30.0f, 20.0f, ScreenWidth - 60.0f, 45.0f);
	FCanvasTextItem TitleItem(FVector2D(45.0f, 26.0f),
		FText::FromString(TEXT("=== M.E.G. DOSSIER TACTIQUE & MANUEL DE TERRAIN [NIVEAU SÉCURITÉ-7] ===")),
		GEngine->GetMediumFont(), FLinearColor(0.2f, 1.0f, 0.5f, 1.0f));
	TitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TitleItem);

	FCanvasTextItem SubtitleItem(FVector2D(45.0f, 48.0f),
		FText::FromString(TEXT("DOCUMENTATION DE SURVIE & CONTRE-MESURES DES ENTITÉS CONNUES")),
		GEngine->GetSmallFont(), FLinearColor(0.6f, 0.9f, 0.7f, 0.85f));
	Canvas->DrawItem(SubtitleItem);

	// 3. Colonnes d'entités
	const float ColWidth = (ScreenWidth - 90.0f) * 0.5f;
	const float Col1X = 40.0f;
	const float Col2X = Col1X + ColWidth + 20.0f;
	const float StartY = 80.0f;
	const float RowStep = 55.0f;

	// Colonne Gauche (Entités 1 à 5)
	struct FEntityDoc { const TCHAR* Name; const TCHAR* Danger; const TCHAR* Counter; };
	const FEntityDoc LeftCol[] = {
		{ TEXT("SMILER [Entite 2]"), TEXT("DANGER : S+"), TEXT("Photophobe agressif. Aveugler avec Flash Strobe (140cd). Ne pas lui tourner le dos.") },
		{ TEXT("JERRY [Entite 7]"), TEXT("DANGER : A"), TEXT("Oiseau culte & hypnose. Perturber au Micro-ondes Sonique. Rompre la ligne de vue.") },
		{ TEXT("WATCHER [Entite 11]"), TEXT("DANGER : B"), TEXT("Oeil spectral observateur. Contact visuel alerte la ruche. Rester a couvert.") },
		{ TEXT("PARTYGOER [Entite 67]"), TEXT("DANGER : S"), TEXT("Embuscade & faux signaux. Ne JAMAIS toucher aux ballons rouges. Lancer un leurre.") },
		{ TEXT("SKINWALKER [Entite 10]"), TEXT("DANGER : A"), TEXT("Usurpateur vocal. Verifier le GPS d'escouade. Lampe frontale irreguliere.") }
	};

	// Colonne Droite (Entités 6 à 10)
	const FEntityDoc RightCol[] = {
		{ TEXT("DEATHMOTH [Entite 8]"), TEXT("DANGER : A"), TEXT("Projection acide & vol. Fuir les salles hautes. Repulsif Eau d'Amande efficace.") },
		{ TEXT("CLUMP [Entite 4]"), TEXT("DANGER : B"), TEXT("Amas tentaculaire au sol. Detecte les pas de course. S'accroupir ou detourner.") },
		{ TEXT("WRETCH [Entite 15]"), TEXT("DANGER : C"), TEXT("Explorateur dechu affame. Garder la distance. Vulnerable aux ondes micro-ondes.") },
		{ TEXT("DULLER [Entite 3]"), TEXT("DANGER : A"), TEXT("Silhouette occulte invisible a l'oeil nu. Reveler au Scanner LIDAR.") },
		{ TEXT("HOUND [Entite 1]"), TEXT("DANGER : B"), TEXT("Traqueur quadrupede. Ne pas courir si coince : maintenir le regard et reculer.") }
	};

	for (int32 i = 0; i < 5; ++i)
	{
		// Boîte de gauche
		DrawRect(FLinearColor(0.02f, 0.06f, 0.03f, 0.8f), Col1X, StartY + (i * RowStep), ColWidth, RowStep - 5.0f);
		FCanvasTextItem Name1(FVector2D(Col1X + 10.0f, StartY + (i * RowStep) + 4.0f),
			FText::FromString(FString::Printf(TEXT("%s  -  %s"), LeftCol[i].Name, LeftCol[i].Danger)),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.4f, 0.2f));
		Name1.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(Name1);
		FCanvasTextItem Desc1(FVector2D(Col1X + 10.0f, StartY + (i * RowStep) + 24.0f),
			FText::FromString(LeftCol[i].Counter), GEngine->GetSmallFont(), FLinearColor(0.85f, 0.9f, 0.85f));
		Canvas->DrawItem(Desc1);

		// Boîte de droite
		DrawRect(FLinearColor(0.02f, 0.06f, 0.03f, 0.8f), Col2X, StartY + (i * RowStep), ColWidth, RowStep - 5.0f);
		FCanvasTextItem Name2(FVector2D(Col2X + 10.0f, StartY + (i * RowStep) + 4.0f),
			FText::FromString(FString::Printf(TEXT("%s  -  %s"), RightCol[i].Name, RightCol[i].Danger)),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.4f, 0.2f));
		Name2.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(Name2);
		FCanvasTextItem Desc2(FVector2D(Col2X + 10.0f, StartY + (i * RowStep) + 24.0f),
			FText::FromString(RightCol[i].Counter), GEngine->GetSmallFont(), FLinearColor(0.85f, 0.9f, 0.85f));
		Canvas->DrawItem(Desc2);
	}

	// 4. Directives de survie
	const float DirectivesY = StartY + (5 * RowStep) + 10.0f;
	DrawRect(FLinearColor(0.03f, 0.1f, 0.05f, 0.85f), 40.0f, DirectivesY, ScreenWidth - 80.0f, 75.0f);
	FCanvasTextItem DirTitle(FVector2D(50.0f, DirectivesY + 6.0f),
		FText::FromString(TEXT("DIRECTIVES DE SURVIE ESSENTIELLES :")),
		GEngine->GetSmallFont(), FLinearColor(0.3f, 1.0f, 0.6f));
	DirTitle.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(DirTitle);

	const FString DirText = TEXT("- SANITÉ : Boire de l'Eau d'Amande des le palier 'Inquiet'. Sous 25% (Psychotique), des hallucinations vous tromperont.\n- CHARGE : Max 60 kg. Au-dela de 25 kg, l'endurance draine 2x plus vite et votre essoufflement alerte les monstres.\n- ORIENTATION : Utilisez la Craie Phosphorescente [AChalkMarkerTool] pour baliser les carrefours vers le sas.");
	FCanvasTextItem DirDesc(FVector2D(50.0f, DirectivesY + 24.0f),
		FText::FromString(DirText), GEngine->GetSmallFont(), FLinearColor(0.9f, 0.95f, 0.9f));
	Canvas->DrawItem(DirDesc);

	// 5. Bas de page
	Canvas->K2_DrawLine(FVector2D(40.0f, ScreenHeight - 45.0f), FVector2D(ScreenWidth - 40.0f, ScreenHeight - 45.0f), 1.0f, FLinearColor(0.2f, 0.8f, 0.4f, 0.5f));
	FCanvasTextItem Footer(FVector2D(40.0f, ScreenHeight - 34.0f),
		FText::FromString(TEXT("[M] ou [J] : FERMER LE MANUEL  |  M.E.G. Division d'Extraction & Securite des Limites")),
		GEngine->GetSmallFont(), FLinearColor(0.5f, 0.9f, 0.6f));
	Footer.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(Footer);
}

void ALiminalScavengerHUD::DrawVHSBodycamOSD(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight)
{
	if (!Canvas)
	{
		return;
	}

	const float TimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	const float Margin = 35.0f;

	// 1. Coins du viseur Camescope (Corner Brackets)
	const float BracketLen = 22.0f;
	const float BracketThick = 1.5f;
	const FLinearColor BracketColor = FLinearColor(0.85f, 0.9f, 0.85f, 0.45f);

	// Haut-Gauche
	Canvas->K2_DrawLine(FVector2D(Margin, Margin), FVector2D(Margin + BracketLen, Margin), BracketThick, BracketColor);
	Canvas->K2_DrawLine(FVector2D(Margin, Margin), FVector2D(Margin, Margin + BracketLen), BracketThick, BracketColor);
	// Haut-Droite
	Canvas->K2_DrawLine(FVector2D(ScreenWidth - Margin, Margin), FVector2D(ScreenWidth - Margin - BracketLen, Margin), BracketThick, BracketColor);
	Canvas->K2_DrawLine(FVector2D(ScreenWidth - Margin, Margin), FVector2D(ScreenWidth - Margin, Margin + BracketLen), BracketThick, BracketColor);
	// Bas-Gauche
	Canvas->K2_DrawLine(FVector2D(Margin, ScreenHeight - Margin), FVector2D(Margin + BracketLen, ScreenHeight - Margin), BracketThick, BracketColor);
	Canvas->K2_DrawLine(FVector2D(Margin, ScreenHeight - Margin), FVector2D(Margin, ScreenHeight - Margin - BracketLen), BracketThick, BracketColor);
	// Bas-Droite
	Canvas->K2_DrawLine(FVector2D(ScreenWidth - Margin, ScreenHeight - Margin), FVector2D(ScreenWidth - Margin - BracketLen, ScreenHeight - Margin), BracketThick, BracketColor);
	Canvas->K2_DrawLine(FVector2D(ScreenWidth - Margin, ScreenHeight - Margin), FVector2D(ScreenWidth - Margin, ScreenHeight - Margin - BracketLen), BracketThick, BracketColor);

	// 2. Haut-Gauche : Clignotement REC rouge + Batterie + SP PLAY
	const bool bRecBlink = FMath::Fmod(TimeSeconds, 1.2f) < 0.75f;
	if (bRecBlink)
	{
		DrawRect(FLinearColor(0.95f, 0.08f, 0.08f, 0.95f), Margin + 5.0f, Margin + 6.0f, 10.0f, 10.0f);
	}
	FCanvasTextItem RecText(FVector2D(Margin + 20.0f, Margin + 4.0f),
		FText::FromString(bRecBlink ? TEXT("REC") : TEXT("STBY")),
		GEngine->GetSmallFont(),
		bRecBlink ? FLinearColor(0.95f, 0.2f, 0.2f, 0.95f) : FLinearColor(0.6f, 0.6f, 0.6f, 0.6f));
	RecText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(RecText);

	FCanvasTextItem PlayText(FVector2D(Margin + 65.0f, Margin + 4.0f),
		FText::FromString(TEXT("SP  PLAY \u25ba")),
		GEngine->GetSmallFont(), FLinearColor(0.85f, 0.95f, 0.85f, 0.85f));
	PlayText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(PlayText);

	// Batterie
	FCanvasTextItem BattText(FVector2D(Margin + 5.0f, Margin + 20.0f),
		FText::FromString(TEXT("BATT [\u25a0\u25a0\u25a0\u25a1] 78%")),
		GEngine->GetSmallFont(), FLinearColor(0.8f, 0.9f, 0.8f, 0.75f));
	BattText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(BattText);

	// 3. Haut-Droite : Identifiant Camera M.E.G. & Frequence Radio
	const FString CamId = TEXT("M.E.G. ARCHIVE CAM-04");
	const FString RadioFreq = TEXT("RX: 142.85 MHz  [SQUAD ACTIVE]");
	FCanvasTextItem CamText(FVector2D(ScreenWidth - Margin - 190.0f, Margin + 4.0f),
		FText::FromString(CamId), GEngine->GetSmallFont(), FLinearColor(0.85f, 0.95f, 0.85f, 0.85f));
	CamText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(CamText);

	FCanvasTextItem FreqText(FVector2D(ScreenWidth - Margin - 215.0f, Margin + 20.0f),
		FText::FromString(RadioFreq), GEngine->GetSmallFont(), FLinearColor(0.3f, 0.95f, 0.5f, 0.75f));
	FreqText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(FreqText);

	// 4. Bas-Gauche : Timestamp VHS 1998 analogique
	const int32 TotalSec = FMath::FloorToInt(TimeSeconds);
	const int32 Sec = TotalSec % 60;
	const int32 Min = (TotalSec / 60) % 60;
	const int32 Hour = (TotalSec / 3600) % 24;
	const FString TimecodeString = FString::Printf(TEXT("04.SEP.1998  %02d:%02d:%02d  NTSC"), (1 + Hour) % 24, (42 + Min) % 60, Sec);

	FCanvasTextItem DateText(FVector2D(Margin + 5.0f, ScreenHeight - Margin - 22.0f),
		FText::FromString(TimecodeString), GEngine->GetSmallFont(), FLinearColor(0.9f, 0.95f, 0.9f, 0.9f));
	DateText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(DateText);

	// 5. Bas-Droite : Compteur de bande magnetique
	const FString TapeString = FString::Printf(TEXT("TAPE %02d:%02d:%02d"), Hour, Min, Sec);
	FCanvasTextItem TapeText(FVector2D(ScreenWidth - Margin - 120.0f, ScreenHeight - Margin - 22.0f),
		FText::FromString(TapeString), GEngine->GetSmallFont(), FLinearColor(0.9f, 0.95f, 0.9f, 0.9f));
	TapeText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TapeText);

	// Barre de tracking analogique / parasite VHS qui descend
	const float TrackingY = FMath::Fmod(TimeSeconds * 60.0f, ScreenHeight);
	DrawRect(FLinearColor(1.0f, 1.0f, 1.0f, 0.025f), 0.0f, TrackingY, ScreenWidth, 4.0f);
}

void ALiminalScavengerHUD::DrawDamageVignette(float Intensity, float ScreenWidth, float ScreenHeight)
{
	if (!Canvas || Intensity <= 0.001f)
	{
		return;
	}

	const float Alpha = FMath::Clamp(Intensity * 0.75f, 0.0f, 0.85f);
	const float BorderSize = FMath::Clamp(80.0f * Intensity, 20.0f, 150.0f);

	// Vignette periphérique rouge sang lors d'un impact violent
	DrawRect(FLinearColor(0.85f, 0.05f, 0.05f, Alpha), 0.0f, 0.0f, ScreenWidth, BorderSize);
	DrawRect(FLinearColor(0.85f, 0.05f, 0.05f, Alpha), 0.0f, ScreenHeight - BorderSize, ScreenWidth, BorderSize);
	DrawRect(FLinearColor(0.85f, 0.05f, 0.05f, Alpha), 0.0f, BorderSize, BorderSize, ScreenHeight - (BorderSize * 2.0f));
	DrawRect(FLinearColor(0.85f, 0.05f, 0.05f, Alpha), ScreenWidth - BorderSize, BorderSize, BorderSize, ScreenHeight - (BorderSize * 2.0f));
}

void ALiminalScavengerHUD::DrawDownedIndicator(AScavengerCharacter* Scavenger, float ScreenWidth, float ScreenHeight)
{
	if (!Canvas || !Scavenger)
	{
		return;
	}

	const float CenterX = ScreenWidth * 0.5f;

	// 1. Si le joueur lui-même est au sol en état K.O.
	if (Scavenger->IsDowned())
	{
		const float Remaining = Scavenger->GetDownedTimeRemaining();
		const float Pulse = 0.5f + 0.5f * FMath::Sin(GlitchTimer * 4.0f);
		const float BoxWidth = 520.0f;
		const float BoxHeight = 60.0f;
		const float BoxX = CenterX - (BoxWidth * 0.5f);
		const float BoxY = ScreenHeight * 0.72f;

		DrawRect(FLinearColor(0.25f, 0.02f, 0.02f, 0.92f), BoxX, BoxY, BoxWidth, BoxHeight);
		Canvas->K2_DrawBox(FVector2D(BoxX, BoxY), FVector2D(BoxWidth, BoxHeight), 2.0f, FLinearColor(1.0f, 0.15f, 0.15f, Pulse));

		const FString DownedMsg = FString::Printf(TEXT("⚠ ÉTAT AGONISANT // HÉMORRAGIE CRITIQUE : %.0fs ⚠"), Remaining);
		FCanvasTextItem DownedItem(FVector2D(BoxX + 25.0f, BoxY + 12.0f),
			FText::FromString(DownedMsg), GEngine->GetMediumFont(), FLinearColor(1.0f, 0.2f, 0.2f, 1.0f));
		DownedItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(DownedItem);

		FCanvasTextItem SubItem(FVector2D(BoxX + 65.0f, BoxY + 36.0f),
			FText::FromString(TEXT("RAMPEZ VERS VOS COÉQUIPIERS POUR RÉANIMATION D'URGENCE")),
			GEngine->GetSmallFont(), FLinearColor(1.0f, 0.85f, 0.3f, Pulse));
		SubItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(SubItem);
		return;
	}

	// 2. Recherche d'alliés au sol dans le monde
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
		{
			AScavengerCharacter* Ally = *It;
			if (Ally && Ally != Scavenger && Ally->IsDowned())
			{
				const float Remaining = Ally->GetDownedTimeRemaining();
				const float Pulse = 0.6f + 0.4f * FMath::Sin(GlitchTimer * 5.0f);
				const float BannerW = 460.0f;
				const float BannerH = 34.0f;
				const float BannerX = CenterX - (BannerW * 0.5f);
				const float BannerY = 100.0f;

				DrawRect(FLinearColor(0.25f, 0.06f, 0.02f, 0.9f), BannerX, BannerY, BannerW, BannerH);
				Canvas->K2_DrawBox(FVector2D(BannerX, BannerY), FVector2D(BannerW, BannerH), 1.5f, FLinearColor(1.0f, 0.35f, 0.1f, Pulse));

				const FString AllyMsg = FString::Printf(TEXT("⚠ ALERTE : COÉQUIPIER AU SOL // RÉANIMATION : %.0fs"), Remaining);
				FCanvasTextItem AllyItem(FVector2D(BannerX + 18.0f, BannerY + 8.0f),
					FText::FromString(AllyMsg), GEngine->GetSmallFont(), FLinearColor(1.0f, 0.85f, 0.2f, 1.0f));
				AllyItem.EnableShadow(FLinearColor::Black);
				Canvas->DrawItem(AllyItem);
				break;
			}
		}
	}
}

void ALiminalScavengerHUD::OpenTerminalUI(ALiminalTerminalActor* InTerminal)
{
	bShowTerminalUI = true;
	ActiveTerminal = InTerminal;
	TerminalActiveTab = 0;
	TerminalSelectedIndex = 0;
	TerminalInputCooldown = 0.2f;

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void ALiminalScavengerHUD::CloseTerminalUI()
{
	bShowTerminalUI = false;
	ActiveTerminal = nullptr;

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());
	}
}

void ALiminalScavengerHUD::HandleTerminalInput()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC)
	{
		return;
	}

	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
	if (TerminalInputCooldown > 0.0f)
	{
		TerminalInputCooldown -= DeltaSeconds;
		return;
	}

	// Fermer le terminal
	if (PC->WasInputKeyJustPressed(EKeys::Escape) || PC->WasInputKeyJustPressed(EKeys::E))
	{
		CloseTerminalUI();
		return;
	}

	// Navigation Onglets
	if (PC->WasInputKeyJustPressed(EKeys::Tab) || PC->WasInputKeyJustPressed(EKeys::Right) || PC->WasInputKeyJustPressed(EKeys::D))
	{
		TerminalActiveTab = (TerminalActiveTab + 1) % 4;
		TerminalSelectedIndex = 0;
		TerminalInputCooldown = 0.15f;
		return;
	}
	if (PC->WasInputKeyJustPressed(EKeys::Left) || PC->WasInputKeyJustPressed(EKeys::A) || PC->WasInputKeyJustPressed(EKeys::Q))
	{
		TerminalActiveTab = (TerminalActiveTab + 3) % 4;
		TerminalSelectedIndex = 0;
		TerminalInputCooldown = 0.15f;
		return;
	}
	if (PC->WasInputKeyJustPressed(EKeys::One) || PC->WasInputKeyJustPressed(EKeys::NumPadOne)) { TerminalActiveTab = 0; TerminalSelectedIndex = 0; TerminalInputCooldown = 0.15f; return; }
	if (PC->WasInputKeyJustPressed(EKeys::Two) || PC->WasInputKeyJustPressed(EKeys::NumPadTwo)) { TerminalActiveTab = 1; TerminalSelectedIndex = 0; TerminalInputCooldown = 0.15f; return; }
	if (PC->WasInputKeyJustPressed(EKeys::Three) || PC->WasInputKeyJustPressed(EKeys::NumPadThree)) { TerminalActiveTab = 2; TerminalSelectedIndex = 0; TerminalInputCooldown = 0.15f; return; }
	if (PC->WasInputKeyJustPressed(EKeys::Four) || PC->WasInputKeyJustPressed(EKeys::NumPadFour)) { TerminalActiveTab = 3; TerminalSelectedIndex = 0; TerminalInputCooldown = 0.15f; return; }

	// Max items par onglet
	int32 MaxItems = 0;
	if (TerminalActiveTab == 0)
	{
		MaxItems = 11; // 11 biomes
	}
	else if (TerminalActiveTab == 1)
	{
		MaxItems = ActiveTerminal.IsValid() ? ActiveTerminal->GetStoreCatalog().Num() : 0;
	}

	if (MaxItems > 0)
	{
		if (PC->WasInputKeyJustPressed(EKeys::Up) || PC->WasInputKeyJustPressed(EKeys::W))
		{
			TerminalSelectedIndex = (TerminalSelectedIndex - 1 + MaxItems) % MaxItems;
			TerminalInputCooldown = 0.12f;
			return;
		}
		if (PC->WasInputKeyJustPressed(EKeys::Down) || PC->WasInputKeyJustPressed(EKeys::S))
		{
			TerminalSelectedIndex = (TerminalSelectedIndex + 1) % MaxItems;
			TerminalInputCooldown = 0.12f;
			return;
		}

		// Validation / Achat
		if (PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::SpaceBar) || PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			if (TerminalActiveTab == 0 && ActiveTerminal.IsValid())
			{
				ActiveTerminal->ServerSelectBiome(static_cast<ELevelBiome>(TerminalSelectedIndex));
				TerminalInputCooldown = 0.25f;
			}
			else if (TerminalActiveTab == 1 && ActiveTerminal.IsValid())
			{
				const TArray<FTerminalStoreItem>& Catalog = ActiveTerminal->GetStoreCatalog();
				if (Catalog.IsValidIndex(TerminalSelectedIndex))
				{
					ActiveTerminal->ServerPurchaseStoreItem(Catalog[TerminalSelectedIndex].ItemId, GetOwningScavenger());
					TerminalInputCooldown = 0.25f;
				}
			}
		}
	}
}

void ALiminalScavengerHUD::DrawTerminalUI(float ScreenWidth, float ScreenHeight)
{
	if (!Canvas) return;

	const float BoxW = 1000.0f;
	const float BoxH = 680.0f;
	const float BoxX = (ScreenWidth - BoxW) * 0.5f;
	const float BoxY = (ScreenHeight - BoxH) * 0.5f;

	// Fond noir CRT
	DrawRect(FLinearColor(0.015f, 0.02f, 0.018f, 0.95f), BoxX, BoxY, BoxW, BoxH);

	// Bordure CRT verte
	const FLinearColor CRTColor(0.15f, 1.0f, 0.35f, 1.0f);
	const FLinearColor CRTDimColor(0.08f, 0.45f, 0.15f, 0.8f);
	const FLinearColor CRTAmber(1.0f, 0.75f, 0.2f, 1.0f);
	Canvas->K2_DrawBox(FVector2D(BoxX, BoxY), FVector2D(BoxW, BoxH), 2.0f, CRTColor);

	// En-tete
	DrawRect(FLinearColor(0.03f, 0.12f, 0.05f, 0.9f), BoxX + 2.0f, BoxY + 2.0f, BoxW - 4.0f, 40.0f);
	Canvas->K2_DrawLine(FVector2D(BoxX, BoxY + 42.0f), FVector2D(BoxX + BoxW, BoxY + 42.0f), 1.5f, CRTColor);

	FCanvasTextItem TitleText(FVector2D(BoxX + 20.0f, BoxY + 10.0f),
		FText::FromString(TEXT("M.E.G. OS v3.12 // TERMINAL LOGISTIQUE DE BASE ALPHA")),
		GEngine->GetMediumFont(), CRTColor);
	TitleText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TitleText);

	// Sous-titre : credits et destination
	ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(GetGameInstance());
	const int32 Credits = GI ? GI->GetTotalCredits() : 0;
	const ELevelBiome CurBiome = ActiveTerminal.IsValid() ? ActiveTerminal->GetCurrentlySelectedBiome() : ELevelBiome::Level0_YellowLobby;

	FString BiomeLabel = TEXT("Niveau 0 — Le Lobby");
	switch (CurBiome)
	{
	case ELevelBiome::Level0_YellowLobby: BiomeLabel = TEXT("Niveau 0 — Le Lobby Jaune"); break;
	case ELevelBiome::Level1_HabitableZone: BiomeLabel = TEXT("Niveau 1 — Zone Habitable"); break;
	case ELevelBiome::Level2_PipeDreams: BiomeLabel = TEXT("Niveau 2 — Pipe Dreams"); break;
	case ELevelBiome::Level3_ElectricalStation: BiomeLabel = TEXT("Niveau 3 — Station Electrique"); break;
	case ELevelBiome::Level4_AbandonedOffice: BiomeLabel = TEXT("Niveau 4 — Bureaux Abandonnes"); break;
	case ELevelBiome::Level6_LightsOut: BiomeLabel = TEXT("Niveau 6 — Noir Absolu (Lights Out)"); break;
	case ELevelBiome::Level8_CaveSystem: BiomeLabel = TEXT("Niveau 8 — Cavernes & Stalactites"); break;
	case ELevelBiome::Level9_DarkSuburbs: BiomeLabel = TEXT("Niveau 9 — Faubourg Obscur"); break;
	case ELevelBiome::Level10_WheatFields: BiomeLabel = TEXT("Niveau 10 — Champs de Ble"); break;
	case ELevelBiome::Level37_Poolrooms: BiomeLabel = TEXT("Niveau 37 — Les Poolrooms"); break;
	case ELevelBiome::LevelRun_RunForYourLife: BiomeLabel = TEXT("Niveau ! — Fuyez pour survivre !"); break;
	default: break;
	}

	const FString Subheader = FString::Printf(TEXT("DESTINATION PROGRAMMEE : [%s]   |   SOLDE CORPORATIF : %d CR"), *BiomeLabel, Credits);
	FCanvasTextItem SubText(FVector2D(BoxX + 20.0f, BoxY + 48.0f), FText::FromString(Subheader), GEngine->GetSmallFont(), CRTAmber);
	SubText.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(SubText);

	// Barre des onglets
	const float TabY = BoxY + 75.0f;
	TArray<FString> TabLabels = {
		TEXT("[ 1. DESTINATIONS ]"),
		TEXT("[ 2. BOUTIQUE / OUTILS ]"),
		TEXT("[ 3. MANDAT & QUOTA ]"),
		TEXT("[ 4. BESTIAIRE & SURVIE ]")
	};
	const float TabW = (BoxW - 40.0f) / 4.0f;
	for (int32 i = 0; i < 4; ++i)
	{
		const float TX = BoxX + 20.0f + i * TabW;
		const bool bIsActive = (i == TerminalActiveTab);
		if (bIsActive)
		{
			DrawRect(FLinearColor(0.05f, 0.25f, 0.1f, 0.9f), TX, TabY, TabW - 6.0f, 28.0f);
			Canvas->K2_DrawBox(FVector2D(TX, TabY), FVector2D(TabW - 6.0f, 28.0f), 1.5f, CRTColor);
		}
		else
		{
			DrawRect(FLinearColor(0.02f, 0.08f, 0.04f, 0.6f), TX, TabY, TabW - 6.0f, 28.0f);
		}
		FCanvasTextItem TText(FVector2D(TX + 12.0f, TabY + 6.0f), FText::FromString(TabLabels[i]),
			GEngine->GetSmallFont(), bIsActive ? CRTColor : CRTDimColor);
		Canvas->DrawItem(TText);
	}
	Canvas->K2_DrawLine(FVector2D(BoxX + 20.0f, TabY + 34.0f), FVector2D(BoxX + BoxW - 20.0f, TabY + 34.0f), 1.0f, CRTColor);

	// Zone de contenu
	const float ContentY = TabY + 45.0f;

	if (TerminalActiveTab == 0)
	{
		// ONGLET 1 : DESTINATIONS (11 BIOMES)
		struct FBiomeInfo { FString Name; FString Danger; FString Scrap; FString Entities; };
		TArray<FBiomeInfo> BiomeList = {
			{ TEXT("Niveau 0 — Le Lobby Jaune"), TEXT("Faible"), TEXT("1.0x"), TEXT("Smiler, Hound") },
			{ TEXT("Niveau 1 — Zone Habitable"), TEXT("Modere"), TEXT("1.2x"), TEXT("Hound, Skin-stealer") },
			{ TEXT("Niveau 2 — Pipe Dreams (Tuyaux)"), TEXT("Eleve"), TEXT("1.5x"), TEXT("Smiler, Clump") },
			{ TEXT("Niveau 3 — Station Electrique"), TEXT("Eleve"), TEXT("1.8x"), TEXT("Duller, Smiler") },
			{ TEXT("Niveau 4 — Bureaux Abandonnes"), TEXT("Modere"), TEXT("1.4x"), TEXT("Hound, Jerry") },
			{ TEXT("Niveau 6 — Noir Absolu (Lights Out)"), TEXT("Mortel"), TEXT("2.2x"), TEXT("Smiler, Wretch") },
			{ TEXT("Niveau 8 — Cavernes & Stalactites"), TEXT("Mortel"), TEXT("2.5x"), TEXT("Deathmoth, Clump") },
			{ TEXT("Niveau 9 — Le Faubourg Obscur"), TEXT("Tres Eleve"), TEXT("2.0x"), TEXT("Skin-stealer") },
			{ TEXT("Niveau 10 — Les Champs de Ble"), TEXT("Eleve"), TEXT("1.9x"), TEXT("Watcher, Smiler") },
			{ TEXT("Niveau 37 — Les Poolrooms"), TEXT("Variable"), TEXT("2.0x"), TEXT("Hydrolitis") },
			{ TEXT("Niveau ! — Fuyez pour survivre !"), TEXT("Extreme"), TEXT("3.5x"), TEXT("Horde Smilers") }
		};

		FCanvasTextItem HeaderRow(FVector2D(BoxX + 30.0f, ContentY),
			FText::FromString(TEXT("  NIVEAU                         DANGER       SCRAP   MENACES PRINCIPALES")),
			GEngine->GetSmallFont(), CRTAmber);
		Canvas->DrawItem(HeaderRow);

		for (int32 i = 0; i < BiomeList.Num(); ++i)
		{
			const float RowY = ContentY + 20.0f + i * 42.0f;
			const bool bSelected = (i == TerminalSelectedIndex);
			const bool bProgrammed = (i == static_cast<int32>(CurBiome));

			if (bSelected)
			{
				DrawRect(FLinearColor(0.08f, 0.35f, 0.15f, 0.7f), BoxX + 25.0f, RowY - 2.0f, BoxW - 50.0f, 36.0f);
				Canvas->K2_DrawBox(FVector2D(BoxX + 25.0f, RowY - 2.0f), FVector2D(BoxW - 50.0f, 36.0f), 1.0f, CRTColor);
			}

			FString Prefix = bSelected ? TEXT("> ") : TEXT("  ");
			FString StatusTag = bProgrammed ? TEXT(" [PROGRAMME]") : TEXT("");
			FString LineStr = FString::Printf(TEXT("%s%-30s %-12s %-7s %s%s"),
				*Prefix, *BiomeList[i].Name, *BiomeList[i].Danger, *BiomeList[i].Scrap, *BiomeList[i].Entities, *StatusTag);

			FCanvasTextItem RowText(FVector2D(BoxX + 30.0f, RowY + 8.0f), FText::FromString(LineStr),
				GEngine->GetSmallFont(), bSelected ? CRTAmber : (bProgrammed ? CRTColor : CRTDimColor));
			Canvas->DrawItem(RowText);
		}
	}
	else if (TerminalActiveTab == 1)
	{
		// ONGLET 2 : BOUTIQUE / OUTILS M.E.G.
		FCanvasTextItem HeaderRow(FVector2D(BoxX + 30.0f, ContentY),
			FText::FromString(TEXT("  ARTICLE / EQUIPEMENT M.E.G.                     PRIX      STATUT DISPONIBLE")),
			GEngine->GetSmallFont(), CRTAmber);
		Canvas->DrawItem(HeaderRow);

		if (ActiveTerminal.IsValid())
		{
			const TArray<FTerminalStoreItem>& Catalog = ActiveTerminal->GetStoreCatalog();
			for (int32 i = 0; i < Catalog.Num(); ++i)
			{
				const float RowY = ContentY + 20.0f + i * 42.0f;
				const bool bSelected = (i == TerminalSelectedIndex);
				const bool bAffordable = (Credits >= Catalog[i].CostCredits);

				if (bSelected)
				{
					DrawRect(FLinearColor(0.08f, 0.35f, 0.15f, 0.7f), BoxX + 25.0f, RowY - 2.0f, BoxW - 50.0f, 36.0f);
					Canvas->K2_DrawBox(FVector2D(BoxX + 25.0f, RowY - 2.0f), FVector2D(BoxW - 50.0f, 36.0f), 1.0f, CRTColor);
				}

				FString Prefix = bSelected ? TEXT("> ") : TEXT("  ");
				FString PriceStr = FString::Printf(TEXT("%d CR"), Catalog[i].CostCredits);
				FString AffordStr = bAffordable ? TEXT("[EN STOCK]") : TEXT("[FONDS INSUFFISANTS]");

				FString ItemLine = FString::Printf(TEXT("%s%-44s %-9s %s"),
					*Prefix, *Catalog[i].DisplayName.ToString(), *PriceStr, *AffordStr);

				FCanvasTextItem ItemText(FVector2D(BoxX + 30.0f, RowY + 8.0f), FText::FromString(ItemLine),
					GEngine->GetSmallFont(), bSelected ? CRTAmber : (bAffordable ? CRTColor : FLinearColor(0.6f, 0.2f, 0.2f, 0.8f)));
				Canvas->DrawItem(ItemText);
			}
		}
	}
	else if (TerminalActiveTab == 2)
	{
		// ONGLET 3 : MANDAT & QUOTA
		FCanvasTextItem MandatTitle(FVector2D(BoxX + 30.0f, ContentY),
			FText::FromString(TEXT("MANDAT CORPORATIF M.E.G. // PROTOCOLE D'EXTRACTION TRI-JOURNALIER")),
			GEngine->GetMediumFont(), CRTColor);
		Canvas->DrawItem(MandatTitle);

		TArray<FString> QuotaLines = {
			TEXT(""),
			TEXT("FORMULE CANONIQUE DU QUOTA : Q(k, N) = floor(180 * (1.32)^(k-1) + 55 * (k-1)^1.4) + 45 * (N - 1)"),
			TEXT("Le quota doit etre solde tous les 3 jours d'incursion a la baie de depose de la Base Alpha."),
			TEXT("Tout butin (composants, cuivre, noyaux anormaux) doit etre decharge dans la zone [AExtractionZone]."),
			TEXT(""),
			TEXT("ASSURANCE FUNERAIRE ET SANCTIONS MILITAIRES :"),
			TEXT("  - Rapatriement de plaque d'identification (Dog Tag) : +50 CR credites a l'escouade."),
			TEXT("  - Abandon de depouille humaine sur le terrain : -80 CR de penalite militaire."),
			TEXT(""),
			TEXT("BATTERIE ET RESERVES DU SAS :"),
			TEXT("  - Reserve electrique du sas : 1000 Wh."),
			TEXT("  - Consommation maintien de porte : 4 Wh/s  |  Purge et surcharge sous pression : 150 Wh."),
			TEXT(""),
			TEXT("AVERTISSEMENT : En cas de defaut de paiement au 3eme jour, l'escouade est bannie de la Base Alpha.")
		};

		for (int32 i = 0; i < QuotaLines.Num(); ++i)
		{
			FCanvasTextItem LineText(FVector2D(BoxX + 30.0f, ContentY + 28.0f + i * 26.0f),
				FText::FromString(QuotaLines[i]), GEngine->GetSmallFont(), (i <= 5) ? CRTColor : CRTAmber);
			Canvas->DrawItem(LineText);
		}
	}
	else if (TerminalActiveTab == 3)
	{
		// ONGLET 4 : DOSSIERS BESTIAIRE & SURVIE
		FCanvasTextItem BestiaryTitle(FVector2D(BoxX + 30.0f, ContentY),
			FText::FromString(TEXT("DOSSIERS DE VULNERABILITE DU BESTIAIRE M.E.G.")),
			GEngine->GetMediumFont(), CRTColor);
		Canvas->DrawItem(BestiaryTitle);

		TArray<FString> BestiaryLines = {
			TEXT(""),
			TEXT("1. SMILER (L'Entite Souriante) :"),
			TEXT("   - Eteindre IMPERATIVEMENT torches et lampes frontales des detection de la lueur des dents."),
			TEXT("   - Maintenir un contact visuel fixe sans ciller. Reculer lentement. Ne JAMAIS tourner le dos."),
			TEXT(""),
			TEXT("2. HOUND (Le Molosse) :"),
			TEXT("   - Traque a l'ouie (RMS acoustique > 0.03 = declenchement immediat de la charge)."),
			TEXT("   - Silence radio absolu. S'accroupir pour etouffer les bruits de pas. Intimidation frontale < 4m."),
			TEXT(""),
			TEXT("3. SKIN-STEALER (L'Ecorcheur) :"),
			TEXT("   - Usurpation d'identite d'agents morts. Verifier le signal sas ou mot de passe oral."),
			TEXT(""),
			TEXT("4. PARTYGOER (L'Hote de Fete) :"),
			TEXT("   - Ne jamais s'approcher ni toucher. Falsifie les transmissions radio avec des emoticones '=)'.")
		};

		for (int32 i = 0; i < BestiaryLines.Num(); ++i)
		{
			FCanvasTextItem LineText(FVector2D(BoxX + 30.0f, ContentY + 28.0f + i * 26.0f),
				FText::FromString(BestiaryLines[i]), GEngine->GetSmallFont(), (i % 4 == 1) ? CRTAmber : CRTColor);
			Canvas->DrawItem(LineText);
		}
	}

	// Pied de page / Raccourcis
	const float FooterY = BoxY + BoxH - 35.0f;
	Canvas->K2_DrawLine(FVector2D(BoxX, FooterY), FVector2D(BoxX + BoxW, FooterY), 1.0f, CRTColor);
	FCanvasTextItem FooterText(FVector2D(BoxX + 20.0f, FooterY + 8.0f),
		FText::FromString(TEXT("[1-4 / TAB] Onglet   |   [W/S / Fleches] Naviguer   |   [ENTREE / ESPACE] Confirmer / Acheter   |   [ECHAP / E] Quitter")),
		GEngine->GetSmallFont(), CRTAmber);
	Canvas->DrawItem(FooterText);
}

