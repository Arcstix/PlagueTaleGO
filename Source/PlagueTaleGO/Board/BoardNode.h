	// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardHighlightType.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "BoardNode.generated.h"

class ABoardPawn;
class UBoardItem;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UBoardNodeType;
class ABoard;
class UBoardCameraView;

/**
 * Une case du plateau, telle qu'elle existe dans la scène.
 *
 * Les cases sont GÉNÉRÉES par le Level Editor à partir du layout du ABoard (Submit) :
 * on ne les place pas et on ne les règle pas à la main.
 * NotPlaceable : la classe (et ses BP enfants) n'apparaît pas dans « Place Actors » et ne se glisse pas
 * dans le niveau ; le Level Editor la crée par code. Sa position est verrouillée à la génération.
 * - Son visuel et ses comportements viennent de son type (UBoardNodeType).
 * - Elle ne connaît PAS ses voisines : UBoardSubsystem les calcule à partir
 *   des liens explicites du layout.
 * - Les entités et objets de départ sont dans le layout, pas sur la case.
 */
UCLASS(NotPlaceable)
class PLAGUETALEGO_API ABoardNode : public AActor
{
	GENERATED_BODY()

public:
	ABoardNode();

	// --- Lecture (utilisée par le subsystem, le GameMode, les états) ---
	UFUNCTION(BlueprintPure, Category = "Board")
	FIntPoint GetGridCoord() const { return GridCoord; }

	UFUNCTION(BlueprintPure, Category = "Board")
	UBoardNodeType* GetNodeType() const { return NodeType; }

	// Point de vue de la case (copié du layout à la génération). nullptr = aucun.
	UBoardCameraView* GetCameraView() const { return CameraView; }
	void SetCameraView(UBoardCameraView* InCameraView);

	// Vrai si le type de la case possède ce comportement (Gameplay Tag, ex. Board.Node.Exit)
	UFUNCTION(BlueprintPure, Category = "Board")
	bool HasBehavior(FGameplayTag Behavior) const;

	// Case infranchissable : comportement Board.Node.Blocked OU blocage temporaire en jeu (SetBlocked)
	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsBlocked() const;

	// Case de sortie : comportement Board.Node.Exit
	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsExit() const;

	// Blocage temporaire en jeu (obstacle, effet d'objet…). Ne modifie pas le niveau.
	void SetBlocked(bool bValue) { bRuntimeBlocked = bValue; }

	// Change le type de la case et met à jour son visuel.
	// Appelé par le Level Editor à la génération ; utilisable en jeu (ex. une case qui change d'état).
	void SetNodeType(UBoardNodeType* InNodeType);

	// Réapplique le visuel du type actuel (ex. après modification du Data Asset du type dans l'éditeur)
	void RefreshVisuals() { ApplyNodeTypeVisuals(); }

	// --- Surbrillance (visuel uniquement, rien n'est sauvegardé dans le niveau) ---

	// Colore la case : Amount = 0 (couleur normale) à 1 (entièrement Color).
	// Le matériau de la case doit avoir les paramètres HighlightColor et HighlightAmount.
	UFUNCTION(BlueprintCallable, Category = "Board|Visuals")
	void SetHighlight(FLinearColor Color, float Amount);

	// Rend à la case sa couleur normale
	UFUNCTION(BlueprintCallable, Category = "Board|Visuals")
	void ClearHighlight();

	// Joue un fondu de surbrillance. Le Tick n'est activé que pendant le fondu.
	// En fin de fondu, la case reste sur Fade.EndAmount. SetHighlight ou ClearHighlight l'interrompent.
	UFUNCTION(BlueprintCallable, Category = "Board|Visuals")
	void PlayHighlightFade(const FBoardHighlightFade& Fade);

	UFUNCTION(BlueprintPure, Category = "Board|Visuals")
	bool IsFading() const { return ActiveFade.Duration > 0.f; }
	
#if WITH_EDITORONLY_DATA
	// --- Ancien format, lu uniquement par la migration (commande BoardEditor.ImportLegacy) ---
	TSubclassOf<ABoardPawn> GetLegacyStartingEntity() const { return StartingEntity_DEPRECATED; }
	TSubclassOf<UBoardItem> GetLegacyItem() const { return Item_DEPRECATED; }
	bool IsLegacyExit() const { return bIsExit_DEPRECATED; }
	bool IsLegacyBlocked() const { return bIsBlocked_DEPRECATED; }
#endif

protected:
	// Cale la case sur la grille et applique le visuel de son type.
	// Rejoué au chargement, au déplacement et après un Ctrl+Z dans l'éditeur.
	virtual void OnConstruction(const FTransform& Transform) override;

	// Utilisé uniquement pendant un fondu (activé par PlayHighlightFade, coupé à la fin)
	virtual void Tick(float DeltaSeconds) override;

#if WITH_EDITOR
	// Après un Ctrl+Z, NodeType a pu changer : on réapplique son visuel
	virtual void PostEditUndo() override;
#endif

	// --- Composants ---

	// Mesh de la case, choisi par son type. Bloque le canal Visibility : c'est lui que le clic du joueur touche.
	UPROPERTY(VisibleAnywhere, Category = "Board")
	TObjectPtr<UStaticMeshComponent> NodeMesh;

	// --- Données (générées par le Level Editor : non éditables) ---

	// Coordonnées sur la grille, calculées depuis la position
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	FIntPoint GridCoord = FIntPoint::ZeroValue;

	// Type de la case (visuel + comportements)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardNodeType> NodeType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardCameraView> CameraView;

	// --- État d'exécution ---

	// Blocage temporaire posé en jeu. Transient : jamais sauvegardé avec le niveau.
	UPROPERTY(VisibleInstanceOnly, Transient, Category = "Board")
	bool bRuntimeBlocked = false;

#if WITH_EDITORONLY_DATA
	// --- Ancien format (avant le Level Editor) ---
	// Le suffixe _DEPRECATED permet de relire les niveaux existants sans jamais réécrire ces valeurs.
	// TEMPORAIRE : à supprimer quand tous les niveaux auront été migrés.
	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le layout du ABoard."))
	TSubclassOf<ABoardPawn> StartingEntity_DEPRECATED;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le layout du ABoard."))
	TSubclassOf<UBoardItem> Item_DEPRECATED;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le comportement Board.Node.Exit."))
	bool bIsExit_DEPRECATED = false;

	UPROPERTY(meta = (DeprecatedProperty, DeprecationMessage = "Remplacé par le comportement Board.Node.Blocked."))
	bool bIsBlocked_DEPRECATED = false;
#endif

private:
	// Applique le mesh, l'échelle et les matériaux du type sur NodeMesh
	void ApplyNodeTypeVisuals();

	// Le ABoard du niveau, ou nullptr s'il n'y en a pas
	ABoard* FindBoard() const;

	// Crée au premier besoin une instance dynamique du matériau pour chaque slot du mesh
	void EnsureHighlightMaterials();

	// Règle les paramètres du matériau, sans toucher au fondu en cours
	void ApplyHighlight(FLinearColor Color, float Amount);

	// Arrête le fondu en cours et coupe le Tick
	void StopFade();

	// Fondu en cours (ActiveFade.Duration = 0 : aucun fondu)
	FBoardHighlightFade ActiveFade;
	float FadeElapsed = 0.f;

	// Instances dynamiques utilisées pour la surbrillance. Transient : créées en jeu, jamais sauvegardées.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> HighlightMaterials;
};
