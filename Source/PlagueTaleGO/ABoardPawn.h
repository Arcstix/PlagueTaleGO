// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ABoardPawn.generated.h"

class UBoardNode; // A se mettre d'accord sur le nom de la classe

UENUM(BlueprintType)
enum class EBoardPawnState : uint8 // A séparer surement plus tard dans un autre fichier appart
{
    Idle,
    Moving
}; 

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnMoveFinished, ABoardPawn*, Entity);

UCLASS(Abstract)
class ABoardPawn : public AActor
{
	GENERATED_BODY()

public:
	ABoardPawn();

	// Suppose la destination déjà validée par le plateau
	UFUNCTION(BlueprintCallable, Category = "Board")
	void BeginMove(UBoardNode* InTargetNode);

	UPROPERTY(BlueprintAssignable, Category = "Board")
	FOnPawnMoveFinished OnMoveFinished;

protected:
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardNode> CurrentNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardNode> LastNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardNode> TargetNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	EBoardPawnState State = EBoardPawnState::Idle;

	float MoveAlpha = 0.f; // progression visuelle 0..1

private:
	void CompleteMove();
};
