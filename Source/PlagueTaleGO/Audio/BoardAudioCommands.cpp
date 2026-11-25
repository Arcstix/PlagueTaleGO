#include "BoardAudioSubsystem.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"     // FAutoConsoleCommandWithWorldAndArgs

/**
 * Commandes console de test de l'audio : écouter un son de la bibliothèque sans brancher de gameplay.
 * En PIE, ouvrir la console du jeu (touche ² en AZERTY, ` en QWERTY) ; Tab complète le nom.
 *
 * FAutoConsoleCommandWithWorldAndArgs : objet global qui enregistre la commande au chargement du module ;
 * « WithWorld » : le moteur passe le monde où la commande est tapée (celui de la partie en PIE).
 * Retirées du jeu final (UE_BUILD_SHIPPING).
 */
#if !UE_BUILD_SHIPPING
namespace
{
	UBoardAudioSubsystem* GetAudioSubsystem(UWorld* World)
	{
		UBoardAudioSubsystem* Audio = World ? World->GetSubsystem<UBoardAudioSubsystem>() : nullptr;
		if (!Audio)
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] Commande disponible en jeu seulement : lancer le PIE et la taper dans la console du jeu."));
		}
		return Audio;
	}

	// BoardAudio.Play <Tag> [3D] [Loop]
	void PlayCommand(const TArray<FString>& Args, UWorld* World)
	{
		UBoardAudioSubsystem* Audio = GetAudioSubsystem(World);
		if (!Audio)
		{
			return;
		}
		if (Args.IsEmpty())
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] Usage : BoardAudio.Play <Tag> [3D] [Loop]   (ex. BoardAudio.Play Audio.Player.Footstep 3D)"));
			return;
		}

		// false : pas d'erreur bloquante si le tag n'existe pas, on renvoie un tag invalide
		const FGameplayTag Event = FGameplayTag::RequestGameplayTag(FName(*Args[0]), false);
		if (!Event.IsValid())
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] Tag inconnu : %s"), *Args[0]);
			return;
		}

		bool b3D = false;
		bool bLoop = false;
		for (int32 Index = 1; Index < Args.Num(); ++Index)
		{
			b3D |= Args[Index].Equals(TEXT("3D"), ESearchCase::IgnoreCase);
			bLoop |= Args[Index].Equals(TEXT("Loop"), ESearchCase::IgnoreCase);
		}

		// Son 3D : joué sur le pion joueur, pour entendre la spatialisation en bougeant la caméra
		const UGameFlowSubsystem* GameFlow = World->GetSubsystem<UGameFlowSubsystem>();
		APlayerPawn* Player = GameFlow ? GameFlow->GetPlayerPawn() : nullptr;
		if (b3D && !Player)
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Audio] Pas de pion joueur : son 3D joué à l'origine du monde."));
		}
		const FVector Location = Player ? Player->GetActorLocation() : FVector::ZeroVector;

		if (bLoop)
		{
			if (!b3D)
			{
				Audio->StartLoop2D(Event);
			}
			else if (Player)
			{
				Audio->StartLoopAttached(Event, Player->GetRootComponent());
			}
			else
			{
				Audio->StartLoopAtLocation(Event, Location);
			}
		}
		else if (b3D)
		{
			Audio->PlaySoundAtLocation(Event, Location);
		}
		else
		{
			Audio->PlaySound2D(Event);
		}
	}

	void StopLoopsCommand(UWorld* World)
	{
		if (UBoardAudioSubsystem* Audio = GetAudioSubsystem(World))
		{
			Audio->StopAllLoops(0.5f);
		}
	}

	FAutoConsoleCommandWithWorldAndArgs GPlayCommand(
		TEXT("BoardAudio.Play"),
		TEXT("Joue un événement sonore de la bibliothèque. Usage : BoardAudio.Play <Tag> [3D] [Loop]. 3D : sur le pion joueur. Loop : son en boucle."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&PlayCommand));

	FAutoConsoleCommandWithWorld GStopLoopsCommand(
		TEXT("BoardAudio.StopLoops"),
		TEXT("Arrête toutes les boucles en cours (fondu de 0,5 s)."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&StopLoopsCommand));
}
#endif
