#include "UI/LiminalDebriefHUD.h"

#include "Engine/Canvas.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Data/LiminalGameInstance.h"

const FLinearColor ALiminalDebriefHUD::CRT_Amber      = FLinearColor(1.0f, 0.75f, 0.2f, 1.0f);
const FLinearColor ALiminalDebriefHUD::CRT_AmberDim   = FLinearColor(0.6f, 0.45f, 0.12f, 0.6f);
const FLinearColor ALiminalDebriefHUD::CRT_AmberBright = FLinearColor(1.0f, 0.85f, 0.35f, 1.0f);
const FLinearColor ALiminalDebriefHUD::CRT_Green      = FLinearColor(0.2f, 1.0f, 0.3f, 1.0f);
const FLinearColor ALiminalDebriefHUD::CRT_Red        = FLinearColor(1.0f, 0.25f, 0.2f, 1.0f);
const FLinearColor ALiminalDebriefHUD::CRT_Background = FLinearColor(0.02f, 0.02f, 0.015f, 0.92f);

ALiminalDebriefHUD::ALiminalDebriefHUD()
{
}

void ALiminalDebriefHUD::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = GetOwningPlayerController())
	{
		PC->bShowMouseCursor = true;
		PC->SetInputMode(FInputModeUIOnly());
	}
}

void ALiminalDebriefHUD::SetDebriefData(const FMissionDebriefData& InData)
{
	DebriefData = InData;
	RevealTimer = 0.0f;
	TypewriterProgress = 0.0f;
	StampTimer = 0.0f;
	bStampRevealed = false;
	bReadyToReturn = false;
}

void ALiminalDebriefHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas)
	{
		return;
	}

	const float W = Canvas->ClipX;
	const float H = Canvas->ClipY;
	const float DeltaSeconds = GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;

	// Timers
	RevealTimer += DeltaSeconds;
	PulseTimer += DeltaSeconds;
	ScanlineOffset = FMath::Fmod(ScanlineOffset + DeltaSeconds * 60.0f, H);
	TypewriterProgress += DeltaSeconds * 30.0f; // Caracteres par seconde
	if (InputCooldown > 0.0f) InputCooldown -= DeltaSeconds;

	// Stamp reveal apres 3 secondes
	if (RevealTimer > 3.0f && !bStampRevealed)
	{
		StampTimer += DeltaSeconds * 2.0f;
		if (StampTimer >= 1.0f)
		{
			bStampRevealed = true;
			bReadyToReturn = true;
		}
	}

	// Fond
	DrawRect(FLinearColor::Black, 0.0f, 0.0f, W, H);

	// Rapport
	DrawDebriefReport(W, H);

	// Scanlines
	for (float Y = FMath::Fmod(ScanlineOffset, 4.0f); Y < H; Y += 4.0f)
	{
		DrawRect(FLinearColor(0.0f, 0.0f, 0.0f, 0.08f), 0.0f, Y, W, 1.0f);
	}

	HandleDebriefInput();
}

void ALiminalDebriefHUD::DrawDebriefReport(float W, float H)
{
	const float FrameW = W * 0.65f;
	const float FrameH = H * 0.85f;
	const float FrameX = (W - FrameW) * 0.5f;
	const float FrameY = (H - FrameH) * 0.5f;
	const float Scale = FMath::Clamp(W / 1920.0f, 0.5f, 1.5f);

	// Fond du formulaire
	DrawRect(CRT_Background, FrameX, FrameY, FrameW, FrameH);

	// Bordure
	DrawRect(CRT_AmberDim, FrameX, FrameY, FrameW, 2.0f);
	DrawRect(CRT_AmberDim, FrameX, FrameY + FrameH - 2.0f, FrameW, 2.0f);
	DrawRect(CRT_AmberDim, FrameX, FrameY, 2.0f, FrameH);
	DrawRect(CRT_AmberDim, FrameX + FrameW - 2.0f, FrameY, 2.0f, FrameH);

	float LineY = FrameY + 15.0f * Scale;
	const float LeftMargin = FrameX + 25.0f;
	const float RightCol = FrameX + FrameW * 0.55f;
	const float LineStep = 24.0f * Scale;

	// Effet machine a ecrire : ne dessine que les N premiers caracteres
	int32 TotalCharsRevealed = static_cast<int32>(TypewriterProgress);
	int32 CharsDrawn = 0;

	auto DrawTyped = [&](const FString& Text, const FLinearColor& Color, float X, float Y, float S = -1.0f)
	{
		if (S < 0.0f) S = Scale;
		const int32 MaxChars = FMath::Max(0, TotalCharsRevealed - CharsDrawn);
		if (MaxChars <= 0) return;
		const FString Visible = Text.Left(FMath::Min(MaxChars, Text.Len()));
		DrawText(Visible, Color, X, Y, nullptr, S);
		CharsDrawn += Text.Len();
	};

	// ─── En-tete ───
	DrawTyped(TEXT("╔══════════════════════════════════════════════════╗"), CRT_AmberDim, LeftMargin, LineY, Scale * 0.7f);
	LineY += LineStep;
	DrawTyped(TEXT("  RAPPORT D'INCURSION — M.E.G. DIVISION RECUPERATION"), CRT_AmberBright, LeftMargin, LineY);
	LineY += LineStep;
	DrawTyped(TEXT("╚══════════════════════════════════════════════════╝"), CRT_AmberDim, LeftMargin, LineY, Scale * 0.7f);
	LineY += LineStep * 1.5f;

	// ─── Infos mission ───
	DrawTyped(FString::Printf(TEXT("Biome d'operation : %s"), *DebriefData.BiomeName),
		CRT_Amber, LeftMargin, LineY);
	LineY += LineStep;

	const int32 Minutes = static_cast<int32>(DebriefData.MissionDurationSeconds) / 60;
	const int32 Seconds = static_cast<int32>(DebriefData.MissionDurationSeconds) % 60;
	DrawTyped(FString::Printf(TEXT("Duree de l'incursion : %02d:%02d"), Minutes, Seconds),
		CRT_Amber, LeftMargin, LineY);
	LineY += LineStep;

	DrawTyped(FString::Printf(TEXT("Cycle de Quota : #%d"), DebriefData.QuotaCycleNumber),
		CRT_Amber, LeftMargin, LineY);
	LineY += LineStep * 1.5f;

	// ─── Separateur ───
	DrawRect(CRT_AmberDim, LeftMargin, LineY, FrameW - 50.0f, 1.0f);
	LineY += LineStep;

	// ─── Economie ───
	DrawTyped(TEXT("════ BILAN ECONOMIQUE ════"), CRT_AmberBright, LeftMargin, LineY);
	LineY += LineStep * 1.2f;

	DrawTyped(FString::Printf(TEXT("Credits extraits     : %d"), DebriefData.TotalCreditsExtracted),
		DebriefData.IsQuotaMet() ? CRT_Green : CRT_Red, LeftMargin, LineY);
	LineY += LineStep;

	DrawTyped(FString::Printf(TEXT("Objectif de quota    : %d"), DebriefData.QuotaTarget),
		CRT_Amber, LeftMargin, LineY);
	LineY += LineStep;

	const int32 Surplus = DebriefData.TotalCreditsExtracted - DebriefData.QuotaTarget;
	if (Surplus >= 0)
	{
		DrawTyped(FString::Printf(TEXT("Excedent             : +%d"), Surplus), CRT_Green, LeftMargin, LineY);
	}
	else
	{
		DrawTyped(FString::Printf(TEXT("Deficit              : %d"), Surplus), CRT_Red, LeftMargin, LineY);
	}
	LineY += LineStep;

	DrawTyped(FString::Printf(TEXT("Dette precedente     : %d"), DebriefData.PreviousDebt),
		DebriefData.PreviousDebt > 0 ? CRT_Red : CRT_AmberDim, LeftMargin, LineY);
	LineY += LineStep;

	DrawTyped(FString::Printf(TEXT("Dette actuelle       : %d"), DebriefData.NewDebt),
		DebriefData.NewDebt > 0 ? CRT_Red : CRT_Green, LeftMargin, LineY);
	LineY += LineStep;

	DrawTyped(FString::Printf(TEXT("Solde bancaire M.E.G.: %d credits"), DebriefData.BankBalanceAfter),
		CRT_AmberBright, LeftMargin, LineY);
	LineY += LineStep * 1.5f;

	// ─── Separateur ───
	DrawRect(CRT_AmberDim, LeftMargin, LineY, FrameW - 50.0f, 1.0f);
	LineY += LineStep;

	// ─── Stats joueurs ───
	DrawTyped(TEXT("════ PERSONNEL D'EXPEDITION ════"), CRT_AmberBright, LeftMargin, LineY);
	LineY += LineStep * 1.2f;

	DrawPlayerStatsTable(LeftMargin, LineY, FrameW - 50.0f, Scale);
	LineY += (DebriefData.PlayerStats.Num() + 1) * LineStep + LineStep;

	// ─── Tampon de verdict ───
	if (RevealTimer > 3.0f)
	{
		DrawStampVerdict(FrameX + FrameW * 0.6f, LineY - LineStep * 3.0f, Scale);
	}

	// ─── Bouton retour ───
	if (bReadyToReturn)
	{
		const float PulseAlpha = (FMath::Sin(PulseTimer * 3.0f) + 1.0f) * 0.5f;
		const FLinearColor ReturnColor(CRT_Amber.R, CRT_Amber.G, CRT_Amber.B, PulseAlpha);
		const FString ReturnText = TEXT("[ ENTREE — RETOUR A LA BASE ALPHA ]");
		DrawText(ReturnText, ReturnColor,
			(W - ReturnText.Len() * 5.5f * Scale) * 0.5f,
			FrameY + FrameH - 35.0f * Scale, nullptr, Scale);
	}
}

void ALiminalDebriefHUD::DrawPlayerStatsTable(float X, float Y, float Width, float Scale)
{
	const float LineH = 22.0f * Scale;
	const float ColName = X;
	const float ColCredits = X + Width * 0.3f;
	const float ColDamage = X + Width * 0.48f;
	const float ColDist = X + Width * 0.65f;
	const float ColStatus = X + Width * 0.82f;

	// En-tete de colonne
	DrawText(TEXT("Nom"), CRT_AmberBright, ColName, Y, nullptr, Scale * 0.75f);
	DrawText(TEXT("Credits"), CRT_AmberBright, ColCredits, Y, nullptr, Scale * 0.75f);
	DrawText(TEXT("Degats"), CRT_AmberBright, ColDamage, Y, nullptr, Scale * 0.75f);
	DrawText(TEXT("Distance"), CRT_AmberBright, ColDist, Y, nullptr, Scale * 0.75f);
	DrawText(TEXT("Statut"), CRT_AmberBright, ColStatus, Y, nullptr, Scale * 0.75f);
	Y += LineH;

	// Ligne separatrice
	DrawRect(CRT_AmberDim, X, Y - 3.0f, Width, 1.0f);

	for (const FPlayerMissionStats& PS : DebriefData.PlayerStats)
	{
		const FLinearColor RowColor = PS.bSurvived ? CRT_Amber : CRT_Red;

		DrawText(PS.PlayerName, RowColor, ColName, Y, nullptr, Scale * 0.7f);
		DrawText(FString::Printf(TEXT("%d"), PS.CreditsCollected), RowColor, ColCredits, Y, nullptr, Scale * 0.7f);
		DrawText(FString::Printf(TEXT("%.0f"), PS.DamageReceived), RowColor, ColDamage, Y, nullptr, Scale * 0.7f);
		DrawText(FString::Printf(TEXT("%.0fm"), PS.DistanceTraveled / 100.0f), RowColor, ColDist, Y, nullptr, Scale * 0.7f);

		const FString Status = PS.bSurvived ? TEXT("EVACUE") : FString::Printf(TEXT("K.I.A. (%s)"), *PS.CauseOfDeath);
		DrawText(Status, PS.bSurvived ? CRT_Green : CRT_Red, ColStatus, Y, nullptr, Scale * 0.65f);
		Y += LineH;
	}
}

void ALiminalDebriefHUD::DrawStampVerdict(float X, float Y, float Scale)
{
	const float StampScale = FMath::Clamp(StampTimer, 0.0f, 1.0f);
	const float StampAlpha = StampScale;

	if (DebriefData.IsQuotaMet())
	{
		// Tampon vert : QUOTA ATTEINT
		const FLinearColor StampColor(0.1f, 0.9f, 0.2f, StampAlpha * 0.85f);
		const FString Line1 = TEXT("╔═══════════════╗");
		const FString Line2 = TEXT("║ QUOTA ATTEINT ║");
		const FString Line3 = TEXT("╚═══════════════╝");

		const float RotatedScale = Scale * 1.3f * StampScale;
		DrawText(Line1, StampColor, X, Y, nullptr, RotatedScale);
		DrawText(Line2, StampColor, X, Y + 22.0f * RotatedScale, nullptr, RotatedScale);
		DrawText(Line3, StampColor, X, Y + 44.0f * RotatedScale, nullptr, RotatedScale);
	}
	else
	{
		// Tampon rouge : DEFICIT
		const FLinearColor StampColor(1.0f, 0.15f, 0.1f, StampAlpha * 0.9f);
		const FString Line1 = TEXT("╔═══════════════════╗");
		const FString Line2 = TEXT("║ DEFICIT — DETTE ║");
		const FString Line3 = TEXT("╚═══════════════════╝");

		const float RotatedScale = Scale * 1.3f * StampScale;
		DrawText(Line1, StampColor, X, Y, nullptr, RotatedScale);
		DrawText(Line2, StampColor, X, Y + 22.0f * RotatedScale, nullptr, RotatedScale);
		DrawText(Line3, StampColor, X, Y + 44.0f * RotatedScale, nullptr, RotatedScale);
	}
}

void ALiminalDebriefHUD::HandleDebriefInput()
{
	APlayerController* PC = GetOwningPlayerController();
	if (!PC || !bReadyToReturn || InputCooldown > 0.0f)
	{
		return;
	}

	if (PC->WasInputKeyJustPressed(EKeys::Enter) || PC->WasInputKeyJustPressed(EKeys::Gamepad_FaceButton_Bottom))
	{
		InputCooldown = 0.5f;

		if (ULiminalGameInstance* GI = Cast<ULiminalGameInstance>(UGameplayStatics::GetGameInstance(this)))
		{
			GI->ReturnToHub();
		}
	}
}
