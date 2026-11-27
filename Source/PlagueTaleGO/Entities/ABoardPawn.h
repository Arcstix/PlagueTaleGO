// BoardPawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlagueTaleGO/Board/BoardTypes.h"
#include "GameplayTagContainer.h"
#include "ABoardPawn.generated.h"

class ABoardNode;
class UStaticMeshComponent;
class ABoardPawn;
class UBoardPlacementSettings;
class UBoardEntityType;

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

	// Type d'entité (Data Asset du Level Editor) qui a créé ce pion. nullptr si le pion a été posé autrement.
	// Sert notamment de « contexte » audio : sons propres au type (UBoardDefinition::SoundOverrides).
	UFUNCTION(BlueprintPure, Category = "Board")
	UBoardEntityType* GetEntityType() const { return EntityType; }

	// Appelé par le GameMode au spawn, AVANT BeginPlay (comme ApplyPlacementSettings)
	void SetEntityType(UBoardEntityType* InEntityType) { EntityType = InEntityType; }

	// Suppose la case déjà validée par le Board
	UFUNCTION(BlueprintCallable, Category = "Board")
	void BeginMove(ABoardNode* InTargetNode);

	UFUNCTION(BlueprintPure, Category = "Board")
	bool IsMoving() const { return State == EBoardPawnState::Moving; }

	// Durée d'un déplacement d'une case (s). Sert à synchroniser des effets avec le pion (trace de poussière bleue…)
	UFUNCTION(BlueprintPure, Category = "Board")
	float GetMoveDuration() const { return MoveDuration; }

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

	// Type d'entité qui a créé ce pion (voir GetEntityType)
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Board")
	TObjectPtr<UBoardEntityType> EntityType;

	// Événement sonore joué au début de chaque déplacement (pas, glissement…), réglé dans le BP de chaque pion. Vide = aucun son.
	// Contexte : le type de la case de destination (ses SoundOverrides peuvent remplacer le son, ex. pas sur du bois).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Board|Audio", meta = (Categories = "Audio", ToolTip = "Sound event played when the pawn starts a move (footstep...). Empty = no sound. The destination cell type can replace the sound (Sound Overrides)."))
	FGameplayTag MoveSoundEvent;

private:
	void CompleteMove();

	// Joue MoveSoundEvent avec le type de la case de destination en contexte (rien si le tag est vide)
	void PlayMoveSound(const ABoardNode* DestinationNode) const;

	float MoveAlpha = 0.f;
	FVector MoveStartLocation;
	FVector MoveTargetLocation;
};