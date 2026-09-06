#include "UI/LiminalDeathScreenHUD.h"

#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameModes/LiminalGameMode.h"
#include "Player/LiminalSpectatorPawn.h"

ALiminalDeathScreenHUD::ALiminalDeathScreenHUD()
{
}

void ALiminalDeathScreenHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeGameAndUI());
	}
}

void ALiminalDeathScreenHUD::SetDeathDetails(const FString& InKillerName, int32 InLostCredits, float InSurvivalTimeSeconds)
{
	KillerName = InKillerName;
	LostCredits = InLostCredits;
	SurvivalTimeSeconds = InSurvivalTimeSeconds;
	AnimationTimer = 0.0f;
	bTransitionTriggered = false;
}

void ALiminalDeathScreenHUD::TransitionToSpectator()
{
	if (bTransitionTriggered)
	{
		return;
	}
	bTransitionTriggered = true;

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());

		// Si un pion spectateur existe, assure la possession
		if (ALiminalSpectatorPawn* Spec = Cast<ALiminalSpectatorPawn>(PC->GetPawn()))
		{
			Spec->ViewNextPlayer();
		}
	}
}

void ALiminalDeathScreenHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;

	AnimationTimer += DeltaSeconds;
	ScanlineOffset = FMath::Fmod(ScanlineOffset + DeltaSeconds * 80.0f, H);

	// Auto-transition apres le delai
	if (AnimationTimer >= AutoTransitionDelay && !bTransitionTriggered)
	{
		TransitionToSpectator();
	}

	DrawCRTBackground(W, H);
	DrawDeathReport(W, H);
	DrawScanlines(W, H);

	// Verification d'input pour accelerer la bascule
	if (APlayerController* PC = GetOwningPlayerController())
	{
		if (PC->WasInputKeyJustPressed(EKeys::SpaceBar) || PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
		{
			TransitionToSpectator();
		}
	}
}

void ALiminalDeathScreenHUD::DrawCRTBackground(float Width, float Height)
{
	// Fond rouge tres sombre avec vignette d'angoisse
	const float Flicker = 0.94f + 0.06f * FMath::Sin(AnimationTimer * 18.0f);
	FCanvasTileItem BgTile(FVector2D::ZeroVector, FVector2D(Width, Height),
		FLinearColor(0.04f * Flicker, 0.01f, 0.01f, 0.95f));
	BgTile.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(BgTile);

	// Liseres CRT d'avertissement
	Canvas->K2_DrawLine(FVector2D(50.0f, 50.0f), FVector2D(Width - 50.0f, 50.0f), 2.0f, FLinearColor(0.85f, 0.15f, 0.15f, 0.8f));
	Canvas->K2_DrawLine(FVector2D(50.0f, Height - 50.0f), FVector2D(Width - 50.0f, Height - 50.0f), 2.0f, FLinearColor(0.85f, 0.15f, 0.15f, 0.8f));
	Canvas->K2_DrawLine(FVector2D(50.0f, 50.0f), FVector2D(50.0f, Height - 50.0f), 2.0f, FLinearColor(0.85f, 0.15f, 0.15f, 0.8f));
	Canvas->K2_DrawLine(FVector2D(Width - 50.0f, 50.0f), FVector2D(Width - 50.0f, Height - 50.0f), 2.0f, FLinearColor(0.85f, 0.15f, 0.15f, 0.8f));
}

void ALiminalDeathScreenHUD::DrawDeathReport(float Width, float Height)
{
	const float CenterX = Width * 0.5f;
	const float Pulse = 0.85f + 0.15f * FMath::Sin(AnimationTimer * 4.0f);

	// 1. Titre d'urgence K.I.A.
	FCanvasTextItem TitleItem(FVector2D(CenterX - 280.0f, Height * 0.15f),
		FText::FromString(TEXT("⚠ M.E.G. // RAPPORT D'ÉCHEC D'INCURSION ⚠")),
		GEngine->GetLargeFont(), FLinearColor(1.0f, 0.2f, 0.2f, Pulse));
	TitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(TitleItem);

	FCanvasTextItem SubtitleItem(FVector2D(CenterX - 220.0f, Height * 0.22f),
		FText::FromString(TEXT("SIGNAL BIOMÉTRIQUE PERDU — OPÉRATEUR K.I.A.")),
		GEngine->GetMediumFont(), FLinearColor(0.9f, 0.4f, 0.4f, 0.9f));
	SubtitleItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(SubtitleItem);

	Canvas->K2_DrawLine(FVector2D(CenterX - 300.0f, Height * 0.27f), FVector2D(CenterX + 300.0f, Height * 0.27f),
		1.5f, FLinearColor(0.8f, 0.2f, 0.2f, 0.7f));

	// 2. Details du deces
	float Y = Height * 0.35f;
	const float StepY = 38.0f;

	auto DrawDetailRow = [this, CenterX](float InY, const FString& Label, const FString& Value, const FLinearColor& ValColor)
	{
		FCanvasTextItem LabelItem(FVector2D(CenterX - 260.0f, InY), FText::FromString(Label),
			GEngine->GetMediumFont(), FLinearColor(0.85f, 0.85f, 0.85f, 0.85f));
		LabelItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(LabelItem);

		FCanvasTextItem ValueItem(FVector2D(CenterX + 60.0f, InY), FText::FromString(Value),
			GEngine->GetMediumFont(), ValColor);
		ValueItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(ValueItem);
	};

	DrawDetailRow(Y, TEXT("CAUSE DU DÉCÈS :"), KillerName.ToUpper(), FLinearColor(1.0f, 0.3f, 0.3f));
	Y += StepY;

	const int32 Mins = FMath::FloorToInt(SurvivalTimeSeconds / 60.0f);
	const int32 Secs = FMath::FloorToInt(FMath::Fmod(SurvivalTimeSeconds, 60.0f));
	const FString TimeStr = FString::Printf(TEXT("%02d MIN %02d SEC"), Mins, Secs);
	DrawDetailRow(Y, TEXT("DURÉE OPÉRATIONNELLE :"), TimeStr, FLinearColor(0.3f, 0.9f, 1.0f));
	Y += StepY;

	const FString LootStr = FString::Printf(TEXT("%d CR (ABANDONNÉS)"), LostCredits);
	DrawDetailRow(Y, TEXT("BUTIN NON SÉCURISÉ :"), LootStr, FLinearColor(1.0f, 0.75f, 0.2f));
	Y += StepY;

	DrawDetailRow(Y, TEXT("STATUT BALISE D'URGENCE :"), TEXT("ACTIVÉE (MODE CCTV SPECTATEUR)"), FLinearColor(0.2f, 1.0f, 0.4f));

	// 3. Prompt de transition
	const float PromptPulse = 0.5f + 0.5f * FMath::Sin(AnimationTimer * 6.0f);
	const float RemainingSecs = FMath::Max(0.0f, AutoTransitionDelay - AnimationTimer);
	const FString PromptStr = FString::Printf(TEXT("APPUYER SUR [ESPACE] POUR BASCULER SUR LA CAMÉRA D'ÉQUIPE (AUTO DANS %.0fs)"), RemainingSecs);

	FCanvasTextItem PromptItem(FVector2D(CenterX - 350.0f, Height * 0.82f),
		FText::FromString(PromptStr), GEngine->GetMediumFont(), FLinearColor(1.0f, 0.85f, 0.3f, PromptPulse));
	PromptItem.EnableShadow(FLinearColor::Black);
	Canvas->DrawItem(PromptItem);
}

void ALiminalDeathScreenHUD::DrawScanlines(float Width, float Height)
{
	const float Step = 4.0f;
	const float Alpha = 0.04f + 0.02f * FMath::Sin(AnimationTimer * 8.0f);
	for (float Y = 0.0f; Y < Height; Y += Step)
	{
		Canvas->K2_DrawLine(FVector2D(0.0f, Y), FVector2D(Width, Y), 1.0f, FLinearColor(0.0f, 0.0f, 0.0f, Alpha));
	}
}
