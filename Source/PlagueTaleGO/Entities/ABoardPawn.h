// BoardPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "ABoardPawn.generated.h"

class ABoardNode;
class UStaticMeshComponent;
class ABoardPawn;
class UBoardPlacementSettings;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEntityMoveFinished, ABoardPawn*, Entity);

UCLASS(Abstract)
class PLAGUETALEGO_API ABoardPawn : public AActor
{
	GENERATED_BODY()

public:
	ABoardPawn();
	void SnapToNode(ABoardNode* Node);

	/**
	 * Reçoit les réglages de son placement (Level Editor) juste après sa création, AVANT BeginPlay.
	 * Settings est nul si le type d'entité n'a pas de SettingsClass.
	 * À surcharger dans les pions qui ont des réglages : Cast<UMesReglages>(Settings).
	 */
	virtual void ApplyPlacementSettings(const UBoardPlacementSettings* Settings) {}

	// Suppose la case déjà validée par le Board
	UFUNCTION(BlueprintCallable, Category = "Board")
	void BeginMove(ABoardNode* InTargetNode);

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsMoving() const { return State == EBoardPawnState::Moving; }

	UFUNCTION(BlueprintPure, Category = "Board")
	ABoardNode* GetCurrentNode() const;

	// Demande au Board la permission de se déplacer
	UFUNCTION(BlueprintCallable, Category = "Board|Player")
	EMoveResult RequestMove(ABoardNode* InTargetNode);

	UPROPERTY(BlueprintAssignable, Category = "Board")
	FOnEntityMoveFinished OnMoveFinished;

protected:
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<ABoardNode> TargetNode;

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