// Fill out your copyright notice in the Description page of Project Settings.


#include "BoardItem.h"

TArray<ABoardNode*> UBoardItem::Use(const FBoardItemUseContext& Context)
{
	// Base abstraite : aucun effet. Les objets concrets (ex. UDirectionalItem) surchargent cette fonction.
	return {};
}

TArray<FBoardDirectionOption> UBoardItem::GetDirectionOptions(const FBoardItemUseContext& BaseContext)
{
	TArray<FBoardDirectionOption> Options;
	if (Targeting != EBoardItemTargeting::Direction)
	{
		return Options;
	}

	static const EBoardDirection AllDirections[] =
	{
		EBoardDirection::North, EBoardDirection::East, EBoardDirection::South, EBoardDirection::West
	};

	for (const EBoardDirection Direction : AllDirections)
	{
		FBoardItemUseContext Context = BaseContext;
		Context.Direction = Direction;

		// Use() ne modifie rien : on peut l'appeler pour « essayer » chaque direction
		const TArray<ABoardNode*> Nodes = Use(Context);
		if (Nodes.Num() == 0)
		{
			continue;   // direction sans case dans la portée : pas un choix valide
		}

		FBoardDirectionOption& Option = Options.AddDefaulted_GetRef();
		Option.Direction = Direction;
		for (ABoardNode* Node : Nodes)
		{
			Option.Nodes.Add(Node);
		}
	}

	return Options;
}

bool UBoardItem::FindDirectionOfNode(const TArray<FBoardDirectionOption>& Options, const ABoardNode* Node,
	EBoardDirection& OutDirection)
{
	if (!Node)
	{
		return false;
	}

	for (const FBoardDirectionOption& Option : Options)
	{
		if (Option.Nodes.Contains(Node))
		{
			OutDirection = Option.Direction;
			return true;
		}
	}
	return false;
}
