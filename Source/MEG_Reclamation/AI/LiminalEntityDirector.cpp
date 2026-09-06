#include "AI/LiminalEntityDirector.h"

#include "AI/LiminalEntity.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "Player/ScavengerCharacter.h"

ULiminalEntityDirector::ULiminalEntityDirector()
{
}

void ULiminalEntityDirector::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	CurrentPhase = EDirectorPhase::BuildUp;
	PhaseTimer = 0.0f;
	GlobalIntensityMultiplier = BaseIntensityMultiplier;
	PeakCount = 0;
	DeathCount = 0;

	UE_LOG(LogTemp, Log, TEXT("EntityDirector: Initialized — starting in BuildUp phase."));
}

void ULiminalEntityDirector::Deinitialize()
{
	Super::Deinitialize();
}

bool ULiminalEntityDirector::ShouldCreateSubsystem(UObject* Outer) const
{
	// N'existe que sur le serveur (ou standalone)
	UWorld* World = Cast<UWorld>(Outer);
	return World && (World->GetNetMode() != NM_Client);
}

void ULiminalEntityDirector::UpdateDirector(float DeltaSeconds)
{
	PhaseTimer += DeltaSeconds;
	StressMetrics.TimeSinceLastPeak += DeltaSeconds;

	// Mise a jour des metriques a intervalle fixe pour eviter les calculs inutiles
	TimeSinceLastUpdate += DeltaSeconds;
	if (TimeSinceLastUpdate >= MetricsUpdateInterval)
	{
		TimeSinceLastUpdate = 0.0f;
		GatherStressMetrics();
	}

	EvaluatePhaseTransition(DeltaSeconds);
}

void ULiminalEntityDirector::GatherStressMetrics()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	float TotalHealth = 0.0f;
	float TotalSanity = 0.0f;
	int32 AliveCount = 0;
	int32 DownedCount = 0;

	for (TActorIterator<AScavengerCharacter> It(World); It; ++It)
	{
		AScavengerCharacter* Scav = *It;
		if (!Scav || Scav->IsDead())
		{
			continue;
		}

		if (Scav->IsDowned())
		{
			DownedCount++;
		}
		else
		{
			AliveCount++;
		}

		TotalHealth += Scav->GetHealthPercent();
		TotalSanity += Scav->GetSanityPercent();
	}

	const int32 TotalPlayers = AliveCount + DownedCount;
	StressMetrics.AlivePlayerCount = AliveCount;
	StressMetrics.DownedPlayerCount = DownedCount;

	if (TotalPlayers > 0)
	{
		StressMetrics.AverageHealthPercent = TotalHealth / static_cast<float>(TotalPlayers);
		StressMetrics.AverageSanityPercent = TotalSanity / static_cast<float>(TotalPlayers);
	}
	else
	{
		StressMetrics.AverageHealthPercent = 0.0f;
		StressMetrics.AverageSanityPercent = 0.0f;
	}

	// Compter les entites en chasse active
	int32 ChasingCount = 0;
	for (TActorIterator<ALiminalEntity> It(World); It; ++It)
	{
		ALiminalEntity* Entity = *It;
		if (Entity && !Entity->IsStunned() && !Entity->IsCalmed())
		{
			ChasingCount++;
		}
	}
	StressMetrics.ActiveChasingEntities = ChasingCount;

	// Calcul du stress composite
	// Facteurs : basse sante, basse sanite, entites en chasse, morts recentes
	const float HealthStress = 1.0f - StressMetrics.AverageHealthPercent;
	const float SanityStress = 1.0f - StressMetrics.AverageSanityPercent;
	const float ChaseStress = FMath::Clamp(static_cast<float>(ChasingCount) / 3.0f, 0.0f, 1.0f);
	const float DownedStress = FMath::Clamp(static_cast<float>(DownedCount) / FMath::Max(1, TotalPlayers), 0.0f, 1.0f);
	const float DeathStress = FMath::Clamp(static_cast<float>(DeathCount) * 0.25f, 0.0f, 1.0f);

	StressMetrics.CompositeStress = FMath::Clamp(
		HealthStress * 0.2f +
		SanityStress * 0.25f +
		ChaseStress * 0.25f +
		DownedStress * 0.15f +
		DeathStress * 0.15f,
		0.0f, 1.0f);

	// Le multiplicateur d'intensite augmente avec le temps de mission et le nombre de Peaks
	GlobalIntensityMultiplier = BaseIntensityMultiplier + (static_cast<float>(PeakCount) * 0.15f);
	GlobalIntensityMultiplier = FMath::Clamp(GlobalIntensityMultiplier, 0.5f, 3.0f);
}

void ULiminalEntityDirector::EvaluatePhaseTransition(float DeltaSeconds)
{
	switch (CurrentPhase)
	{
	case EDirectorPhase::BuildUp:
	{
		// Condition de sortie : temps minimum ecoule ET stress suffisamment bas
		// OU temps maximum atteint (force la montee de tension)
		const bool bTimedOut = PhaseTimer >= MaxBuildUpDuration;
		const bool bReadyToEscalate = PhaseTimer >= MinBuildUpDuration && StressMetrics.CompositeStress < BoredomThreshold;

		if (bTimedOut || bReadyToEscalate)
		{
			ForcePhase(EDirectorPhase::Sustain);
		}
		break;
	}

	case EDirectorPhase::Sustain:
	{
		// La tension monte pendant SustainDuration, puis Peak
		if (PhaseTimer >= SustainDuration)
		{
			// Ne pas declencher de Peak si l'escouade est deja en terreur
			if (StressMetrics.CompositeStress >= StressCapForPeak)
			{
				// Les joueurs sont deja terrifies — passer directement en Respite
				ForcePhase(EDirectorPhase::Respite);
			}
			else
			{
				ForcePhase(EDirectorPhase::Peak);
			}
		}
		break;
	}

	case EDirectorPhase::Peak:
	{
		if (PhaseTimer >= MinPeakDuration)
		{
			// Si le stress est tres eleve, abreger le peak
			if (StressMetrics.CompositeStress >= StressCapForPeak || PhaseTimer >= MinPeakDuration * 2.0f)
			{
				ForcePhase(EDirectorPhase::Respite);
			}
		}
		break;
	}

	case EDirectorPhase::Respite:
	{
		if (PhaseTimer >= RespiteDuration)
		{
			ForcePhase(EDirectorPhase::BuildUp);
		}
		break;
	}
	}
}

void ULiminalEntityDirector::ForcePhase(EDirectorPhase NewPhase)
{
	if (CurrentPhase == NewPhase)
	{
		return;
	}

	const EDirectorPhase OldPhase = CurrentPhase;
	CurrentPhase = NewPhase;
	PhaseTimer = 0.0f;

	if (NewPhase == EDirectorPhase::Peak)
	{
		PeakCount++;
		StressMetrics.TimeSinceLastPeak = 0.0f;
		SelectAndTriggerEvent();
	}

	OnPhaseChanged.Broadcast(NewPhase);

	UE_LOG(LogTemp, Log, TEXT("EntityDirector: Phase %d -> %d (Peak #%d, Intensity x%.2f, Stress %.2f)"),
		static_cast<int32>(OldPhase), static_cast<int32>(NewPhase),
		PeakCount, GlobalIntensityMultiplier, StressMetrics.CompositeStress);
}

void ULiminalEntityDirector::SelectAndTriggerEvent()
{
	// Selection d'evenement basee sur le contexte et la progression
	TArray<FName> CandidateEvents;

	// Blackout toujours disponible — evenement de base
	CandidateEvents.Add(FName("Blackout"));

	// Reality Shift apres le 2e peak
	if (PeakCount >= 2)
	{
		CandidateEvents.Add(FName("RealityShift"));
	}

	// Infestation apres le 3e peak ou si le stress est tres bas
	if (PeakCount >= 3 || StressMetrics.CompositeStress < 0.3f)
	{
		CandidateEvents.Add(FName("Infestation"));
	}

	// Effondrement en fin de mission (peak 4+)
	if (PeakCount >= 4)
	{
		CandidateEvents.Add(FName("Collapse"));
	}

	// Alarme toujours possible
	CandidateEvents.Add(FName("Alarm"));

	// Selection aleatoire ponderee
	if (CandidateEvents.Num() > 0)
	{
		const int32 Index = FMath::RandRange(0, CandidateEvents.Num() - 1);
		const FName SelectedEvent = CandidateEvents[Index];

		OnEventTriggered.Broadcast(SelectedEvent);

		UE_LOG(LogTemp, Log, TEXT("EntityDirector: Triggered event '%s' (from %d candidates)"),
			*SelectedEvent.ToString(), CandidateEvents.Num());
	}
}

void ULiminalEntityDirector::NotifyEventCompleted(FName EventName)
{
	UE_LOG(LogTemp, Log, TEXT("EntityDirector: Event '%s' completed."), *EventName.ToString());
}

void ULiminalEntityDirector::NotifyPlayerDied()
{
	DeathCount++;

	UE_LOG(LogTemp, Log, TEXT("EntityDirector: Player died (total deaths: %d). Adjusting pressure."), DeathCount);

	// Apres une mort, forcer une courte Respite pour ne pas faire ragequit
	if (CurrentPhase == EDirectorPhase::Peak || CurrentPhase == EDirectorPhase::Sustain)
	{
		ForcePhase(EDirectorPhase::Respite);
	}
}

void ULiminalEntityDirector::NotifyPlayerRevived()
{
	UE_LOG(LogTemp, Log, TEXT("EntityDirector: Player revived."));
}
