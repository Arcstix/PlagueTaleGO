#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "AWormPawn.generated.h"

/**
 * Le ver : un ennemi qui avance d'une case vers le joueur à chaque tour ennemi (prototype).
 *
 * Toute sa logique de tour est dans UEnemyTurnState (trouver le ver, calculer le chemin, le déplacer) :
 * le pion lui-même ne fait que demander ses déplacements au Board (RequestMove, hérité de ABoardPawn).
 *
 * Placement : comme tout pion, il est placé avec le Level Editor via un Data Asset BoardEntityType
 * (DA_Entity_Worm, PawnClass = BP_WormPawn), puis créé par ABoardGameMode au lancement de la partie.
 * Aucun réglage par placement pour l'instant (pas de SettingsClass) : ApplyPlacementSettings n'est pas surchargé.
 */
UCLASS()
class PLAGUETALEGO_API AWormPawn : public ABoardPawn
{
	GENERATED_BODY()
};