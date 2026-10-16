// BoardEntity.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoardTypes.h"
#include "ABoardPawn.generated.h"

class ABoardNode;

// Émis quand l'entité a fini son déplacement visuel
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityMoveFinished, ABoardPawn*, Entity);

UCLASS(Abstract)
class ABoardPawn : public AActor
{
	GENERATED_BODY()

public:
	ABoardPawn();

	// Lance l'animation de déplacement. Appelée par le Board, qui a déjà validé la case.
	// UFUNCTION(BlueprintCallable, Category = "Board")
	// void BeginMove(ABoardNode* InTargetNode);

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsMoving() const { return State == EBoardEntityState::Moving; }

	// UFUNCTION(BlueprintPure, Category = "Board")
	// ABoardNode* GetCurrentNode() const { return CurrentNode; }

	// UFUNCTION(BlueprintPure, Category = "Board")
	// ABoardNode* GetTargetNode() const { return TargetNode; }

	// La StateMachine écoute ce délégué pour savoir quand passer à la suite
	UPROPERTY(BlueprintAssignable, Category = "Board")
	FOnEntityMoveFinished OnMoveFinished;

protected:
	virtual void Tick(float DeltaSeconds) override;

	// Nœud où se trouve visuellement l'entité (mis à jour à la fin du déplacement)
	// UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	// TObjectPtr<ABoardNode> CurrentNode;
	//
	// UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	// TObjectPtr<ABoardNode> LastNode;
	//
	// UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	// TObjectPtr<ABoardNode> TargetNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	EBoardEntityState State = EBoardEntityState::Idle;

	// Durée de l'animation en secondes, réglable dans l'éditeur
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board")
	float MoveDuration = 0.4f;

private:
	void CompleteMove();

	float MoveAlpha = 0.f;
	FVector MoveStartLocation;
};