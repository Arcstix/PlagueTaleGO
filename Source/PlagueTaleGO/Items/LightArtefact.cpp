// Fill out your copyright notice in the Description page of Project Settings.


#include "LightArtefact.h"
#include "PlagueTaleGO/Board/BoardSubsystem.h"

ULightArtefact::ULightArtefact()
{
	// Nature de l'objet : le joueur choisit une direction avant de l'utiliser
	Targeting = EBoardItemTargeting::Direction;
}

TArray<ABoardNode*> ULightArtefact::Use(const FBoardItemUseContext& Context)
{
	if (!Context.Board || !Context.OriginNode)
	{
		return {};
	}
	// Le plateau fait le calcul : il est la seule autorité sur la grille
	return Context.Board->GetNodesInDirection(Context.OriginNode, Context.Direction, Range, bStopAtGap);
}