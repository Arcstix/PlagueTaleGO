// BoardSubsystem.h
#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.h"
#include "Subsystems/WorldSubsystem.h"
#include "BoardSubsystem.generated.h"

class ABoardNode;
class ABoardPawn;

// Un USTRUCT est nécessaire : UPROPERTY ne gère pas les conteneurs imbriqués
USTRUCT()
struct FNodeOccupants
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<TObjectPtr<ABoardPawn>> Entities;
};

UCLASS()
class PLAGUETALEGO_API UBoardSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Enregistre une entité présente sur un nœud au démarrage
	void RegisterPawn(ABoardPawn* Pawn, ABoardNode* Node);

	// Seule autorité : valide, met à jour l'occupation, lance le déplacement
	EMoveResult TryMovePawn(ABoardPawn* Pawn, ABoardNode* InTargetNode);

	bool IsOccupied(ABoardNode* Node);

private:
	UPROPERTY()
	TMap<TObjectPtr<ABoardNode>, FNodeOccupants> Occupancy;
};