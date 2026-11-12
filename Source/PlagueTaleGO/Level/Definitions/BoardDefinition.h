#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
// Include complet : tester ou lire SettingsClass appelle UBoardPlacementSettings::StaticClass()
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "BoardDefinition.generated.h"

class UTexture2D;

/**
 * Base de toutes les « définitions » manipulées par le Level Editor :
 * types de cases (UBoardNodeType), d'entités (UBoardEntityType), d'objets (UBoardItemType).
 *
 * Une définition est un Data Asset créé dans le Content Browser
 * (clic droit > Miscellaneous > Data Asset, puis choisir la classe).
 * Le Level Editor trouve automatiquement tous les assets de ces classes :
 * créer, renommer ou supprimer un asset suffit, aucun code à modifier.
 * Équivalent Godot : une Resource personnalisée (.tres).
 *
 * Les champs « Level Editor » ne servent qu'à la présentation dans l'outil (nom, aide, couleur).
 */
UCLASS(Abstract, BlueprintType)
class PLAGUETALEGO_API UBoardDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// Nom à afficher : DisplayName s'il est renseigné, sinon le nom de l'asset
	UFUNCTION(BlueprintPure, Category = "Level Editor")
	FText GetDisplayText() const;

	// Crée un objet de réglages vierge pour un nouveau placement de ce type.
	// Renvoie nullptr si SettingsClass est vide ou abstraite. Outer = propriétaire de l'objet.
	UBoardPlacementSettings* CreateSettings(UObject* Outer) const;

	// --- Présentation dans le Level Editor ---

	// Nom affiché dans la palette et les messages. Vide = nom de l'asset.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (ToolTip = "Name shown in the Board mode and in messages. Empty = asset name."))
	FText DisplayName;

	// Texte d'aide affiché dans le Level Editor : à quoi sert ce type, quand l'utiliser.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (MultiLine = true, ToolTip = "Help text shown in the Board mode: what this type is for, when to use it."))
	FText Description;

	// Couleur utilisée par le Level Editor pour l'aperçu (cases fantômes, pastilles…)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (ToolTip = "Color used in the Board mode preview (center dot of cells, entity arrows...)."))
	FLinearColor EditorColor = FLinearColor::White;

	// Icône de la palette (optionnelle). Référence « soft » : la texture n'est pas chargée en jeu.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (ToolTip = "Optional icon. Not loaded in game."))
	TSoftObjectPtr<UTexture2D> Icon;

	// Ordre dans la palette : les petites valeurs d'abord, puis ordre alphabétique
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (ToolTip = "Order in lists: smaller values first, then alphabetical order."))
	int32 SortOrder = 0;

	// --- Réglages par placement ---

	// Réglages propres à chaque élément placé avec ce type (ex. patrouille d'un ennemi).
	// Vide = aucun réglage. Voir UBoardPlacementSettings.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Editor", meta = (ToolTip = "Settings specific to each element placed with this type (e.g. an enemy patrol). Empty = no settings."))
	TSubclassOf<UBoardPlacementSettings> SettingsClass;
};
