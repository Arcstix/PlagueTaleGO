#pragma once

#include "CoreMinimal.h"
#include "BoardDefinition.h"
// Include complet (et non une déclaration anticipée) : tester ou lire PawnClass appelle
// ABoardPawn::StaticClass() (vérification de TSubclassOf), qui exige la classe complète.
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "BoardEntityType.generated.h"

/**
 * Un type d'entité de départ (joueur, futurs ennemis…).
 * Le Level Editor place des entités de ce type sur les cases ; le GameMode crée le pion
 * (PawnClass) au lancement de la partie, avec l'orientation et les réglages du placement.
 *
 * Le joueur est l'entité dont PawnClass dérive de APlayerPawn (un seul par niveau).
 */
UCLASS(BlueprintType, meta = (ToolTip = "A starting entity type for the Board Editor: player, enemies..."))
class PLAGUETALEGO_API UBoardEntityType : public UBoardDefinition
{
	GENERATED_BODY()

public:
	// Vrai si ce type crée le pion joueur
	UFUNCTION(BlueprintPure, Category = "Entity")
	bool IsPlayer() const;

	// Pion créé au lancement de la partie (BP enfant d'une classe C++ de pion)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Entity", meta = (ToolTip = "Pawn spawned when the game starts (Blueprint child of a C++ pawn class). A child of APlayerPawn = the player."))
	TSubclassOf<ABoardPawn> PawnClass;
};
