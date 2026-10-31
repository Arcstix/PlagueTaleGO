// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.h"
#include "GameFramework/Actor.h"
#include "Board.generated.h"

class UInstancedStaticMeshComponent;

/**
 * Un seul par niveau. Définit la grille (origine = position de l'acteur),
 * stocke les liens bloqués et dessine tous les liens ouverts.
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
	
	// Passages fermés entre deux cases voisines
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board|Grid")
	TSet<FBoardLink> BlockedLinks;

    FIntPoint WorldToGrid(const FVector& Location) const;
    FVector   GridToWorld(FIntPoint Coord) const;

	// --- Visuels des liens ---
	// Bouton « Rebuild Link Visuals » dans le panneau Details
	UFUNCTION(CallInEditor, Category = "Board|Edition")
	void RebuildLinkVisuals();
	
	bool IsLinkBlocked(FIntPoint A, FIntPoint B) const;

    UFUNCTION(CallInEditor, Category = "Édition") void ToggleLinkBetweenSelectedNodes(); // sélectionner 2 cases → bloquer/débloquer
    UFUNCTION(CallInEditor, Category = "Édition") void ValidateBoard();
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

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
