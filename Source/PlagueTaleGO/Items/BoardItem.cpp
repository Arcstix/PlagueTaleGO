// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardItem.h"

TArray<ABoardNode*> UBoardItem::Use(const FBoardItemUseContext& Context)
{
	// Base abstraite : aucun effet. Les objets concrets (ex. UDirectionalItem) surchargent cette fonction.
	return {};
}