#include "BoardLayout.h"
#include "BoardPlacementSettings.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"

const FBoardCell* FBoardLayout::FindCell(FIntPoint Coord) const
{
	return Cells.FindByPredicate([Coord](const FBoardCell& Cell) { return Cell.Coord == Coord; });
}

FBoardCell* FBoardLayout::FindCell(FIntPoint Coord)
{
	return Cells.FindByPredicate([Coord](const FBoardCell& Cell) { return Cell.Coord == Coord; });
}

FBoardCell& FBoardLayout::AddCell(FIntPoint Coord, UBoardNodeType* NodeType)
{
	if (FBoardCell* Existing = FindCell(Coord))
	{
		return *Existing;
	}

	FBoardCell& Cell = Cells.AddDefaulted_GetRef();
	Cell.Coord = Coord;
	Cell.NodeType = NodeType;
	return Cell;
}

bool FBoardLayout::RemoveCell(FIntPoint Coord)
{
	const int32 Removed = Cells.RemoveAll([Coord](const FBoardCell& Cell) { return Cell.Coord == Coord; });
	if (Removed == 0)
	{
		return false;
	}

	// Un lien vers une case qui n'existe plus n'a pas de sens
	for (auto It = Links.CreateIterator(); It; ++It)
	{
		if (It->Involves(Coord))
		{
			It.RemoveCurrent();
		}
	}
	return true;
}

bool FBoardLayout::HasLink(FIntPoint A, FIntPoint B) const
{
	return Links.Contains(FBoardLink::Make(A, B));
}

bool FBoardLayout::CanLink(FIntPoint A, FIntPoint B) const
{
	return A != B && FBoardLink::AreAdjacent(A, B) && HasCell(A) && HasCell(B);
}

bool FBoardLayout::AddLink(FIntPoint A, FIntPoint B)
{
	if (!CanLink(A, B) || HasLink(A, B))
	{
		return false;
	}
	Links.Add(FBoardLink::Make(A, B));
	return true;
}

bool FBoardLayout::RemoveLink(FIntPoint A, FIntPoint B)
{
	return Links.Remove(FBoardLink::Make(A, B)) > 0;
}

TArray<FIntPoint> FBoardLayout::GetLinkedCoords(FIntPoint Coord) const
{
	TArray<FIntPoint> Result;
	for (const FBoardLink& Link : Links)
	{
		if (Link.Involves(Coord))
		{
			Result.Add(Link.GetOther(Coord));
		}
	}
	return Result;
}

int32 FBoardLayout::CountLinks(FIntPoint Coord) const
{
	int32 Count = 0;
	for (const FBoardLink& Link : Links)
	{
		if (Link.Involves(Coord))
		{
			++Count;
		}
	}
	return Count;
}

TSet<FIntPoint> FBoardLayout::GetReachableCoords(FIntPoint Start, TFunction<bool(const FBoardCell&)> CanEnter) const
{
	TSet<FIntPoint> Visited;
	if (!HasCell(Start))
	{
		return Visited;
	}

	// File d'attente du parcours (on avance avec un index plutôt que de retirer en tête)
	TArray<FIntPoint> Queue = { Start };
	Visited.Add(Start);

	for (int32 Index = 0; Index < Queue.Num(); ++Index)
	{
		for (const FIntPoint& Next : GetLinkedCoords(Queue[Index]))
		{
			if (Visited.Contains(Next))
			{
				continue;
			}

			const FBoardCell* NextCell = FindCell(Next);
			if (!NextCell)
			{
				continue;   // lien vers une case absente : signalé par la validation
			}
			if (CanEnter && !CanEnter(*NextCell))
			{
				continue;
			}

			Visited.Add(Next);
			Queue.Add(Next);
		}
	}
	return Visited;
}

void FBoardLayout::DeepCopySettings(UObject* NewOuter)
{
	if (!NewOuter)
	{
		return;
	}

	// DuplicateObject crée une copie complète de l'objet, rattachée au nouveau propriétaire
	auto Duplicate = [NewOuter](TObjectPtr<UBoardPlacementSettings>& Settings)
	{
		if (Settings)
		{
			Settings = DuplicateObject<UBoardPlacementSettings>(Settings, NewOuter);
		}
	};

	for (FBoardCell& Cell : Cells)
	{
		Duplicate(Cell.NodeSettings);
		for (FBoardEntityPlacement& Entity : Cell.Entities)
		{
			Duplicate(Entity.Settings);
		}
		for (FBoardItemPlacement& Item : Cell.Items)
		{
			Duplicate(Item.Settings);
		}
	}
}
