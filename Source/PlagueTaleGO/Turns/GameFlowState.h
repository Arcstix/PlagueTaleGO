// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlowType.h"
#include "UObject/Object.h"
#include "GameFlowState.generated.h"

class UGameFlowSubsystem;

/**
 * 
 */
UCLASS(Abstract)
class PLAGUETALEGO_API UGameFlowState : public UObject
{
	GENERATED_BODY()

public:
	// Appelé par le subsystem juste après NewObject, avant Enter()
	void Initialize(UGameFlowSubsystem* InOwner);
	
	virtual void Enter();
	virtual void Exit();
	
	// Renvoie true si l'intention a été acceptée
	virtual bool HandleIntent(const FPlayerIntent& Intent) { return false; }
	
	// Case survolée par le curseur (nullptr = plus rien de survolé).
	// Pur affichage : ne change jamais l'état de la partie. La base ne fait rien.
	virtual void HandleHover(ABoardNode* Node) {}

	// Vrai si cliquer cette case serait accepté par l'état (le controller choisit alors entre jeu et caméra).
	// La base ne rend aucune case jouable.
	virtual bool IsNodeSelectable(ABoardNode* Node) const { return false; }
	
	virtual bool AllowsPlayerInput() const { return false; }

	// Permet aux états d'utiliser GetWorld() (timers, subsystems, logs…)
	virtual UWorld* GetWorld() const override;

protected:
	UGameFlowSubsystem* GetOwner() const;

	// Raccourci : « j'ai fini » → Owner->NotifyStateFinished(this)
	void Finish();

private:
	UPROPERTY()
	TObjectPtr<UGameFlowSubsystem> Owner;
};
