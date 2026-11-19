// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameFlowType.h"
#include "GameFlowSubsystem.generated.h"

class UGameFlowState;
class UBoardSubsystem;
class UBoardItem;
class APlayerPawn;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameFlowStateChanged, UGameFlowState*, NewState);

/**
 * Le chef d'orchestre : SEUL à décider du passage d'un état à un autre.
 * Les états font leur travail puis appellent NotifyStateFinished ;
 * toutes les règles de transition sont dans DecideNextState.
 */
UCLASS()
class PLAGUETALEGO_API UGameFlowSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// Créé seulement en jeu et en PIE (pas dans l'éditeur ni les aperçus)
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Deinitialize() override;

	// Appelé par le GameMode une fois les pions créés
	void StartGame(APlayerPawn* InPlayerPawn);

	// Seul point d'entrée des états pour signaler qu'ils ont fini
	void NotifyStateFinished(UGameFlowState* State);

	// Seul point d'entrée du controller : transmis à l'état courant
	bool SubmitPlayerIntent(const FPlayerIntent& Intent);

	// Survol transmis par le controller : passé à l'état courant s'il accepte les entrées
	void SubmitHover(ABoardNode* Node);

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	bool CanPlayerAct() const;

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	UGameFlowState* GetCurrentState() const { return CurrentState; }

	// --- Données de partie utilisées par les états ---
	UFUNCTION(BlueprintPure, Category = "GameFlow")
	APlayerPawn* GetPlayerPawn() const { return PlayerPawn; }

	UBoardSubsystem* GetBoard() const;

	// Objet ramassé, à utiliser au début du prochain tour joueur
	bool HasPendingItem() const { return PendingItem != nullptr; }

	UFUNCTION(BlueprintPure, Category = "GameFlow")
	UBoardItem* GetPendingItem() const { return PendingItem; }

	// Rend l'objet en attente et le retire (appelé par UItemUseState une fois l'objet utilisé)
	UBoardItem* ConsumePendingItem();

	// UI, debug, sons…
	UPROPERTY(BlueprintAssignable, Category = "GameFlow")
	FOnGameFlowStateChanged OnStateChanged;

private:
	// Toutes les règles de transition sont ICI et nulle part ailleurs
	TSubclassOf<UGameFlowState> DecideNextState() const;

	// Conséquences de l'état qui vient de finir (ex. prendre l'objet de la case).
	// Appelé par Advance juste avant DecideNextState, qui reste const : décider ne modifie rien.
	void HandleFinishedState();

	// Si le joueur est sur une case qui donne un objet : le prend et le met en attente
	void TryPickUpItem();
	
	TSubclassOf<UGameFlowState> StartOfPlayerTurn() const;
	bool IsPlayerCaught() const;

	void Advance();
	void ChangeState(TSubclassOf<UGameFlowState> NextClass);

	UPROPERTY()
	TObjectPtr<UGameFlowState> CurrentState;

	UPROPERTY()
	TObjectPtr<APlayerPawn> PlayerPawn;

	UPROPERTY()
	TObjectPtr<UBoardItem> PendingItem;

	bool bIsChangingState = false;
	bool bAdvanceRequested = false;
};
