#pragma once

#include "CoreMinimal.h"

class ABoard;
class UWorld;
struct FBoardLayout;

/**
 * Fait le lien, dans l'éditeur, entre le layout (données du ABoard) et la scène (acteurs ABoardNode).
 *
 * Règle : le layout est la source de vérité. Les acteurs ABoardNode sont une représentation
 * GÉNÉRÉE : on ne les modifie jamais à la main, on modifie le layout puis on synchronise.
 *
 * Toutes ces fonctions sont réservées à l'éditeur (le jeu lit le layout, il ne l'écrit jamais).
 */
struct FBoardLayoutSync
{
	// Le ABoard du monde. Logue une erreur s'il n'y en a aucun, un avertissement s'il y en a plusieurs.
	static ABoard* FindBoard(UWorld* World);

	/**
	 * Submit : écrit NewLayout dans le Board puis régénère les cases de la scène.
	 * Le tout forme UNE seule transaction : un Ctrl+Z annule le Submit complet.
	 * Ne valide PAS le layout : l'appelant décide (option « Ignorer les erreurs » du Level Editor).
	 */
	static void Submit(ABoard& Board, const FBoardLayout& NewLayout, const FText& TransactionName);

	/**
	 * Met les acteurs ABoardNode en accord avec le layout actuel du Board :
	 * crée les cases manquantes, met à jour le type des existantes, détruit celles en trop.
	 * À appeler dans une transaction (FScopedTransaction) pour que Ctrl+Z fonctionne.
	 */
	static void SyncNodeActors(ABoard& Board);

	/**
	 * Migration : construit un layout à partir de l'ancien format (avant le Level Editor) :
	 * cases ABoardNode placées à la main + leurs propriétés _DEPRECATED + liens bloqués du Board.
	 * Les types sont cherchés parmi les Data Assets existants. OutReport liste ce qui n'a pas pu être converti.
	 * TEMPORAIRE : à supprimer avec les propriétés _DEPRECATED quand tous les niveaux seront migrés.
	 */
	static FBoardLayout BuildLayoutFromLegacyScene(ABoard& Board, TArray<FString>& OutReport);
};
