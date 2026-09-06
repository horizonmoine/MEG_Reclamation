#include "UI/LiminalMainMenuHUD.h"

#include "Engine/Canvas.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Data/LiminalGameInstance.h"

// ─── Couleurs du terminal CRT ───────────────────────────────────────────────
const FLinearColor ALiminalMainMenuHUD::CRT_Amber      = FLinearColor(1.0f, 0.75f, 0.2f, 1.0f);
const FLinearColor ALiminalMainMenuHUD::CRT_AmberDim   = FLinearColor(0.6f, 0.45f, 0.12f, 0.6f);
const FLinearColor ALiminalMainMenuHUD::CRT_AmberBright = FLinearColor(1.0f, 0.85f, 0.35f, 1.0f);
const FLinearColor ALiminalMainMenuHUD::CRT_Green      = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f);
const FLinearColor ALiminalMainMenuHUD::CRT_Red        = FLinearColor(1.0f, 0.25f, 0.2f, 1.0f);
const FLinearColor ALiminalMainMenuHUD::CRT_Background = FLinearColor(0.02f, 0.02f, 0.015f, 0.92f);
const FLinearColor ALiminalMainMenuHUD::CRT_Scanline   = FLinearColor(0.0f, 0.0f, 0.0f, 0.08f);

ALiminalMainMenuHUD::ALiminalMainMenuHUD()
{
}

void ALiminalMainMenuHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}

	LoadCurrentSettings();
}

// ─── DRAW HUD (point d'entree chaque frame) ────────────────────────────────
void ALiminalMainMenuHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;

	// Timers d'animation
	TitlePulseTimer += DeltaSeconds;
	ScanlineOffset = FMath::Fmod(ScanlineOffset + DeltaSeconds * 60.0f, H);
	CRTFlickerTimer += DeltaSeconds * 7.0f;
	if (InputCooldown > 0.0f)
	{
		InputCooldown -= DeltaSeconds;
	}

	// Transition alpha
	if (bTransitioning)
	{
		TransitionAlpha += DeltaSeconds * 4.0f;
		if (TransitionAlpha >= 1.0f)
		{
			TransitionAlpha = 0.0f;
			CurrentScreen = TransitionTarget;
			bTransitioning = false;
			SelectedButtonIndex = 0;
		}
	}

	// Fond noir
	DrawRect(FLinearColor::Black, 0.0f, 0.0f, W, H);

	// Dessin de l'ecran actif
	switch (CurrentScreen)
	{
	case EMenuScreen::Title:
		DrawTitleScreen(W, H);
		break;
	case EMenuScreen::MainMenu:
		DrawMainMenu(W, H);
		break;
	case EMenuScreen::Settings:
		DrawSettingsScreen(W, H);
		break;
	case EMenuScreen::HostLobby:
		DrawHostLobby(W, H);
		break;
	case EMenuScreen::JoinLobby:
		DrawJoinLobby(W, H);
		break;
	}

	// Cadre CRT et scanlines par-dessus tout
	DrawCRTFrame(W, H);
	DrawScanlines(W, H);

	// Overlay de transition (fondu noir)
	if (bTransitioning)
	{
		DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, TransitionAlpha), 0.0f, 0.0f, W, H);
	}

	// Input
	HandleMenuInput();
}

// ─── ECRAN TITRE ────────────────────────────────────────────────────────────
void ALiminalMainMenuHUD::DrawTitleScreen(float W, float H)
{
	const float CenterX = W * 0.5f;
	const float CenterY = H * 0.5f;

	// Cadre CRT semi-transparent de fond
	const float FrameW = W * 0.7f;
	const float FrameH = H * 0.6f;
	DrawRect(CRT_Background, (W - FrameW) * 0.5f, (H - FrameH) * 0.5f, FrameW, FrameH);

	// Titre du jeu
	const FString TitleLine1 = TEXT("M . E . G .");
	const FString TitleLine2 = TEXT("R E C L A M A T I O N");
	const float TitleScale = FMath::Clamp(W / 1920.0f, 0.6f, 2.0f);

	DrawText(TitleLine1, CRT_AmberBright, CenterX - TitleLine1.Len() * 12.0f * TitleScale,
		CenterY - 80.0f * TitleScale, nullptr, TitleScale * 2.5f);
	DrawText(TitleLine2, CRT_Amber, CenterX - TitleLine2.Len() * 7.0f * TitleScale,
		CenterY - 20.0f * TitleScale, nullptr, TitleScale * 1.5f);

	// Sous-titre
	const FString Subtitle = TEXT("MAJOR EXPLORER GROUP — DIVISION RECUPERATION");
	DrawText(Subtitle, CRT_AmberDim, CenterX - Subtitle.Len() * 4.0f * TitleScale,
		CenterY + 40.0f * TitleScale, nullptr, TitleScale * 0.85f);

	// "APPUYER SUR UNE TOUCHE" clignotant
	const float PulseAlpha = (FMath::Sin(TitlePulseTimer * 3.0f) + 1.0f) * 0.5f;
	const FString PressKey = TEXT("[ APPUYER SUR UNE TOUCHE ]");
	const FLinearColor PulseColor = FLinearColor(CRT_Amber.R, CRT_Amber.G, CRT_Amber.B, PulseAlpha);
	DrawText(PressKey, PulseColor, CenterX - PressKey.Len() * 4.5f * TitleScale,
		CenterY + 120.0f * TitleScale, nullptr, TitleScale * 1.0f);

	// Version en bas a droite
	const FString Version = TEXT("v0.1.0-alpha — UE 5.8 — M.E.G. CONFIDENTIEL");
	DrawText(Version, CRT_AmberDim, W - Version.Len() * 6.5f, H - 30.0f, nullptr, 0.75f);
}

// ─── MENU PRINCIPAL ─────────────────────────────────────────────────────────
void ALiminalMainMenuHUD::DrawMainMenu(float W, float H)
{
	const float FrameW = W * 0.45f;
	const float FrameH = H * 0.65f;
	const float FrameX = (W - FrameW) * 0.5f;
	const float FrameY = (H - FrameH) * 0.5f;

	// Fond terminal
	DrawRect(CRT_Background, FrameX, FrameY, FrameW, FrameH);

	// En-tete
	const FString Header = TEXT("═══ TERMINAL M.E.G. — MENU PRINCIPAL ═══");
	const float Scale = FMath::Clamp(W / 1920.0f, 0.5f, 1.5f);
	DrawText(Header, CRT_AmberBright, FrameX + 20.0f * Scale, FrameY + 15.0f * Scale, nullptr, Scale);

	// Ligne separatrice
	DrawRect(CRT_AmberDim, FrameX + 15.0f, FrameY + 40.0f * Scale, FrameW - 30.0f, 1.0f);

	// Boutons du menu
	const float BtnX = FrameX + 40.0f;
	const float BtnW = FrameW - 80.0f;
	const float BtnH = 42.0f * Scale;
	float BtnY = FrameY + 60.0f * Scale;
	const float BtnSpacing = BtnH + 10.0f * Scale;

	MaxButtonCount = 6;

	DrawMenuButton(BtnX, BtnY, BtnW, BtnH, TEXT("> NOUVELLE EXPEDITION"), 0, SelectedButtonIndex == 0);
	BtnY += BtnSpacing;
	DrawMenuButton(BtnX, BtnY, BtnW, BtnH, TEXT("> CONTINUER"), 1, SelectedButtonIndex == 1);
	BtnY += BtnSpacing;
	DrawMenuButton(BtnX, BtnY, BtnW, BtnH, TEXT("> HEBERGER UNE PARTIE"), 2, SelectedButtonIndex == 2);
	BtnY += BtnSpacing;
	DrawMenuButton(BtnX, BtnY, BtnW, BtnH, TEXT("> REJOINDRE UNE PARTIE"), 3, SelectedButtonIndex == 3);
	BtnY += BtnSpacing;
	DrawMenuButton(BtnX, BtnY, BtnW, BtnH, TEXT("> OPTIONS"), 4, SelectedButtonIndex == 4);
	BtnY += BtnSpacing;
	DrawMenuButton(BtnX, BtnY, BtnW, BtnH, TEXT("> QUITTER"), 5, SelectedButtonIndex == 5);

	// Credits en bas
	BtnY += BtnSpacing;
	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		const FString Credits = FString::Printf(TEXT("Banque M.E.G. : %d credits — Cycle %d — Dette : %d"),
			GI->GetTotalCredits(), GI->GetActiveQuotaCycle(), GI->GetSaveData().CurrentDebt);
		DrawText(Credits, CRT_Green, FrameX + 20.0f, FrameY + FrameH - 35.0f * Scale, nullptr, Scale * 0.75f);
	}
}

// ─── ECRAN SETTINGS ─────────────────────────────────────────────────────────
void ALiminalMainMenuHUD::DrawSettingsScreen(float W, float H)
{
	const float FrameW = W * 0.6f;
	const float FrameH = H * 0.75f;
	const float FrameX = (W - FrameW) * 0.5f;
	const float FrameY = (H - FrameH) * 0.5f;
	const float Scale = FMath::Clamp(W / 1920.0f, 0.5f, 1.5f);

	DrawRect(CRT_Background, FrameX, FrameY, FrameW, FrameH);

	// En-tete
	const FString Header = TEXT("═══ PARAMETRES SYSTEME ═══");
	DrawText(Header, CRT_AmberBright, FrameX + 20.0f * Scale, FrameY + 15.0f * Scale, nullptr, Scale);

	// Onglets
	TArray<FString> Tabs = { TEXT("VIDEO"), TEXT("AUDIO"), TEXT("GAMEPLAY"), TEXT("CONTROLES") };
	DrawTabBar(FrameX + 15.0f, FrameY + 45.0f * Scale, FrameW - 30.0f, Tabs, static_cast<int32>(CurrentSettingsTab));

	const float ContentY = FrameY + 90.0f * Scale;
	const float ContentX = FrameX + 30.0f;
	const float ContentW = FrameW - 60.0f;
	float LineY = ContentY;
	const float LineH = 35.0f * Scale;

	switch (CurrentSettingsTab)
	{
	case ESettingsTab::Video:
	{
		MaxButtonCount = 5;

		// Resolution
		TArray<FString> Resolutions = { TEXT("1280x720"), TEXT("1920x1080"), TEXT("2560x1440"), TEXT("3840x2160") };
		const FString ResLabel = FString::Printf(TEXT("Resolution : < %s >"),
			*Resolutions[FMath::Clamp(PendingResolutionIndex, 0, Resolutions.Num() - 1)]);
		DrawMenuButton(ContentX, LineY, ContentW, LineH, ResLabel, 0, SelectedButtonIndex == 0);
		LineY += LineH + 8.0f;

		// Plein ecran
		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Plein Ecran : %s"), bPendingFullscreen ? TEXT("[OUI]") : TEXT("[NON]")),
			1, SelectedButtonIndex == 1);
		LineY += LineH + 8.0f;

		// Qualite graphique
		TArray<FString> QualityLabels = { TEXT("Basse"), TEXT("Moyenne"), TEXT("Haute"), TEXT("Epique"), TEXT("Cinematique") };
		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Qualite : < %s >"),
				*QualityLabels[FMath::Clamp(PendingQualityLevel, 0, QualityLabels.Num() - 1)]),
			2, SelectedButtonIndex == 2);
		LineY += LineH + 8.0f;

		// V-Sync
		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("V-Sync : %s"), bPendingVSync ? TEXT("[OUI]") : TEXT("[NON]")),
			3, SelectedButtonIndex == 3);
		LineY += LineH + 8.0f;

		// FOV
		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Champ de Vision (FOV) : < %.0f >"), PendingFOV),
			4, SelectedButtonIndex == 4);
		LineY += LineH + 8.0f;
		break;
	}
	case ESettingsTab::Audio:
	{
		MaxButtonCount = 4;

		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Volume General : < %.0f%% >"), PendingMasterVolume * 100.0f),
			0, SelectedButtonIndex == 0);
		LineY += LineH + 8.0f;

		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Bruitages (SFX) : < %.0f%% >"), PendingSFXVolume * 100.0f),
			1, SelectedButtonIndex == 1);
		LineY += LineH + 8.0f;

		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Musique / Ambiance : < %.0f%% >"), PendingMusicVolume * 100.0f),
			2, SelectedButtonIndex == 2);
		LineY += LineH + 8.0f;

		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Voix (VOIP) : < %.0f%% >"), PendingVoiceVolume * 100.0f),
			3, SelectedButtonIndex == 3);
		break;
	}
	case ESettingsTab::Gameplay:
	{
		MaxButtonCount = 2;

		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Sensibilite Souris : < %.1f >"), PendingMouseSensitivity),
			0, SelectedButtonIndex == 0);
		LineY += LineH + 8.0f;

		DrawMenuButton(ContentX, LineY, ContentW, LineH,
			FString::Printf(TEXT("Inverser Axe Y : %s"), bPendingInvertY ? TEXT("[OUI]") : TEXT("[NON]")),
			1, SelectedButtonIndex == 1);
		break;
	}
	case ESettingsTab::Controls:
	{
		MaxButtonCount = 0;
		const TArray<TPair<FString, FString>> Bindings = {
			{TEXT("Deplacement"), TEXT("ZQSD / WASD")},
			{TEXT("Regarder"), TEXT("Souris")},
			{TEXT("Sprint"), TEXT("Shift")},
			{TEXT("Sauter"), TEXT("Espace")},
			{TEXT("Ramasser / Lacher"), TEXT("E")},
			{TEXT("Utiliser Outil"), TEXT("Click Gauche")},
			{TEXT("Changer Outil"), TEXT("Tab / Molette")},
			{TEXT("Deployer Outil"), TEXT("G")},
			{TEXT("Lampe Frontale"), TEXT("T")},
			{TEXT("Manuel de Terrain"), TEXT("M")}
		};

		for (const auto& Pair : Bindings)
		{
			DrawText(FString::Printf(TEXT("%-24s %s"), *Pair.Key, *Pair.Value),
				CRT_Amber, ContentX, LineY, nullptr, Scale * 0.85f);
			LineY += LineH * 0.8f;
		}
		break;
	}
	}

	// Boutons bas de page
	const float FooterY = FrameY + FrameH - 40.0f * Scale;
	DrawText(TEXT("[Echap] Retour    [←→] Onglet    [↑↓] Navigation    [Entree] Confirmer"),
		CRT_AmberDim, FrameX + 20.0f, FooterY, nullptr, Scale * 0.7f);
}

// ─── HOST LOBBY ─────────────────────────────────────────────────────────────
void ALiminalMainMenuHUD::DrawHostLobby(float W, float H)
{
	const float FrameW = W * 0.55f;
	const float FrameH = H * 0.65f;
	const float FrameX = (W - FrameW) * 0.5f;
	const float FrameY = (H - FrameH) * 0.5f;
	const float Scale = FMath::Clamp(W / 1920.0f, 0.5f, 1.5f);

	DrawRect(CRT_Background, FrameX, FrameY, FrameW, FrameH);

	const FString Header = TEXT("═══ HEBERGER UNE EXPEDITION ═══");
	DrawText(Header, CRT_AmberBright, FrameX + 20.0f * Scale, FrameY + 15.0f * Scale, nullptr, Scale);

	const float ContentX = FrameX + 30.0f;
	const float ContentW = FrameW - 60.0f;
	float LineY = FrameY + 55.0f * Scale;
	const float LineH = 38.0f * Scale;

	MaxButtonCount = 4;

	// Biome selection
	const TArray<FString> BiomeNames = {
		TEXT("Niv.0 Lobby Jaune"), TEXT("Niv.1 Zone Habitable"), TEXT("Niv.2 Pipe Dreams"),
		TEXT("Niv.3 Station Electrique"), TEXT("Niv.4 Bureaux Abandonnes"), TEXT("Niv.6 Lights Out"),
		TEXT("Niv.8 Cavernes"), TEXT("Niv.9 Faubourg Obscur"), TEXT("Niv.10 Champs de Ble"),
		TEXT("Niv.37 Poolrooms"), TEXT("Niv.! Run For Your Life")
	};
	const int32 ClampedBiome = FMath::Clamp(HostSelectedBiomeIndex, 0, BiomeNames.Num() - 1);
	DrawMenuButton(ContentX, LineY, ContentW, LineH,
		FString::Printf(TEXT("Biome : < %s >"), *BiomeNames[ClampedBiome]),
		0, SelectedButtonIndex == 0);
	LineY += LineH + 10.0f;

	// Map scale
	const TArray<FString> ScaleNames = { TEXT("Compact (8 salles)"), TEXT("Standard (16 salles)"),
		TEXT("Grand (24 salles)"), TEXT("Mega (36 salles)") };
	const int32 ClampedScale = FMath::Clamp(HostSelectedScaleIndex, 0, ScaleNames.Num() - 1);
	DrawMenuButton(ContentX, LineY, ContentW, LineH,
		FString::Printf(TEXT("Echelle : < %s >"), *ScaleNames[ClampedScale]),
		1, SelectedButtonIndex == 1);
	LineY += LineH + 10.0f;

	// Max players
	DrawMenuButton(ContentX, LineY, ContentW, LineH,
		FString::Printf(TEXT("Joueurs Max : < %d >"), HostMaxPlayers),
		2, SelectedButtonIndex == 2);
	LineY += LineH + 20.0f;

	// Launch button
	DrawMenuButton(ContentX, LineY, ContentW, LineH + 8.0f,
		TEXT(">>> LANCER L'INCURSION <<<"), 3, SelectedButtonIndex == 3);

	// Footer
	const float FooterY = FrameY + FrameH - 40.0f * Scale;
	DrawText(TEXT("[Echap] Retour    [←→] Modifier    [Entree] Confirmer"),
		CRT_AmberDim, FrameX + 20.0f, FooterY, nullptr, Scale * 0.7f);
}

// ─── JOIN LOBBY ─────────────────────────────────────────────────────────────
void ALiminalMainMenuHUD::DrawJoinLobby(float W, float H)
{
	const float FrameW = W * 0.5f;
	const float FrameH = H * 0.45f;
	const float FrameX = (W - FrameW) * 0.5f;
	const float FrameY = (H - FrameH) * 0.5f;
	const float Scale = FMath::Clamp(W / 1920.0f, 0.5f, 1.5f);

	DrawRect(CRT_Background, FrameX, FrameY, FrameW, FrameH);

	const FString Header = TEXT("═══ REJOINDRE UNE EXPEDITION ═══");
	DrawText(Header, CRT_AmberBright, FrameX + 20.0f * Scale, FrameY + 15.0f * Scale, nullptr, Scale);

	const float ContentX = FrameX + 30.0f;
	const float ContentW = FrameW - 60.0f;
	float LineY = FrameY + 65.0f * Scale;
	const float LineH = 38.0f * Scale;

	MaxButtonCount = 2;

	// Champ IP
	DrawText(TEXT("Adresse IP de l'hote :"), CRT_Amber, ContentX, LineY, nullptr, Scale * 0.9f);
	LineY += 30.0f * Scale;

	// Zone de texte IP avec curseur clignotant
	DrawRect(FLinearColor(0.05f, 0.05f, 0.04f, 0.9f), ContentX, LineY, ContentW, LineH);
	const float CursorBlink = FMath::Sin(TitlePulseTimer * 5.0f) > 0.0f ? 1.0f : 0.0f;
	FString DisplayIP = JoinIPAddress;
	if (SelectedButtonIndex == 0 && CursorBlink > 0.5f)
	{
		DisplayIP += TEXT("_");
	}
	DrawText(DisplayIP, SelectedButtonIndex == 0 ? CRT_AmberBright : CRT_Amber,
		ContentX + 10.0f, LineY + 6.0f * Scale, nullptr, Scale * 1.2f);
	if (SelectedButtonIndex == 0)
	{
		DrawRect(CRT_AmberBright, ContentX, LineY + LineH - 2.0f, ContentW, 2.0f);
	}
	LineY += LineH + 20.0f;

	// Connect button
	DrawMenuButton(ContentX, LineY, ContentW, LineH + 8.0f,
		TEXT(">>> CONNEXION <<<"), 1, SelectedButtonIndex == 1);

	// Footer
	const float FooterY = FrameY + FrameH - 35.0f * Scale;
	DrawText(TEXT("[Echap] Retour    [Entree] Connexion"),
		CRT_AmberDim, FrameX + 20.0f, FooterY, nullptr, Scale * 0.7f);
}

// ─── COMPOSANTS VISUELS CRT ────────────────────────────────────────────────

void ALiminalMainMenuHUD::DrawCRTFrame(float W, float H)
{
	const float Border = 3.0f;
	// Vignette assombrie sur les bords
	const FLinearColor VignetteColor(0.0f, 0.0f, 0.0f, 0.4f);
	DrawRect(VignetteColor, 0.0f, 0.0f, W, Border * 4.0f);
	DrawRect(VignetteColor, 0.0f, H - Border * 4.0f, W, Border * 4.0f);
	DrawRect(VignetteColor, 0.0f, 0.0f, Border * 4.0f, H);
	DrawRect(VignetteColor, W - Border * 4.0f, 0.0f, Border * 4.0f, H);

	// Cadre fin ambre
	DrawRect(CRT_AmberDim, 0.0f, 0.0f, W, Border);
	DrawRect(CRT_AmberDim, 0.0f, H - Border, W, Border);
	DrawRect(CRT_AmberDim, 0.0f, 0.0f, Border, H);
	DrawRect(CRT_AmberDim, W - Border, 0.0f, Border, H);
}

void ALiminalMainMenuHUD::DrawScanlines(float W, float H)
{
	// Lignes de balayage horizontales subtiles
	for (float Y = FMath::Fmod(ScanlineOffset, 4.0f); Y < H; Y += 4.0f)
	{
		DrawRect(CRT_Scanline, 0.0f, Y, W, 1.0f);
	}

	// Occasional VHS tracking glitch line
	if (FMath::Sin(CRTFlickerTimer) > 0.95f)
	{
		const float GlitchY = FMath::Fmod(ScanlineOffset * 3.7f, H);
		DrawRect(FLinearColor(CRT_Amber.R, CRT_Amber.G, CRT_Amber.B, 0.15f),
			0.0f, GlitchY, W, 3.0f);
	}
}

void ALiminalMainMenuHUD::DrawMenuButton(float X, float Y, float Width, float Height,
	const FString& Label, int32 ButtonIndex, bool bSelected)
{
	const FLinearColor BgColor = bSelected
		? FLinearColor(CRT_Amber.R * 0.15f, CRT_Amber.G * 0.15f, CRT_Amber.B * 0.15f, 0.5f)
		: FLinearColor(0.03f, 0.03f, 0.02f, 0.3f);
	const FLinearColor TextColor = bSelected ? CRT_AmberBright : CRT_AmberDim;

	DrawRect(BgColor, X, Y, Width, Height);

	// Bordure gauche d'accentuation si selectionne
	if (bSelected)
	{
		DrawRect(CRT_AmberBright, X, Y, 3.0f, Height);
		// Indicateur ">" anime
		const float PulseOffset = FMath::Sin(TitlePulseTimer * 4.0f) * 3.0f;
		DrawText(TEXT("▶"), CRT_AmberBright, X + 8.0f + PulseOffset, Y + Height * 0.15f);
	}

	const float Scale = FMath::Clamp(Height / 42.0f, 0.5f, 1.5f);
	DrawText(Label, TextColor, X + 28.0f, Y + Height * 0.2f, nullptr, Scale * 0.9f);
}

void ALiminalMainMenuHUD::DrawTabBar(float X, float Y, float Width, const TArray<FString>& TabLabels, int32 ActiveTab)
{
	const float TabW = Width / FMath::Max(1, TabLabels.Num());

	for (int32 i = 0; i < TabLabels.Num(); ++i)
	{
		const float TabX = X + TabW * i;
		const bool bActive = (i == ActiveTab);

		DrawRect(bActive ? FLinearColor(0.08f, 0.06f, 0.02f, 0.7f) : FLinearColor(0.02f, 0.02f, 0.015f, 0.4f),
			TabX, Y, TabW - 2.0f, 30.0f);

		if (bActive)
		{
			DrawRect(CRT_AmberBright, TabX, Y + 27.0f, TabW - 2.0f, 3.0f);
		}

		DrawText(TabLabels[i], bActive ? CRT_AmberBright : CRT_AmberDim,
			TabX + 10.0f, Y + 6.0f, nullptr, 0.85f);
	}
}

// ─── INPUT ──────────────────────────────────────────────────────────────────
void ALiminalMainMenuHUD::HandleMenuInput()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || InputCooldown > 0.0f || bTransitioning)
	{
		return;
	}

	// Ecran titre : n'importe quelle touche
	if (CurrentScreen == EMenuScreen::Title)
	{
		if (PC->WasInputKeyJustPressed(EKeys::AnyKey) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
		{
			CurrentScreen = EMenuScreen::MainMenu;
			InputCooldown = 0.3f;
		}
		return;
	}

	// Navigation haut/bas
	if (PC->WasInputKeyJustPressed(EKeys::Up) || PC->WasInputKeyJustPressed(EKeys::W) ||
		PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Up))
	{
		NavigateUp();
		InputCooldown = 0.15f;
	}
	else if (PC->WasInputKeyJustPressed(EKeys::Down) || PC->WasInputKeyJustPressed(EKeys::S) ||
		PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Down))
	{
		NavigateDown();
		InputCooldown = 0.15f;
	}

	// Navigation gauche/droite (settings values, tabs)
	if (PC->WasInputKeyJustPressed(EKeys::Left) || PC->WasInputKeyJustPressed(EKeys::A) ||
		PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Left))
	{
		NavigateLeft();
		InputCooldown = 0.15f;
	}
	else if (PC->WasInputKeyJustPressed(EKeys::Right) || PC->WasInputKeyJustPressed(EKeys::D) ||
		PC->WasInputKeyJustPressed(EKeys::Gamepad_DPad_Right))
	{
		NavigateRight();
		InputCooldown = 0.15f;
	}

	// Confirmer
	if (PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
	{
		ConfirmSelection();
		InputCooldown = 0.25f;
	}

	// Retour
	if (PC->WasInputKeyJustPressed(EKeys::Escape) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Right))
	{
		GoBack();
		InputCooldown = 0.25f;
	}
}

void ALiminalMainMenuHUD::NavigateUp()
{
	if (MaxButtonCount > 0)
	{
		SelectedButtonIndex = (SelectedButtonIndex - 1 + MaxButtonCount) % MaxButtonCount;
	}
}

void ALiminalMainMenuHUD::NavigateDown()
{
	if (MaxButtonCount > 0)
	{
		SelectedButtonIndex = (SelectedButtonIndex + 1) % MaxButtonCount;
	}
}

void ALiminalMainMenuHUD::NavigateLeft()
{
	if (CurrentScreen == EMenuScreen::Settings)
	{
		if (MaxButtonCount == 0 || SelectedButtonIndex < 0)
		{
			// Switch settings tab
			int32 TabIdx = static_cast<int32>(CurrentSettingsTab);
			TabIdx = (TabIdx - 1 + 4) % 4;
			CurrentSettingsTab = static_cast<ESettingsTab>(TabIdx);
			SelectedButtonIndex = 0;
			return;
		}

		// Modify value left
		switch (CurrentSettingsTab)
		{
		case ESettingsTab::Video:
			if (SelectedButtonIndex == 0) PendingResolutionIndex = FMath::Max(0, PendingResolutionIndex - 1);
			else if (SelectedButtonIndex == 1) bPendingFullscreen = !bPendingFullscreen;
			else if (SelectedButtonIndex == 2) PendingQualityLevel = FMath::Max(0, PendingQualityLevel - 1);
			else if (SelectedButtonIndex == 3) bPendingVSync = !bPendingVSync;
			else if (SelectedButtonIndex == 4) PendingFOV = FMath::Clamp(PendingFOV - 5.0f, 60.0f, 120.0f);
			break;
		case ESettingsTab::Audio:
			if (SelectedButtonIndex == 0) PendingMasterVolume = FMath::Clamp(PendingMasterVolume - 0.05f, 0.0f, 1.0f);
			else if (SelectedButtonIndex == 1) PendingSFXVolume = FMath::Clamp(PendingSFXVolume - 0.05f, 0.0f, 1.0f);
			else if (SelectedButtonIndex == 2) PendingMusicVolume = FMath::Clamp(PendingMusicVolume - 0.05f, 0.0f, 1.0f);
			else if (SelectedButtonIndex == 3) PendingVoiceVolume = FMath::Clamp(PendingVoiceVolume - 0.05f, 0.0f, 1.0f);
			break;
		case ESettingsTab::Gameplay:
			if (SelectedButtonIndex == 0) PendingMouseSensitivity = FMath::Clamp(PendingMouseSensitivity - 0.1f, 0.1f, 5.0f);
			else if (SelectedButtonIndex == 1) bPendingInvertY = !bPendingInvertY;
			break;
		default: break;
		}
	}
	else if (CurrentScreen == EMenuScreen::HostLobby)
	{
		if (SelectedButtonIndex == 0) HostSelectedBiomeIndex = FMath::Max(0, HostSelectedBiomeIndex - 1);
		else if (SelectedButtonIndex == 1) HostSelectedScaleIndex = FMath::Max(0, HostSelectedScaleIndex - 1);
		else if (SelectedButtonIndex == 2) HostMaxPlayers = FMath::Clamp(HostMaxPlayers - 1, 1, 4);
	}
}

void ALiminalMainMenuHUD::NavigateRight()
{
	if (CurrentScreen == EMenuScreen::Settings)
	{
		if (MaxButtonCount == 0 || SelectedButtonIndex < 0)
		{
			int32 TabIdx = static_cast<int32>(CurrentSettingsTab);
			TabIdx = (TabIdx + 1) % 4;
			CurrentSettingsTab = static_cast<ESettingsTab>(TabIdx);
			SelectedButtonIndex = 0;
			return;
		}

		switch (CurrentSettingsTab)
		{
		case ESettingsTab::Video:
			if (SelectedButtonIndex == 0) PendingResolutionIndex = FMath::Min(3, PendingResolutionIndex + 1);
			else if (SelectedButtonIndex == 1) bPendingFullscreen = !bPendingFullscreen;
			else if (SelectedButtonIndex == 2) PendingQualityLevel = FMath::Min(4, PendingQualityLevel + 1);
			else if (SelectedButtonIndex == 3) bPendingVSync = !bPendingVSync;
			else if (SelectedButtonIndex == 4) PendingFOV = FMath::Clamp(PendingFOV + 5.0f, 60.0f, 120.0f);
			break;
		case ESettingsTab::Audio:
			if (SelectedButtonIndex == 0) PendingMasterVolume = FMath::Clamp(PendingMasterVolume + 0.05f, 0.0f, 1.0f);
			else if (SelectedButtonIndex == 1) PendingSFXVolume = FMath::Clamp(PendingSFXVolume + 0.05f, 0.0f, 1.0f);
			else if (SelectedButtonIndex == 2) PendingMusicVolume = FMath::Clamp(PendingMusicVolume + 0.05f, 0.0f, 1.0f);
			else if (SelectedButtonIndex == 3) PendingVoiceVolume = FMath::Clamp(PendingVoiceVolume + 0.05f, 0.0f, 1.0f);
			break;
		case ESettingsTab::Gameplay:
			if (SelectedButtonIndex == 0) PendingMouseSensitivity = FMath::Clamp(PendingMouseSensitivity + 0.1f, 0.1f, 5.0f);
			else if (SelectedButtonIndex == 1) bPendingInvertY = !bPendingInvertY;
			break;
		default: break;
		}
	}
	else if (CurrentScreen == EMenuScreen::HostLobby)
	{
		if (SelectedButtonIndex == 0) HostSelectedBiomeIndex = FMath::Min(10, HostSelectedBiomeIndex + 1);
		else if (SelectedButtonIndex == 1) HostSelectedScaleIndex = FMath::Min(3, HostSelectedScaleIndex + 1);
		else if (SelectedButtonIndex == 2) HostMaxPlayers = FMath::Clamp(HostMaxPlayers + 1, 1, 4);
	}
}

void ALiminalMainMenuHUD::ConfirmSelection()
{
	switch (CurrentScreen)
	{
	case EMenuScreen::MainMenu:
		switch (SelectedButtonIndex)
		{
		case 0: OnNewExpedition(); break;
		case 1: OnContinueGame(); break;
		case 2: OnHostGame(); break;
		case 3: OnJoinGame(); break;
		case 4: OnOpenSettings(); break;
		case 5: OnQuitGame(); break;
		}
		break;

	case EMenuScreen::Settings:
		// Entree sur l'ecran settings = appliquer et sauvegarder
		ApplyVideoSettings();
		ApplyAudioSettings();
		break;

	case EMenuScreen::HostLobby:
		if (SelectedButtonIndex == 3)
		{
			OnLaunchMission();
		}
		break;

	case EMenuScreen::JoinLobby:
		if (SelectedButtonIndex == 1)
		{
			OnConnectToHost();
		}
		break;

	default: break;
	}
}

void ALiminalMainMenuHUD::GoBack()
{
	switch (CurrentScreen)
	{
	case EMenuScreen::Settings:
	case EMenuScreen::HostLobby:
	case EMenuScreen::JoinLobby:
		CurrentScreen = EMenuScreen::MainMenu;
		SelectedButtonIndex = 0;
		break;
	case EMenuScreen::MainMenu:
		CurrentScreen = EMenuScreen::Title;
		break;
	default: break;
	}
}

// ─── ACTIONS ────────────────────────────────────────────────────────────────

void ALiminalMainMenuHUD::OnNewExpedition()
{
	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		// Reset save data for new expedition
		GI->LoadGameFromDisk(); // Ensure fresh load
	}
	CurrentScreen = EMenuScreen::HostLobby;
	SelectedButtonIndex = 0;
}

void ALiminalMainMenuHUD::OnContinueGame()
{
	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->LoadGameFromDisk();
		GI->ReturnToHub();
	}
}

void ALiminalMainMenuHUD::OnHostGame()
{
	CurrentScreen = EMenuScreen::HostLobby;
	SelectedButtonIndex = 0;
}

void ALiminalMainMenuHUD::OnJoinGame()
{
	CurrentScreen = EMenuScreen::JoinLobby;
	SelectedButtonIndex = 0;
}

void ALiminalMainMenuHUD::OnOpenSettings()
{
	LoadCurrentSettings();
	CurrentScreen = EMenuScreen::Settings;
	CurrentSettingsTab = ESettingsTab::Video;
	SelectedButtonIndex = 0;
}

void ALiminalMainMenuHUD::OnQuitGame()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		UKismetSystemLibrary::QuitGame(GetWorld(), PC, EQuitPreference::Quit, false);
	}
}

void ALiminalMainMenuHUD::OnLaunchMission()
{
	if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
	{
		GI->SetSelectedBiome(static_cast<ELevelBiome>(FMath::Clamp(HostSelectedBiomeIndex, 0, 10)));
		GI->SetSelectedMapScale(HostSelectedScaleIndex);

		// Desactiver le curseur avant de lancer
		if (APlayerController* PC = GetOwningPlayerController())
		{
			PC->bShowMouseCursor = false;
			PC->SetInputMode(FInputModeGameOnly());
		}

		GI->ReturnToHub();
	}
}

void ALiminalMainMenuHUD::OnConnectToHost()
{
	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = false;
		PC->SetInputMode(FInputModeGameOnly());

		// Travel client vers le serveur hote
		const FString TravelURL = FString::Printf(TEXT("%s"), *JoinIPAddress);
		PC->ClientTravel(TravelURL, TRAVEL_Absolute);
	}
}

// ─── SETTINGS PERSISTENCE ──────────────────────────────────────────────────

void ALiminalMainMenuHUD::ApplyVideoSettings()
{
	UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr;
	if (!Settings)
	{
		return;
	}

	// Resolution
	const TArray<FIntPoint> Resolutions = {
		{1280, 720}, {1920, 1080}, {2560, 1440}, {3840, 2160}
	};
	const int32 ResIdx = FMath::Clamp(PendingResolutionIndex, 0, Resolutions.Num() - 1);
	Settings->SetScreenResolution(Resolutions[ResIdx]);

	// Fullscreen
	Settings->SetFullscreenMode(bPendingFullscreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);

	// Quality
	Settings->SetOverallScalabilityLevel(PendingQualityLevel);

	// V-Sync
	Settings->SetVSyncEnabled(bPendingVSync);

	Settings->ApplySettings(false);
	Settings->SaveSettings();

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Video settings applied: %dx%d, Fullscreen=%d, Quality=%d, VSync=%d"),
		Resolutions[ResIdx].X, Resolutions[ResIdx].Y, bPendingFullscreen, PendingQualityLevel, bPendingVSync);
}

void ALiminalMainMenuHUD::ApplyAudioSettings()
{
	// Les volumes sont stockes et peuvent etre appliques via Sound Mix ou Sound Class
	// Pour l'instant on les sauvegarde dans le config
	if (GConfig)
	{
		GConfig->SetFloat(TEXT("Audio"), TEXT("MasterVolume"), PendingMasterVolume, GGameUserSettingsIni);
		GConfig->SetFloat(TEXT("Audio"), TEXT("SFXVolume"), PendingSFXVolume, GGameUserSettingsIni);
		GConfig->SetFloat(TEXT("Audio"), TEXT("MusicVolume"), PendingMusicVolume, GGameUserSettingsIni);
		GConfig->SetFloat(TEXT("Audio"), TEXT("VoiceVolume"), PendingVoiceVolume, GGameUserSettingsIni);
		GConfig->SetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), PendingMouseSensitivity, GGameUserSettingsIni);
		GConfig->SetBool(TEXT("Gameplay"), TEXT("InvertY"), bPendingInvertY, GGameUserSettingsIni);
		GConfig->SetFloat(TEXT("Gameplay"), TEXT("FOV"), PendingFOV, GGameUserSettingsIni);
		GConfig->Flush(false, GGameUserSettingsIni);
	}

	UE_LOG(LogTemp, Log, TEXT("[MainMenu] Audio/Gameplay settings saved: Master=%.0f%%, SFX=%.0f%%, Music=%.0f%%, Sens=%.1f"),
		PendingMasterVolume * 100.0f, PendingSFXVolume * 100.0f, PendingMusicVolume * 100.0f, PendingMouseSensitivity);
}

void ALiminalMainMenuHUD::LoadCurrentSettings()
{
	if (UGameUserSettings* Settings = GEngine ? GEngine->GetGameUserSettings() : nullptr)
	{
		const FIntPoint CurrentRes = Settings->GetScreenResolution();
		if (CurrentRes.X >= 3840) PendingResolutionIndex = 3;
		else if (CurrentRes.X >= 2560) PendingResolutionIndex = 2;
		else if (CurrentRes.X >= 1920) PendingResolutionIndex = 1;
		else PendingResolutionIndex = 0;

		bPendingFullscreen = (Settings->GetFullscreenMode() == EWindowMode::Fullscreen);
		PendingQualityLevel = Settings->GetOverallScalabilityLevel();
		if (PendingQualityLevel < 0) PendingQualityLevel = 2;
		bPendingVSync = Settings->IsVSyncEnabled();
	}

	if (GConfig)
	{
		GConfig->GetFloat(TEXT("Audio"), TEXT("MasterVolume"), PendingMasterVolume, GGameUserSettingsIni);
		GConfig->GetFloat(TEXT("Audio"), TEXT("SFXVolume"), PendingSFXVolume, GGameUserSettingsIni);
		GConfig->GetFloat(TEXT("Audio"), TEXT("MusicVolume"), PendingMusicVolume, GGameUserSettingsIni);
		GConfig->GetFloat(TEXT("Audio"), TEXT("VoiceVolume"), PendingVoiceVolume, GGameUserSettingsIni);
		GConfig->GetFloat(TEXT("Gameplay"), TEXT("MouseSensitivity"), PendingMouseSensitivity, GGameUserSettingsIni);
		GConfig->GetBool(TEXT("Gameplay"), TEXT("InvertY"), bPendingInvertY, GGameUserSettingsIni);
		GConfig->GetFloat(TEXT("Gameplay"), TEXT("FOV"), PendingFOV, GGameUserSettingsIni);
	}
}
