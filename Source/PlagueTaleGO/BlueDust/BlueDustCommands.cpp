#include "BlueDustTrailComponent.h"
#include "PlagueTaleGO/PlagueTaleGO.h"
#include "PlagueTaleGO/Entities/Player/APlayerPawn.h"
#include "PlagueTaleGO/Turns/GameFlowSubsystem.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "GameFramework/Actor.h"
#include "HAL/IConsoleManager.h"     // FAutoConsoleCommandWithWorldAndArgs

/**
 * Commandes console de test de la poussière bleue : manipuler la trace sans les features qui l'utiliseront (ver, statue).
 * En PIE, ouvrir la console du jeu (touche ² en AZERTY, ` en QWERTY) ; Tab complète le nom.
 * Même mécanisme que BoardAudioCommands.cpp. Retirées du jeu final (UE_BUILD_SHIPPING).
 */
#if !UE_BUILD_SHIPPING
namespace
{
	// Acteur vide qui joue le rôle d'une statue pour BlueDust.Suppress / BlueDust.Release.
	// Le détruire teste exactement le cas « statue détruite » (blocage levé par AActor::OnDestroyed).
	TWeakObjectPtr<AActor> GDebugBlocker;

	UBlueDustTrailComponent* GetPlayerTrail(UWorld* World)
	{
		const UGameFlowSubsystem* GameFlow = World ? World->GetSubsystem<UGameFlowSubsystem>() : nullptr;
		const APlayerPawn* Player = GameFlow ? GameFlow->GetPlayerPawn() : nullptr;
		UBlueDustTrailComponent* Trail = Player ? Player->GetBlueDustTrail() : nullptr;
		if (!Trail)
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[BlueDust] Commande disponible en jeu seulement : lancer le PIE et la taper dans la console du jeu."));
		}
		return Trail;
	}

	// BlueDust.Debug [0|1]
	void DebugCommand(const TArray<FString>& Args, UWorld* World)
	{
		if (UBlueDustTrailComponent* Trail = GetPlayerTrail(World))
		{
			// Sans argument : bascule
			const bool bEnable = Args.IsEmpty() ? !Trail->IsDrawingDebug() : Args[0].ToBool();
			Trail->SetDrawDebug(bEnable);
			UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] Dessin de debug %s"), bEnable ? TEXT("activé") : TEXT("coupé"));
		}
	}

	void DumpCommand(UWorld* World)
	{
		if (const UBlueDustTrailComponent* Trail = GetPlayerTrail(World))
		{
			Trail->DumpToLog();
		}
	}

	void ClearCommand(UWorld* World)
	{
		if (UBlueDustTrailComponent* Trail = GetPlayerTrail(World))
		{
			Trail->ClearTrail();
		}
	}

	// BlueDust.SetLength <N>
	void SetLengthCommand(const TArray<FString>& Args, UWorld* World)
	{
		UBlueDustTrailComponent* Trail = GetPlayerTrail(World);
		if (!Trail)
		{
			return;
		}
		if (Args.IsEmpty() || !Args[0].IsNumeric())
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[BlueDust] Usage : BlueDust.SetLength <N>   (ex. BlueDust.SetLength 3 ; 0 = aucune trace)"));
			return;
		}
		Trail->SetTrailLength(FCString::Atoi(*Args[0]));
		UE_LOG(LogPlagueTaleGO, Log, TEXT("[BlueDust] TrailLength = %d"), Trail->GetTrailLength());
	}

	void SuppressCommand(UWorld* World)
	{
		UBlueDustTrailComponent* Trail = GetPlayerTrail(World);
		if (!Trail)
		{
			return;
		}
		if (!GDebugBlocker.IsValid())
		{
			FActorSpawnParameters Params;
			// Les acteurs appartiennent au niveau : le nom doit être unique dans le niveau (sinon le spawn échoue)
			Params.Name = MakeUniqueObjectName(World->PersistentLevel, AActor::StaticClass(), TEXT("BlueDustDebugBlocker"));
			GDebugBlocker = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
		}
		Trail->SuppressTrail(GDebugBlocker.Get());
	}

	void ReleaseCommand(UWorld* World)
	{
		if (!GDebugBlocker.IsValid())
		{
			UE_LOG(LogPlagueTaleGO, Warning, TEXT("[BlueDust] Aucun blocage de debug en cours (BlueDust.Suppress d'abord)."));
			return;
		}
		// Destruction (et non RemoveEmissionBlocker) : même chemin qu'une statue détruite par le ver ou l'artefact
		GDebugBlocker->Destroy();
		GDebugBlocker.Reset();
	}

	FAutoConsoleCommandWithWorldAndArgs GDebugCommand(
		TEXT("BlueDust.Debug"),
		TEXT("Affiche / masque le dessin de debug de la trace du joueur. Usage : BlueDust.Debug [0|1] (sans argument : bascule)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&DebugCommand));

	FAutoConsoleCommandWithWorld GDumpCommand(
		TEXT("BlueDust.Dump"),
		TEXT("Écrit dans le log les traces du joueur (N-1, N-2…), leur durée restante et les bloqueurs."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&DumpCommand));

	FAutoConsoleCommandWithWorld GClearCommand(
		TEXT("BlueDust.Clear"),
		TEXT("Retire toutes les traces du joueur."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&ClearCommand));

	FAutoConsoleCommandWithWorldAndArgs GSetLengthCommand(
		TEXT("BlueDust.SetLength"),
		TEXT("Change la durée de vie des traces du joueur. Usage : BlueDust.SetLength <N> (0 = aucune trace)."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&SetLengthCommand));

	FAutoConsoleCommandWithWorld GSuppressCommand(
		TEXT("BlueDust.Suppress"),
		TEXT("Simule une statue qui poursuit le joueur : retire toutes les traces et bloque l'émission jusqu'à BlueDust.Release."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&SuppressCommand));

	FAutoConsoleCommandWithWorld GReleaseCommand(
		TEXT("BlueDust.Release"),
		TEXT("Simule la destruction de la statue : détruit le bloqueur de debug, la trace reprend au prochain déplacement."),
		FConsoleCommandWithWorldDelegate::CreateStatic(&ReleaseCommand));
}
#endif
