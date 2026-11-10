#pragma once

#include "CoreMinimal.h"
#include "Delegates/IDelegateInstance.h"   // FDelegateHandle

class UObject;
struct FPropertyChangedEvent;

/**
 * Resynchronise automatiquement la scène quand une donnée du plateau est modifiée dans l'éditeur,
 * pour ne pas avoir à taper BoardEditor.Resync :
 * - le Layout d'un ABoard (ex. édition manuelle temporaire, § 8.9 du CLAUDE.md) ;
 * - le NodeClass d'un type de case (les acteurs de cette case doivent être recréés).
 * Le visuel d'un type de case (mesh, matériaux) est déjà mis à jour par UBoardNodeType lui-même.
 *
 * Mécanisme UE : FCoreUObjectDelegates::OnObjectPropertyChanged est diffusé par l'éditeur
 * après chaque modification d'une propriété (panneau Details), pour n'importe quel objet.
 * La modification est déjà dans une transaction : la synchronisation y est ajoutée,
 * donc un seul Ctrl+Z annule la modification ET la mise à jour de la scène.
 *
 * Coût : une synchronisation parcourt les cases du niveau (quelques dizaines), uniquement quand
 * une modification est validée (pas pendant le glissement d'un curseur).
 */
struct FBoardAutoSync
{
	static void Register();
	static void Unregister();

private:
	static void HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event);

	static FDelegateHandle Handle;
};
