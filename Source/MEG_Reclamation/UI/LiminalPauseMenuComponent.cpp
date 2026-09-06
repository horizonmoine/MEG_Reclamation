#include "UI/LiminalPauseMenuComponent.h"

#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Data/LiminalGameInstance.h"

// Couleurs CRT (coherentes avec MainMenuHUD)
static const FLinearColor Pause_Amber(1.0f, 0.75f, 0.2f, 1.0f);
static const FLinearColor Pause_AmberDim(0.6f, 0.45f, 0.12f, 0.6f);
static const FLinearColor Pause_AmberBright(1.0f, 0.85f, 0.35f, 1.0f);
static const FLinearColor Pause_Background(0.02f, 0.02f, 0.015f, 0.95f);

ULiminalPauseMenuComponent::ULiminalPauseMenuComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void ULiminalPauseMenuComponent::TogglePause()
{
	bIsPaused = !bIsPaused;
	SelectedButton = 0;
	bShowInlineSettings = false;

	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (bIsPaused)
		{
			PC->bShowMouseCursor = true;
			PC->SetPause(true);
		}
		else
		{
			PC->bShowMouseCursor = false;
			PC->SetPause(false);
		}
	}
}

void ULiminalPauseMenuComponent::DrawPauseMenu(UCanvas* InCanvas, float ScreenWidth, float ScreenHeight, float DeltaTime)
{
	if (!bIsPaused || !InCanvas)
	{
		return;
	}

	PulseTimer += DeltaTime;
	if (InputCooldown > 0.0f)
	{
		InputCooldown -= DeltaTime;
	}

	// Fond semi-transparent plein ecran
	const FLinearColor Overlay(0.0f, 0.0f, 0.0f, 0.7f);
	FCanvasTileItem OverlayBox(FVector2D::ZeroVector, FVector2D(ScreenWidth, ScreenHeight), Overlay);
	OverlayBox.BlendMode = SE_BLEND_Translucent;
	InCanvas->DrawItem(OverlayBox, FVector2D(0.0f, 0.0f));

	// Cadre du menu
	const float FrameW = ScreenWidth * 0.35f;
	const float FrameH = ScreenHeight * 0.4f;
	const float FrameX = (ScreenWidth - FrameW) * 0.5f;
	const float FrameY = (ScreenHeight - FrameH) * 0.5f;
	const float Scale = FMath::Clamp(ScreenWidth / 1920.0f, 0.5f, 1.5f);

	FCanvasTileItem BgBox(FVector2D::ZeroVector, FVector2D(FrameW, FrameH), Pause_Background);
	BgBox.BlendMode = SE_BLEND_Translucent;
	InCanvas->DrawItem(BgBox, FVector2D(FrameX, FrameY));

	// Bordure ambre
	InCanvas->K2_DrawBox(FVector2D(FrameX, FrameY), FVector2D(FrameW, FrameH), 2.0f, Pause_AmberDim);

	// Titre
	const FString Title = TEXT("═══ PAUSE ═══");
	FCanvasTextItem TitleItem(FVector2D(FrameX + 20.0f, FrameY + 15.0f * Scale), FText::FromString(Title), nullptr, Pause_AmberBright);
	TitleItem.Scale = FVector2D(Scale, Scale);
	InCanvas->DrawItem(TitleItem);

	// Boutons
	const TArray<FString> Labels = { TEXT("> REPRENDRE"), TEXT("> OPTIONS"), TEXT("> QUITTER AU MENU") };
	const float BtnX = FrameX + 40.0f;
	const float BtnW = FrameW - 80.0f;
	const float BtnH = 40.0f * Scale;
	float BtnY = FrameY + 70.0f * Scale;

	for (int32 i = 0; i < Labels.Num(); ++i)
	{
		const bool bSel = (i == SelectedButton);
		const FLinearColor BgC = bSel
			? FLinearColor(Pause_Amber.R * 0.15f, Pause_Amber.G * 0.15f, Pause_Amber.B * 0.15f, 0.5f)
			: FLinearColor(0.03f, 0.03f, 0.02f, 0.3f);
		const FLinearColor TxtC = bSel ? Pause_AmberBright : Pause_AmberDim;

		FCanvasTileItem BtnBg(FVector2D::ZeroVector, FVector2D(BtnW, BtnH), BgC);
		BtnBg.BlendMode = SE_BLEND_Translucent;
		InCanvas->DrawItem(BtnBg, FVector2D(BtnX, BtnY));

		if (bSel)
		{
			FCanvasTileItem Accent(FVector2D::ZeroVector, FVector2D(3.0f, BtnH), Pause_AmberBright);
			Accent.BlendMode = SE_BLEND_Translucent;
			InCanvas->DrawItem(Accent, FVector2D(BtnX, BtnY));
		}

		FCanvasTextItem BtnText(FVector2D(BtnX + 28.0f, BtnY + BtnH * 0.2f),
			FText::FromString(Labels[i]), nullptr, TxtC);
		BtnText.Scale = FVector2D(Scale * 0.9f, Scale * 0.9f);
		InCanvas->DrawItem(BtnText);

		BtnY += BtnH + 10.0f * Scale;
	}
}

void ULiminalPauseMenuComponent::HandlePauseInput(APlayerController* PC)
{
	if (!PC || !bIsPaused || InputCooldown > 0.0f)
	{
		return;
	}

	if (PC->WasInputKeyJustPressed(EKeys::Up) || PC->WasInputKeyJustPressed(EKeys::W))
	{
		SelectedButton = (SelectedButton - 1 + BUTTON_COUNT) % BUTTON_COUNT;
		InputCooldown = 0.15f;
	}
	else if (PC->WasInputKeyJustPressed(EKeys::Down) || PC->WasInputKeyJustPressed(EKeys::S))
	{
		SelectedButton = (SelectedButton + 1) % BUTTON_COUNT;
		InputCooldown = 0.15f;
	}

	if (PC->WasInputKeyJustPressed(EKeys::Enter))
	{
		switch (SelectedButton)
		{
		case 0: OnResume(); break;
		case 1: OnOptions(); break;
		case 2: OnQuitToMenu(); break;
		}
		InputCooldown = 0.25f;
	}

	if (PC->WasInputKeyJustPressed(EKeys::Escape))
	{
		OnResume();
		InputCooldown = 0.25f;
	}
}

void ULiminalPauseMenuComponent::OnResume()
{
	TogglePause();
}

void ULiminalPauseMenuComponent::OnOptions()
{
	bShowInlineSettings = !bShowInlineSettings;
	InlineSettingsSelection = 0;
}

void ULiminalPauseMenuComponent::OnQuitToMenu()
{
	// Unpause avant de quitter
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		PC->SetPause(false);
		PC->bShowMouseCursor = true;
	}

	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SaveGameToDisk();
		// Retour au menu principal (la carte du menu principal)
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("/Game/Maps/Lvl_MainMenu")));
	}
}
