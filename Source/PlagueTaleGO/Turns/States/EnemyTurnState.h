// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlagueTaleGO/Turns/GameFlowState.h"
#include "EnemyTurnState.generated.h"

class AWormPawn;
class ABoardNode;
class ABoardPawn;

/**
 * Tour ennemi : la statue avance d'une case vers le joueur,
 * puis l'état attend la fin de son animation avant de Finish().
 */
UCLASS()
class PLAGUETALEGO_API UEnemyTurnState : public UGameFlowState
{
	GENERATED_BODY()

public:
	virtual void Enter() override;
	virtual void Exit() override;

private:
	AWormPawn* FindWorm() const;
	
	ABoardNode* FindNextStep(const AWormPawn* InStatue) const;
	
	//    Renvoie false si le Board refuse 
	bool MoveWorm(ABoardNode* TargetNode);
	
	UFUNCTION()
	void HandleStatueMoveFinished(ABoardPawn* MovedPawn);
	
	UPROPERTY()
	TObjectPtr<AWormPawn> Statue;
};
