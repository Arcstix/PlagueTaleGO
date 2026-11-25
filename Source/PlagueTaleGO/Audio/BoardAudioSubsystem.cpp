#include "BoardAudioSubsystem.h"
#include "BoardAudioLibrary.h"
#include "BoardAudioSettings.h"
#include "BoardSoundEntry.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Level/Definitions/BoardDefinition.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundAttenuation.h"
#include "Sound/SoundBase.h"
#include "Sound/SoundConcurrency.h"

// ---------------------------------------------------------------------------
// Cycle de vie
// ---------------------------------------------------------------------------
UBoardAudioSubsystem* UBoardAudioSubsystem::Get(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	return World ? World->GetSubsystem<UBoardAudioSubsystem>() : nullptr;
}

bool UBoardAudioSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UBoardAudioSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// GetDefault : l'objet « par défaut » de la classe, qui porte les valeurs lues dans DefaultGame.ini
	const UBoardAudioSettings* Settings = GetDefault<UBoardAudioSettings>();
	Library = Settings ? Settings->Library.LoadSynchronous() : nullptr;

	if (!Library)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] Aucune bibliothèque de sons : la choisir dans Project Settings > Game > Board Audio > Library. Aucun son ne sera joué."));
	}
}

// ---------------------------------------------------------------------------
// Sons courts
// ---------------------------------------------------------------------------
void UBoardAudioSubsystem::PlaySound2D(FGameplayTag Event, const UBoardDefinition* Context)
{
	const FBoardSoundEntry* Entry = FindEntry(Event, Context);
	if (!Entry)
	{
		return;
	}
	USoundBase* Sound = PickSound(*Entry, Event, false);
	if (!Sound)
	{
		return;
	}
	UGameplayStatics::PlaySound2D(this, Sound, Entry->VolumeMultiplier, Entry->PickPitch(), 0.f, Entry->Concurrency);
}

void UBoardAudioSubsystem::PlaySoundAtLocation(FGameplayTag Event, FVector Location, const UBoardDefinition* Context)
{
	const FBoardSoundEntry* Entry = FindEntry(Event, Context);
	if (!Entry)
	{
		return;
	}
	USoundBase* Sound = PickSound(*Entry, Event, false);
	if (!Sound)
	{
		return;
	}
	UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, FRotator::ZeroRotator, Entry->VolumeMultiplier, Entry->PickPitch(),
		0.f, ResolveAttenuation(*Entry, Sound), Entry->Concurrency);
}

// ---------------------------------------------------------------------------
// Boucles
// ---------------------------------------------------------------------------
UAudioComponent* UBoardAudioSubsystem::StartLoop2D(FGameplayTag Event, const UBoardDefinition* Context)
{
	const FBoardSoundEntry* Entry = FindEntry(Event, Context);
	if (!Entry)
	{
		return nullptr;
	}
	USoundBase* Sound = PickSound(*Entry, Event, true);
	if (!Sound)
	{
		return nullptr;
	}
	// Spawn* (contrairement à Play*) renvoie le composant audio : c'est lui qui permet d'arrêter la boucle.
	// bAutoDestroy = true : le composant se détruit seul une fois le son arrêté.
	return RegisterLoop(UGameplayStatics::SpawnSound2D(this, Sound, Entry->VolumeMultiplier, Entry->PickPitch(), 0.f,
		Entry->Concurrency, /*bPersistAcrossLevelTransition*/ false, /*bAutoDestroy*/ true));
}

UAudioComponent* UBoardAudioSubsystem::StartLoopAtLocation(FGameplayTag Event, FVector Location, const UBoardDefinition* Context)
{
	const FBoardSoundEntry* Entry = FindEntry(Event, Context);
	if (!Entry)
	{
		return nullptr;
	}
	USoundBase* Sound = PickSound(*Entry, Event, true);
	if (!Sound)
	{
		return nullptr;
	}
	return RegisterLoop(UGameplayStatics::SpawnSoundAtLocation(this, Sound, Location, FRotator::ZeroRotator, Entry->VolumeMultiplier,
		Entry->PickPitch(), 0.f, ResolveAttenuation(*Entry, Sound), Entry->Concurrency, /*bAutoDestroy*/ true));
}

UAudioComponent* UBoardAudioSubsystem::StartLoopAttached(FGameplayTag Event, USceneComponent* AttachTo, const UBoardDefinition* Context)
{
	if (!AttachTo)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] StartLoopAttached(%s) sans composant auquel s'attacher : ignoré."), *Event.ToString());
		return nullptr;
	}
	const FBoardSoundEntry* Entry = FindEntry(Event, Context);
	if (!Entry)
	{
		return nullptr;
	}
	USoundBase* Sound = PickSound(*Entry, Event, true);
	if (!Sound)
	{
		return nullptr;
	}
	// bStopWhenAttachedToDestroyed = true : la boucle s'arrête d'elle-même quand le pion disparaît
	return RegisterLoop(UGameplayStatics::SpawnSoundAttached(Sound, AttachTo, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset, /*bStopWhenAttachedToDestroyed*/ true, Entry->VolumeMultiplier, Entry->PickPitch(), 0.f,
		ResolveAttenuation(*Entry, Sound), Entry->Concurrency, /*bAutoDestroy*/ true));
}

void UBoardAudioSubsystem::StopLoop(UAudioComponent* Loop, float FadeOutDuration)
{
	if (!IsValid(Loop))
	{
		return;
	}
	ActiveLoops.Remove(Loop);

	if (FadeOutDuration > 0.f)
	{
		// FadeOut baisse le volume jusqu'à FadeVolumeLevel (0) puis arrête le son
		Loop->FadeOut(FadeOutDuration, 0.f);
	}
	else
	{
		Loop->Stop();
	}
}

void UBoardAudioSubsystem::StopAllLoops(float FadeOutDuration)
{
	// Copie : StopLoop retire chaque boucle de ActiveLoops pendant le parcours
	const TArray<TWeakObjectPtr<UAudioComponent>> Loops = ActiveLoops;
	for (const TWeakObjectPtr<UAudioComponent>& Loop : Loops)
	{
		StopLoop(Loop.Get(), FadeOutDuration);
	}
	ActiveLoops.Reset();
}

// ---------------------------------------------------------------------------
// Outils internes
// ---------------------------------------------------------------------------
const FBoardSoundEntry* UBoardAudioSubsystem::FindEntry(FGameplayTag Event, const UBoardDefinition* Context)
{
	if (!Event.IsValid())
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] Son demandé sans tag d'événement : ignoré."));
		return nullptr;
	}

	// 1. Variante propre au contenu (ex. pas sur une case en bois)
	if (Context)
	{
		const FBoardSoundEntry* Override = Context->SoundOverrides.Find(Event);
		if (Override && Override->HasSound())
		{
			return Override;
		}
	}

	// 2. Son par défaut de la bibliothèque
	if (const FBoardSoundEntry* Entry = Library ? Library->FindEntry(Event) : nullptr)
	{
		return Entry;
	}

	// 3. Aucun son : normal tant que le son n'a pas été intégré. Le jeu continue en silence.
	WarnOnce(Event.ToString(), FString::Printf(TEXT("[Audio] Aucun son pour %s : l'ajouter dans DA_AudioLibrary."), *Event.ToString()));
	return nullptr;
}

USoundBase* UBoardAudioSubsystem::PickSound(const FBoardSoundEntry& Entry, FGameplayTag Event, bool bExpectLoop)
{
	USoundBase* Sound = Entry.PickRandomSound();
	if (!Sound)
	{
		return nullptr;
	}

	// Un son en boucle lancé avec Play* ne pourrait jamais être arrêté (aucune référence) : on refuse.
	if (!bExpectLoop && Sound->IsLooping())
	{
		WarnOnce(Event.ToString() + TEXT("|Loop"), FString::Printf(
			TEXT("[Audio] %s : « %s » est un son en boucle, il faut le lancer avec StartLoop*. Lecture refusée."),
			*Event.ToString(), *Sound->GetName()));
		return nullptr;
	}
	// L'inverse est sans danger : le son sera joué une seule fois
	if (bExpectLoop && !Sound->IsLooping())
	{
		WarnOnce(Event.ToString() + TEXT("|NotLoop"), FString::Printf(
			TEXT("[Audio] %s : « %s » n'est pas en boucle (case Looping de l'asset), il ne sera joué qu'une fois."),
			*Event.ToString(), *Sound->GetName()));
	}
	return Sound;
}

USoundAttenuation* UBoardAudioSubsystem::ResolveAttenuation(const FBoardSoundEntry& Entry, const USoundBase* Sound) const
{
	if (Entry.Attenuation)
	{
		return Entry.Attenuation;
	}
	if (Sound && Sound->AttenuationSettings)
	{
		return nullptr;   // nullptr = le moteur utilise l'atténuation de l'asset son
	}
	return Library ? Library->DefaultAttenuation.Get() : nullptr;
}

UAudioComponent* UBoardAudioSubsystem::RegisterLoop(UAudioComponent* Loop)
{
	// Loop peut être nul : son refusé par sa Concurrency, ou audio désactivé (-nosound)
	if (Loop)
	{
		// Ménage des boucles déjà détruites, puis ajout
		ActiveLoops.RemoveAll([](const TWeakObjectPtr<UAudioComponent>& Item) { return !Item.IsValid(); });
		ActiveLoops.Add(Loop);
	}
	return Loop;
}

void UBoardAudioSubsystem::WarnOnce(const FString& Key, const FString& Message)
{
	bool bAlreadyReported = false;
	ReportedWarnings.Add(Key, &bAlreadyReported);
	if (!bAlreadyReported)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("%s"), *Message);
	}
}
