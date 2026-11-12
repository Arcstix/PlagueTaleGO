#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "BoardLayout.generated.h"

class UBoardNodeType;
class UBoardEntityType;
class UBoardItemType;
class UBoardPlacementSettings;

/**
 * Ce fichier décrit le « layout » d'un niveau : la description complète du plateau
 * (cases, liens, entités de départ, objets), écrite par le Level Editor et stockée dans ABoard.
 * C'est la SOURCE DE VÉRITÉ du niveau : les acteurs ABoardNode n'en sont qu'une représentation
 * générée dans la scène au moment du Submit.
 *
 * Tout est en données pures (aucun acteur, aucun subsystem) : le layout se lit et se valide
 * aussi bien dans l'éditeur qu'en jeu.
 */

// Une entité de départ posée sur une case
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardEntityPlacement
{
	GENERATED_BODY()

	// Type d'entité (Data Asset)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Type", ToolTip = "Entity type (Data Asset): player, enemy..."))
	TObjectPtr<UBoardEntityType> Type;

	// Direction dans laquelle le pion regarde au départ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Facing", ToolTip = "Direction the pawn faces at start. North = top of the top-down view."))
	EBoardDirection Facing = EBoardDirection::North;

	// Réglages propres à ce placement (classe choisie par Type->SettingsClass). Peut être nul.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Settings", ToolTip = "Settings specific to this placement. Created automatically from the type's Settings Class."))
	TObjectPtr<UBoardPlacementSettings> Settings;
};

// Un objet posé sur une case
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardItemPlacement
{
	GENERATED_BODY()

	// Type d'objet (Data Asset)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Type", ToolTip = "Item type (Data Asset)."))
	TObjectPtr<UBoardItemType> Type;

	// Réglages propres à ce placement (classe choisie par Type->SettingsClass). Peut être nul.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Settings", ToolTip = "Settings specific to this placement. Created automatically from the type's Settings Class."))
	TObjectPtr<UBoardPlacementSettings> Settings;
};

// Une case du layout
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardCell
{
	GENERATED_BODY()

	// Position sur la grille (unique dans le layout)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Coordinates", ToolTip = "Position on the grid (unique in the layout)."))
	FIntPoint Coord = FIntPoint::ZeroValue;

	// Type de la case : donne son visuel et ses comportements
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Cell Type", ToolTip = "Cell type: gives its visuals and its behaviors."))
	TObjectPtr<UBoardNodeType> NodeType;

	// Réglages propres à cette case (classe choisie par NodeType->SettingsClass). Peut être nul.
	UPROPERTY(EditAnywhere, Instanced, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Cell Settings", ToolTip = "Settings specific to this cell (only if its type declares a Settings Class)."))
	TObjectPtr<UBoardPlacementSettings> NodeSettings;

	// Entités créées sur cette case au lancement (liste : plusieurs possibles)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Entities", ToolTip = "Entities spawned on this cell when the game starts (player start, enemies...)."))
	TArray<FBoardEntityPlacement> Entities;

	// Objets posés sur cette case (liste : plusieurs possibles)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Items", ToolTip = "Items placed on this cell."))
	TArray<FBoardItemPlacement> Items;
};

/**
 * Le plateau complet d'un niveau, avec les requêtes de graphe dont ont besoin
 * le jeu (UBoardSubsystem), la validation et le Level Editor.
 *
 * Les niveaux font quelques dizaines de cases : les recherches sont linéaires,
 * c'est volontaire (code simple, coût négligeable).
 */
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardLayout
{
	GENERATED_BODY()

	// Les cases du plateau
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Cells", ToolTip = "Board cells. Edited only with the Board mode."))
	TArray<FBoardCell> Cells;

	// Liens explicites entre cases voisines. Pas de lien = pas de passage.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Level", meta = (DisplayName = "Links", ToolTip = "Explicit links between neighboring cells. No link = no passage."))
	TSet<FBoardLink> Links;

	// --- Cases ---

	const FBoardCell* FindCell(FIntPoint Coord) const;
	FBoardCell* FindCell(FIntPoint Coord);
	bool HasCell(FIntPoint Coord) const { return FindCell(Coord) != nullptr; }

	// Ajoute une case sans lien. Si la case existe déjà, la renvoie sans la modifier.
	FBoardCell& AddCell(FIntPoint Coord, UBoardNodeType* NodeType);

	// Supprime la case ET tous ses liens. Renvoie false si elle n'existait pas.
	bool RemoveCell(FIntPoint Coord);

	// --- Liens ---

	bool HasLink(FIntPoint A, FIntPoint B) const;

	// Vrai si un lien est autorisé : deux cases existantes, distinctes et voisines orthogonales
	bool CanLink(FIntPoint A, FIntPoint B) const;

	// Ajoute le lien s'il est autorisé. Renvoie false sinon (ou s'il existait déjà).
	bool AddLink(FIntPoint A, FIntPoint B);

	// Renvoie false si le lien n'existait pas
	bool RemoveLink(FIntPoint A, FIntPoint B);

	// Coordonnées des cases reliées à Coord
	TArray<FIntPoint> GetLinkedCoords(FIntPoint Coord) const;

	int32 CountLinks(FIntPoint Coord) const;

	// --- Graphe ---

	/**
	 * Cases atteignables depuis Start en suivant les liens (parcours en largeur), Start inclus.
	 * CanEnter (optionnel) : filtre les cases dans lesquelles on peut entrer (ex. exclure les cases bloquées).
	 * Renvoie un ensemble vide si Start n'est pas une case du layout.
	 */
	TSet<FIntPoint> GetReachableCoords(FIntPoint Start, TFunction<bool(const FBoardCell&)> CanEnter = nullptr) const;

	// --- Utilitaires ---

	bool IsEmpty() const { return Cells.Num() == 0; }

	/**
	 * Remplace chaque objet de réglages (sous-objet instancié) par une copie appartenant à NewOuter.
	 * À appeler après avoir copié un layout vers un nouveau propriétaire (brouillon du Level Editor,
	 * ABoard au Submit) : sinon les deux layouts partageraient les mêmes objets de réglages.
	 */
	void DeepCopySettings(UObject* NewOuter);
};
