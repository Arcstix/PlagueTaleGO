#pragma once

#include "NativeGameplayTags.h"

/**
 * Gameplay Tags « natifs » du plateau : ceux que le code C++ teste directement.
 *
 * Un Gameplay Tag est un nom hiérarchique (ex. Board.Node.Exit) déclaré une seule fois et
 * comparé très rapidement. On s'en sert pour décrire les COMPORTEMENTS d'un type de case
 * (UBoardNodeType::Behaviors) au lieu d'ajouter un booléen par comportement.
 * Équivalent Godot : les « groups » d'un nœud, mais hiérarchiques et vérifiés par le moteur.
 *
 * - Les tags déclarés ici existent dès le lancement de l'éditeur (pas besoin de .ini).
 * - Les GD peuvent créer d'autres tags dans Project Settings > GameplayTags, sous Board.Node.
 *   Un tag que le code ne teste pas n'a aucun effet en jeu : il faut ajouter le test en C++.
 * - Ajouter un tag natif : UE_DECLARE ici + UE_DEFINE dans BoardTags.cpp, puis le documenter
 *   dans Source/PlagueTaleGOEditor/CLAUDE.md (tableau des comportements).
 */
namespace BoardTags
{
	// Atteindre la case fait gagner la partie
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Node_Exit);

	// Case infranchissable
	PLAGUETALEGO_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Node_Blocked);
}
