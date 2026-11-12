// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "GameFramework/Actor.h"
#include "Board.generated.h"

class UInstancedStaticMeshComponent;

/**
 * Un seul par niveau. Définit la grille (origine = position de l'acteur),
 * porte le layout du niveau (cases, liens, entités, objets) et dessine les liens.
 *
 * Le layout n'est modifiable QUE par le Level Editor (module PlagueTaleGOEditor) :
 * il est en lecture seule dans le panneau Details. Voir Source/PlagueTaleGOEditor/CLAUDE.md.
 */
UCLASS()
class PLAGUETALEGO_API ABoard : public AActor
{
	GENERATED_BODY()
		
public:
	ABoard();
	
	virtual void Tick(float DeltaSeconds) override;
	// Autorise le Tick dans le viewport de l'éditeur (hors Play)
	virtual bool ShouldTickIfViewportsOnly() const override;
	
	// --- Grille ---
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Grid", meta = (ClampMin = "10"))
	float CellSize = 200.f;
	
    FIntPoint WorldToGrid(const FVector& Location) const;
    FVector   GridToWorld(FIntPoint Coord) const;

	// --- Layout du niveau ---
	const FBoardLayout& GetLayout() const { return Layout; }

	// Vrai si un lien a été tracé entre A et B
	bool HasLink(FIntPoint A, FIntPoint B) const { return Layout.HasLink(A, B); }

#if WITH_EDITOR
	/**
	 * Remplace le layout. Réservé au Level Editor (Submit) : ne pas appeler ailleurs.
	 * Appelle Modify() (Ctrl+Z) et duplique les réglages pour qu'ils appartiennent à ce Board.
	 * Ne met PAS à jour les acteurs ABoardNode : c'est le rôle de FBoardLayoutSync (module éditeur).
	 */
	void SetLayout(const FBoardLayout& NewLayout);
#endif

#if WITH_EDITORONLY_DATA
	// Ancien format, lu uniquement par la migration (commande BoardEditor.ImportLegacy)
	const TSet<FBoardLink>& GetLegacyBlockedLinks() const { return BlockedLinks_DEPRECATED; }
#endif

	// --- Visuels des liens ---
	// Bouton « Rebuild Link Visuals » dans le panneau Details
	UFUNCTION(CallInEditor, Category = "Board|Edition")
	void RebuildLinkVisuals();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

#if WITH_EDITOR
	// Après un Ctrl+Z, le layout a pu changer : on redessine les liens
	virtual void PostEditUndo() override;
#endif

	// --- Layout du niveau ---

	// Le plateau du niveau : cases, liens, entités de départ, objets.
	// Lecture seule ici : se modifie uniquement avec le mode « Board Editor » du Level Editor.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board|Level")
	FBoardLayout Layout;

#if WITH_EDITORONLY_DATA
	// Ancien format (liens bloqués entre cases voisines). Le suffixe _DEPRECATED permet de relire
	// les niveaux sauvegardés avant le Level Editor, sans jamais réécrire la valeur.
	// TEMPORAIRE : à supprimer quand tous les niveaux auront été migrés.
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par Layout.Links (liens explicites)."))
	TSet<FBoardLink> BlockedLinks_DEPRECATED;
#endif

	// --- Aperçu de la grille (éditeur uniquement) ---
	UPROPERTY(EditAnywhere, Category = "Board|Grid Preview")
	bool bShowGrid = true;

	// N'affiche la grille que lorsque le Board est sélectionné
	UPROPERTY(EditAnywhere, Category = "Board|Grid Preview")
	bool bOnlyWhenSelected = false;

	// Nombre de cases affichées de chaque côté de l'origine (10 → de -10 à +10)
	UPROPERTY(EditAnywhere, Category = "Board|Grid Preview")
	FIntPoint GridExtent = FIntPoint(10, 10);

	UPROPERTY(EditAnywhere, Category = "Board|Grid Preview")
	FColor GridColor = FColor(80, 200, 255);

	// Légère surélévation pour que les lignes ne disparaissent pas dans le sol (cm)
	UPROPERTY(EditAnywhere, Category = "Board|Grid Preview")
	float GridZOffset = 2.f;

private:
#if WITH_EDITOR
	void DrawGridPreview() const;
#endif

protected:
	UPROPERTY(VisibleAnywhere, Category = "Board")
	TObjectPtr<USceneComponent> SceneRoot;

	// Un seul composant pour tous les traits (le mesh est choisi dans le Blueprint)
	UPROPERTY(VisibleAnywhere, Category = "Board")
	TObjectPtr<UInstancedStaticMeshComponent> LinkMeshes;

	// Épaisseur du trait (échelle Y/Z appliquée au mesh)
	UPROPERTY(EditAnywhere, Category = "Board|Visuals", meta = (ClampMin = "0.01"))
	float LinkThickness = 0.1f;

	// Espace laissé entre le bout du trait et le centre de chaque case (cm)
	UPROPERTY(EditAnywhere, Category = "Board|Visuals", meta = (ClampMin = "0"))
	float LinkGap = 40.f;

	// Décalage vertical du trait par rapport aux cases (cm)
	UPROPERTY(EditAnywhere, Category = "Board|Visuals")
	float LinkZOffset = 0.f;
};
