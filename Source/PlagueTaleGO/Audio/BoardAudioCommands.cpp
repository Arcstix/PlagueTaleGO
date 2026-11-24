#include "BoardAudioSubsystem.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"     // FAutoConsoleCommandWithWorldAndArgs

/**
 * Audio test console commands: listen to a library sound without wiring any gameplay.
 * In PIE, open the game console (² key on AZERTY, ` on QWERTY); Tab completes the name.
 *
 * FAutoConsoleCommandWithWorldAndArgs: global object that registers the command when the module loads;
 * "WithWorld": the engine passes the world where the command is typed (the game world in PIE).
 * Stripped from the final game (UE_BUILD_SHIPPING).
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

		// false: no blocking error if the tag does not exist, an invalid tag is returned instead
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

		// 3D sound: played on the player pawn, to hear the spatialization while moving the camera
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
		TEXT("Plays a sound event from the library. Usage: BoardAudio.Play <Tag> [3D] [Loop]. 3D: on the player pawn. Loop: looping sound."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&PlayCommand));

	FAutoConsoleCommandWithWorld GStopLoopsCommand(
		TEXT("BoardAudio.StopLoops"),
		TEXT("Stops every running loop (0.5 s fade out)."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&StopLoopsCommand));
}
#endif
