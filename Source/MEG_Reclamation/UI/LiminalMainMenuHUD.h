#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Data/LiminalGameInstance.h"
#include "LiminalMainMenuHUD.generated.h"

/**
 * Menu Principal du M.E.G. : Reclamation.
 * Dessine sur Canvas sans dependance Blueprint, style terminal CRT analogique.
 *
 * Ecrans :
 * - Title      : Logo + "APPUYER POUR COMMENCER"
 * - MainMenu   : Nouvelle Expedition / Continuer / Heberger / Rejoindre / Options / Quitter
 * - Settings   : Video (resolution, plein ecran, qualite, FOV) / Audio (volumes) / Gameplay (sensibilite)
 * - HostLobby  : Configuration de session (biome, echelle, lancement)
 * - JoinLobby  : Saisie d'adresse IP et connexion
 *
 * Integre UGameUserSettings pour la persistance des parametres graphiques/audio
 * et ULiminalGameInstance pour la gestion des sessions et la sauvegarde.
 */
UENUM()
enum class EMenuScreen : uint8
{
	Title,
	MainMenu,
	Settings,
	HostLobby,
	JoinLobby
};

UENUM()
enum class ESettingsTab : uint8
{
	Video,
	Audio,
	Gameplay,
	Controls
};

UCLASS()
class MEG_RECLAMATION_API ALiminalMainMenuHUD : public AHUD
{
	GENERATED_BODY()

public:
	ALiminalMainMenuHUD();

	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

protected:
	// Ecrans
	void DrawTitleScreen(float W, float H);
	void DrawMainMenu(float W, float H);
	void DrawSettingsScreen(float W, float H);
	void DrawHostLobby(float W, float H);
	void DrawJoinLobby(float W, float H);

	// Composants visuels CRT partages
	void DrawCRTFrame(float W, float H);
	void DrawScanlines(float W, float H);
	void DrawMenuButton(float X, float Y, float Width, float Height,
		const FString& Label, int32 ButtonIndex, bool bSelected);
	void DrawSlider(float X, float Y, float Width, float Height,
		const FString& Label, float Value, float Min, float Max);
	void DrawToggle(float X, float Y, const FString& Label, bool bEnabled);
	void DrawTextInput(float X, float Y, float Width, const FString& Label,
		const FString& CurrentText, bool bActive);
	void DrawTabBar(float X, float Y, float Width, const TArray<FString>& TabLabels,
		int32 ActiveTab);

	// Input
	void HandleMenuInput();
	void NavigateUp();
	void NavigateDown();
	void NavigateLeft();
	void NavigateRight();
	void ConfirmSelection();
	void GoBack();

	// Actions des boutons
	void OnNewExpedition();
	void OnContinueGame();
	void OnHostGame();
	void OnJoinGame();
	void OnOpenSettings();
	void OnQuitGame();
	void OnLaunchMission();
	void OnConnectToHost();

	// Settings appliquees
	void ApplyVideoSettings();
	void ApplyAudioSettings();
	void LoadCurrentSettings();

private:
	UPROPERTY()
	EMenuScreen CurrentScreen = EMenuScreen::Title;

	UPROPERTY()
	ESettingsTab CurrentSettingsTab = ESettingsTab::Video;

	// Etat de navigation
	int32 SelectedButtonIndex = 0;
	int32 MaxButtonCount = 0;
	float InputCooldown = 0.0f;
	bool bWaitingForAnyKey = true;

	// Animation
	float TitlePulseTimer = 0.0f;
	float ScanlineOffset = 0.0f;
	float CRTFlickerTimer = 0.0f;
	float TransitionAlpha = 0.0f;
	EMenuScreen TransitionTarget = EMenuScreen::Title;
	bool bTransitioning = false;

	// Settings temporaires (avant application)
	int32 PendingResolutionIndex = 0;
	bool bPendingFullscreen = true;
	int32 PendingQualityLevel = 2; // 0=Low, 1=Mid, 2=High, 3=Epic, 4=Cinematic
	bool bPendingVSync = true;
	float PendingFOV = 90.0f;
	float PendingMasterVolume = 1.0f;
	float PendingSFXVolume = 1.0f;
	float PendingMusicVolume = 0.8f;
	float PendingVoiceVolume = 1.0f;
	float PendingMouseSensitivity = 1.0f;
	bool bPendingInvertY = false;

	// Join lobby
	FString JoinIPAddress = TEXT("127.0.0.1");
	int32 JoinIPCursorPos = 0;

	// Host lobby
	int32 HostSelectedBiomeIndex = 0;
	int32 HostSelectedScaleIndex = 2;
	int32 HostMaxPlayers = 4;

	// Couleurs CRT
	static const FLinearColor CRT_Amber;
	static const FLinearColor CRT_AmberDim;
	static const FLinearColor CRT_AmberBright;
	static const FLinearColor CRT_Green;
	static const FLinearColor CRT_Red;
	static const FLinearColor CRT_Background;
	static const FLinearColor CRT_Scanline;
};
