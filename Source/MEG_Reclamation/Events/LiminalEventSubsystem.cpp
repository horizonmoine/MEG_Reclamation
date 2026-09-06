#include "Events/LiminalEventSubsystem.h"

#include "AI/LiminalEntity.h"
#include "AI/LiminalEntityDirector.h"
#include "EngineUtils.h"
#include "GameModes/LiminalGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Perception/AISense_Hearing.h"

ULiminalEventSubsystem::ULiminalEventSubsystem()
{
}

void ULiminalEventSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RegisterDefaultEvents();

	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Initialized with %d event types."), EventRegistry.Num());
}

void ULiminalEventSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

bool ULiminalEventSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	UWorld* World = Cast<UWorld>(Outer);
	return World && (World->GetNetMode() != NM_Client);
}

void ULiminalEventSubsystem::RegisterDefaultEvents()
{
	// Blackout — coupure de courant
	{
		FLiminalEventDescriptor Desc;
		Desc.EventId = FName("Blackout");
		Desc.DisplayName = FText::FromString(TEXT("Blackout"));
		Desc.DurationSeconds = 45.0f;
		Desc.CooldownSeconds = 90.0f;
		Desc.EntityAggressionMultiplier = 1.8f;
		Desc.MinPeakCountRequired = 0;
		EventRegistry.Add(Desc.EventId, Desc);
	}

	// Reality Shift — reconfiguration silencieuse de la geometrie
	{
		FLiminalEventDescriptor Desc;
		Desc.EventId = FName("RealityShift");
		Desc.DisplayName = FText::FromString(TEXT("Reality Shift"));
		Desc.DurationSeconds = 0.0f; // Instantane
		Desc.CooldownSeconds = 180.0f;
		Desc.EntityAggressionMultiplier = 1.0f;
		Desc.MinPeakCountRequired = 2;
		EventRegistry.Add(Desc.EventId, Desc);
	}

	// Infestation — rush massif d'une entite
	{
		FLiminalEventDescriptor Desc;
		Desc.EventId = FName("Infestation");
		Desc.DisplayName = FText::FromString(TEXT("Infestation"));
		Desc.DurationSeconds = 30.0f;
		Desc.CooldownSeconds = 120.0f;
		Desc.EntityAggressionMultiplier = 2.0f;
		Desc.MinPeakCountRequired = 3;
		EventRegistry.Add(Desc.EventId, Desc);
	}

	// Alarme — sirene attirant toutes les entites
	{
		FLiminalEventDescriptor Desc;
		Desc.EventId = FName("Alarm");
		Desc.DisplayName = FText::FromString(TEXT("Alarm"));
		Desc.DurationSeconds = 20.0f;
		Desc.CooldownSeconds = 60.0f;
		Desc.EntityAggressionMultiplier = 1.5f;
		Desc.MinPeakCountRequired = 0;
		EventRegistry.Add(Desc.EventId, Desc);
	}

	// Collapse — effondrement de sections
	{
		FLiminalEventDescriptor Desc;
		Desc.EventId = FName("Collapse");
		Desc.DisplayName = FText::FromString(TEXT("Collapse"));
		Desc.DurationSeconds = 15.0f;
		Desc.CooldownSeconds = 240.0f;
		Desc.EntityAggressionMultiplier = 1.2f;
		Desc.MinPeakCountRequired = 4;
		EventRegistry.Add(Desc.EventId, Desc);
	}
}

void ULiminalEventSubsystem::UpdateEvents(float DeltaSeconds)
{
	if (!bEventActive)
	{
		return;
	}

	if (ActiveEventTimer > 0.0f)
	{
		ActiveEventTimer -= DeltaSeconds;

		if (ActiveEventTimer <= 0.0f)
		{
			EndCurrentEvent();
		}
	}
}

void ULiminalEventSubsystem::TriggerEvent(FName EventId)
{
	if (bEventActive)
	{
		UE_LOG(LogTemp, Warning, TEXT("EventSubsystem: Cannot trigger '%s' — event '%s' already active."),
			*EventId.ToString(), *ActiveEventId.ToString());
		return;
	}

	const FLiminalEventDescriptor* Desc = EventRegistry.Find(EventId);
	if (!Desc)
	{
		UE_LOG(LogTemp, Warning, TEXT("EventSubsystem: Unknown event '%s'."), *EventId.ToString());
		return;
	}

	// Verifier le cooldown
	if (const double* CooldownEnd = EventCooldowns.Find(EventId))
	{
		const double Now = GetWorld()->GetTimeSeconds();
		if (Now < *CooldownEnd)
		{
			UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Event '%s' on cooldown (%.1f sec remaining)."),
				*EventId.ToString(), *CooldownEnd - Now);
			return;
		}
	}

	bEventActive = true;
	ActiveEventId = EventId;
	ActiveEventTimer = Desc->DurationSeconds;

	// Mettre en cooldown
	EventCooldowns.Add(EventId, GetWorld()->GetTimeSeconds() + Desc->CooldownSeconds);

	// Executer la logique specifique de l'evenement
	if (EventId == FName("Blackout"))
	{
		ExecuteBlackout();
	}
	else if (EventId == FName("RealityShift"))
	{
		ExecuteRealityShift();
	}
	else if (EventId == FName("Infestation"))
	{
		ExecuteInfestation();
	}
	else if (EventId == FName("Alarm"))
	{
		ExecuteAlarm();
	}
	else if (EventId == FName("Collapse"))
	{
		ExecuteCollapse();
	}

	OnEventStarted.Broadcast(EventId, Desc->DurationSeconds);

	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Event '%s' started (duration: %.1f sec, aggression: x%.2f)."),
		*EventId.ToString(), Desc->DurationSeconds, Desc->EntityAggressionMultiplier);

	// Si l'evenement est instantane, le terminer immediatement
	if (Desc->DurationSeconds <= 0.0f)
	{
		EndCurrentEvent();
	}
}

void ULiminalEventSubsystem::EndCurrentEvent()
{
	if (!bEventActive)
	{
		return;
	}

	// Executer la logique de fin specifique
	if (ActiveEventId == FName("Blackout"))
	{
		EndBlackout();
	}
	else if (ActiveEventId == FName("RealityShift"))
	{
		EndRealityShift();
	}
	else if (ActiveEventId == FName("Infestation"))
	{
		EndInfestation();
	}
	else if (ActiveEventId == FName("Alarm"))
	{
		EndAlarm();
	}
	else if (ActiveEventId == FName("Collapse"))
	{
		EndCollapse();
	}

	OnEventEnded.Broadcast(ActiveEventId);

	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Event '%s' ended."), *ActiveEventId.ToString());

	// Notifier le Director
	if (ULiminalEntityDirector* Director = GetWorld()->GetSubsystem<ULiminalEntityDirector>())
	{
		Director->NotifyEventCompleted(ActiveEventId);
	}

	bEventActive = false;
	ActiveEventId = NAME_None;
	ActiveEventTimer = 0.0f;
}

// --- Implementations specifiques des evenements ---

void ULiminalEventSubsystem::ExecuteBlackout()
{
	// Utiliser le systeme de blackout existant du GameMode
	if (ALiminalGameMode* GM = Cast<ALiminalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		const FLiminalEventDescriptor* Desc = EventRegistry.Find(FName("Blackout"));
		GM->TriggerBlackout(Desc ? Desc->DurationSeconds : 45.0f);
	}
}

void ULiminalEventSubsystem::ExecuteRealityShift()
{
	// La geometrie se reconfigure silencieusement hors du champ de vision du joueur
	// TODO: Integrer avec le LiminalLevelGenerator pour modifier des sections du layout
	// Pour l'instant, bloquer certains passages et en ouvrir d'autres
	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Reality Shift — reconfiguring layout sections..."));

	// Effet de distortion visuelle sur tous les joueurs (via SanityPostProcess)
	// Ce sera connecte au LiminalSanityPostProcessComponent une fois le systeme de portails non-euclidiens en place
}

void ULiminalEventSubsystem::ExecuteInfestation()
{
	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Infestation — spawning entity rush..."));

	// Spawn un rush massif d'une seule entite (Wretches ou Clumps)
	// Les entites spawned seront despawned a la fin de l'evenement
	// TODO: Integrer avec le spawn system du LiminalLevelGenerator
}

void ULiminalEventSubsystem::ExecuteAlarm()
{
	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Alarm — attracting all entities to alarm location..."));

	// Emettre un son massif au centre du niveau qui attire toutes les entites
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector AlarmLocation = FVector::ZeroVector; // TODO: choisir une position pertinente

	// Emettre un bruit attracteur tres puissant pour l'IA
	UAISense_Hearing::ReportNoiseEvent(World, AlarmLocation, 5.0f, nullptr, 0.0f, FName("Alarm"));
}

void ULiminalEventSubsystem::ExecuteCollapse()
{
	UE_LOG(LogTemp, Log, TEXT("EventSubsystem: Collapse — destroying sections..."));

	// Sections du niveau qui s'effondrent physiquement
	// TODO: Integrer avec le LiminalLevelGenerator pour desactiver des chunks ISM
	// et spawner des debris/particles a leur place
}

// --- Fin d'evenement ---

void ULiminalEventSubsystem::EndBlackout()
{
	if (ALiminalGameMode* GM = Cast<ALiminalGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GM->RestorePower();
	}
}

void ULiminalEventSubsystem::EndRealityShift()
{
	// Le shift est permanent — pas de retour a la normale
}

void ULiminalEventSubsystem::EndInfestation()
{
	// Despawner les entites rush supplementaires
	// Les entites de base de la map restent
}

void ULiminalEventSubsystem::EndAlarm()
{
	// La sirene s'arrete — les entites reprennent leur comportement normal
}

void ULiminalEventSubsystem::EndCollapse()
{
	// Les sections effondrees restent detruites — modification permanente
}
