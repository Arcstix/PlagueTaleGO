// BoardPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Node/BoardTypes.h"
#include "ABoardPawn.generated.h"

class AANode;
class UStaticMeshComponent;
class ABoardPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityMoveFinished, ABoardPawn*, Entity);

UCLASS(Abstract)
class PLAGUETALEGO_API ABoardPawn : public AActor
{
	GENERATED_BODY()

public:
	ABoardPawn();

	// Suppose la case déjà validée par le Board
	UFUNCTION(BlueprintCallable, Category = "Board")
	void BeginMove(AANode* InTargetNode);

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsMoving() const { return State == EBoardPawnState::Moving; }

	UFUNCTION(BlueprintPure, Category = "Board")
	AANode* GetCurrentNode() const { return CurrentNode; }

	UPROPERTY(BlueprintAssignable, Category = "Board")
	FOnEntityMoveFinished OnMoveFinished;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UStaticMeshComponent> Mesh;

	// Nœud de départ, à régler dans l'éditeur
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<AANode> StartNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<AANode> CurrentNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<AANode> LastNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<AANode> TargetNode;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	EBoardPawnState State = EBoardPawnState::Idle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Board")
	float MoveDuration = 0.4f;

private:
	void CompleteMove();

	float MoveAlpha = 0.f;
	FVector MoveStartLocation;
	FVector MoveTargetLocation;
};