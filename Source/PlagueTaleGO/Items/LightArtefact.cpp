// Fill out your copyright notice in the Description page of Project Settings.


#include "LightArtefact.h"

ULightArtefact::ULightArtefact()
{
	// Nature de l'objet : le joueur choisit une direction avant de l'utiliser
	Targeting = EBoardItemTargeting::Direction;
}

TArray<ABoardNode*> ULightArtefact::Use(const FBoardItemUseContext& Context)
{
	// Tâche 10 : renverra Context.Board->GetNodesInDirection(Context.OriginNode, Context.Direction, Range, bStopAtGap).
	// En attendant, aucune case n'est touchée.
	return {};
}