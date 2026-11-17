#include "BoardLayoutSync.h"
#include "PlagueTaleGOEditor/PlagueTaleGOEditor.h"
#include "PlagueTaleGOEditor/Definitions/BoardDefinitionRegistry.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "PlagueTaleGO/Board/BoardTags.h"
#include "PlagueTaleGO/Entities/ABoardPawn.h"
#include "PlagueTaleGO/Items/BoardItem.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGO/Level/BoardPlacementSettings.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "PlagueTaleGO/Level/Definitions/BoardEntityType.h"
#include "PlagueTaleGO/Level/Definitions/BoardItemType.h"
#include "Engine/Level.h"
#include "EngineUtils.h"         // TActorIterator
#include "ScopedTransaction.h"   // FScopedTransaction (Ctrl+Z)
#include "UObject/Package.h"     // GetTransientPackage

namespace
{
	// Dossier de l'Outliner où sont rangées les cases générées
	const FName NodesFolder(TEXT("Board/Nodes"));

	// Crée l'acteur d'une case, dans le même niveau que le Board
	ABoardNode* SpawnNode(ABoard& Board, UClass* NodeClass, FIntPoint Coord)
	{
		UWorld* World = Board.GetWorld();
		if (!World || !NodeClass)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.OverrideLevel = Board.GetLevel();
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ABoardNode* Node = World->SpawnActor<ABoardNode>(NodeClass, Board.GridToWorld(Coord), FRotator::ZeroRotator, Params);
		if (!Node)
		{
			UE_LOG(LogBoardEditor, Error, TEXT("[BoardEditor] Impossible de créer la case (%d, %d) avec la classe %s."),
				Coord.X, Coord.Y, *GetNameSafe(NodeClass));
			return nullptr;
		}

		// Nom lisible dans l'Outliner, rangé dans un dossier dédié
		Node->SetActorLabel(FString::Printf(TEXT("Node_%d_%d"), Coord.X, Coord.Y));
		Node->SetFolderPath(NodesFolder);
		return Node;
	}

	FString CoordToString(FIntPoint Coord)
	{
		return FString::Printf(TEXT("(%d, %d)"), Coord.X, Coord.Y);
	}
}

ABoard* FBoardLayoutSync::FindBoard(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}

	ABoard* Found = nullptr;
	int32 Count = 0;
	for (TActorIterator<ABoard> It(World); It; ++It)
	{
		if (!Found)
		{
			Found = *It;
		}
		++Count;
	}

	if (Count == 0)
	{
		UE_LOG(LogBoardEditor, Error, TEXT("[BoardEditor] Aucun ABoard dans le niveau : placer un BP_Board."));
	}
	else if (Count > 1)
	{
		UE_LOG(LogBoardEditor, Warning, TEXT("[BoardEditor] %d ABoard dans le niveau : seul %s est utilisé."),
			Count, *GetNameSafe(Found));
	}
	return Found;
}

void FBoardLayoutSync::Submit(ABoard& Board, const FBoardLayout& NewLayout, const FText& TransactionName)
{
	// Tout ce qui est modifié (avec Modify()) pendant la vie de cet objet forme UNE étape d'annulation
	const FScopedTransaction Transaction(TransactionName);

	Board.SetLayout(NewLayout);
	SyncNodeActors(Board);
}

void FBoardLayoutSync::SyncNodeActors(ABoard& Board)
{
	UWorld* World = Board.GetWorld();
	ULevel* Level = Board.GetLevel();
	if (!World || !Level)
	{
		return;
	}

	// Le niveau va gagner ou perdre des acteurs : on l'enregistre dans la transaction
	Level->Modify();

	// 1. Cases déjà présentes dans la scène, rangées par coordonnées
	TMap<FIntPoint, ABoardNode*> Existing;
	TArray<ABoardNode*> ToDestroy;
	for (TActorIterator<ABoardNode> It(World); It; ++It)
	{
		ABoardNode* Node = *It;
		const FIntPoint Coord = Board.WorldToGrid(Node->GetActorLocation());
		if (Existing.Contains(Coord))
		{
			ToDestroy.Add(Node);   // doublon sur la même case
			continue;
		}
		Existing.Add(Coord, Node);
	}

	// 2. Une case par cellule du layout. On réutilise l'acteur existant si sa classe convient,
	//    pour ne pas changer inutilement les acteurs (et garder un historique Ctrl+Z léger).
	int32 SpawnedCount = 0;
	int32 UpdatedCount = 0;
	for (const FBoardCell& Cell : Board.GetLayout().Cells)
	{
		UClass* WantedClass = (Cell.NodeType && Cell.NodeType->NodeClass)
			? Cell.NodeType->NodeClass.Get()
			: ABoardNode::StaticClass();

		ABoardNode* Node = nullptr;
		Existing.RemoveAndCopyValue(Cell.Coord, Node);

		if (Node && Node->GetClass() != WantedClass)
		{
			ToDestroy.Add(Node);
			Node = nullptr;
		}

		if (Node)
		{
			++UpdatedCount;
		}
		else
		{
			Node = SpawnNode(Board, WantedClass, Cell.Coord);
			if (!Node)
			{
				continue;
			}
			++SpawnedCount;
		}

		Node->Modify();
		Node->SetNodeType(Cell.NodeType);

		// Une case générée ne se déplace pas à la main (sa position vient de la grille)
		Node->SetLockLocation(true);
	}

	// 3. Cases qui ne sont plus dans le layout
	for (const TPair<FIntPoint, ABoardNode*>& Pair : Existing)
	{
		ToDestroy.Add(Pair.Value);
	}
	for (ABoardNode* Node : ToDestroy)
	{
		World->EditorDestroyActor(Node, /*bShouldModifyLevel*/ true);
	}

	Board.RebuildLinkVisuals();

	UE_LOG(LogBoardEditor, Log, TEXT("[BoardEditor] Scène synchronisée : %d case(s) créée(s), %d mise(s) à jour, %d supprimée(s)."),
		SpawnedCount, UpdatedCount, ToDestroy.Num());
}

FBoardLayout FBoardLayoutSync::BuildLayoutFromLegacyScene(ABoard& Board, TArray<FString>& OutReport)
{
	FBoardLayout Layout;
	UWorld* World = Board.GetWorld();
	if (!World)
	{
		return Layout;
	}

	// 1. Types de cases correspondant aux anciens booléens : le premier asset trouvé pour chaque cas
	UBoardNodeType* FloorType = nullptr;     // aucun comportement
	UBoardNodeType* ExitType = nullptr;      // Board.Node.Exit
	UBoardNodeType* BlockedType = nullptr;   // Board.Node.Blocked
	for (UBoardNodeType* Type : FBoardDefinitionRegistry::GetAll<UBoardNodeType>())
	{
		const bool bExit = Type->HasBehavior(BoardTags::Node_Exit);
		const bool bBlocked = Type->HasBehavior(BoardTags::Node_Blocked);
		if (bExit && !bBlocked && !ExitType)
		{
			ExitType = Type;
		}
		else if (bBlocked && !bExit && !BlockedType)
		{
			BlockedType = Type;
		}
		else if (!bExit && !bBlocked && !FloorType)
		{
			FloorType = Type;
		}
	}

	const TArray<UBoardEntityType*> EntityTypes = FBoardDefinitionRegistry::GetAll<UBoardEntityType>();
	const TArray<UBoardItemType*> ItemTypes = FBoardDefinitionRegistry::GetAll<UBoardItemType>();

	// 2. Une cellule par case de la scène
	for (TActorIterator<ABoardNode> It(World); It; ++It)
	{
		const ABoardNode* Node = *It;
		const FIntPoint Coord = Board.WorldToGrid(Node->GetActorLocation());
		const FString Where = CoordToString(Coord);

		if (Layout.HasCell(Coord))
		{
			OutReport.Add(FString::Printf(TEXT("%s : plusieurs cases au même endroit, %s ignorée."), *Where, *GetNameSafe(Node)));
			continue;
		}

		UBoardNodeType* NodeType = FloorType;
		const TCHAR* Needed = TEXT("sans comportement");
		if (Node->IsLegacyExit())
		{
			NodeType = ExitType;
			Needed = TEXT("avec le comportement Board.Node.Exit");
		}
		else if (Node->IsLegacyBlocked())
		{
			NodeType = BlockedType;
			Needed = TEXT("avec le comportement Board.Node.Blocked");
		}
		if (!NodeType)
		{
			OutReport.AddUnique(FString::Printf(TEXT("Aucun type de case %s : créer un Data Asset UBoardNodeType, puis relancer l'import."), Needed));
		}

		FBoardCell& Cell = Layout.AddCell(Coord, NodeType);

		// Entité de départ : on cherche le type d'entité dont PawnClass correspond
		if (UClass* PawnClass = Node->GetLegacyStartingEntity().Get())
		{
			UBoardEntityType* const* Found = EntityTypes.FindByPredicate([PawnClass](const UBoardEntityType* Type)
			{
				return Type && Type->PawnClass.Get() == PawnClass;
			});
			if (Found)
			{
				FBoardEntityPlacement& Entity = Cell.Entities.AddDefaulted_GetRef();
				Entity.Type = *Found;
				// Objet temporaire : ABoard::SetLayout en fera une copie qui lui appartient
				Entity.Settings = (*Found)->CreateSettings(GetTransientPackage());
			}
			else
			{
				OutReport.Add(FString::Printf(TEXT("%s : aucun UBoardEntityType avec PawnClass = %s, entité ignorée."),
					*Where, *GetNameSafe(PawnClass)));
			}
		}

		// Objet : même principe avec ItemClass
		if (UClass* ItemClass = Node->GetLegacyItem().Get())
		{
			UBoardItemType* const* Found = ItemTypes.FindByPredicate([ItemClass](const UBoardItemType* Type)
			{
				return Type && Type->ItemClass.Get() == ItemClass;
			});
			if (Found)
			{
				FBoardItemPlacement& Item = Cell.Items.AddDefaulted_GetRef();
				Item.Type = *Found;
				Item.Settings = (*Found)->CreateSettings(GetTransientPackage());
			}
			else
			{
				OutReport.Add(FString::Printf(TEXT("%s : aucun UBoardItemType avec ItemClass = %s, objet ignoré."),
					*Where, *GetNameSafe(ItemClass)));
			}
		}
	}

	// 3. Liens : l'ancien modèle reliait toutes les voisines, sauf les liens bloqués du Board.
	//    On recrée donc un lien explicite pour chaque paire de voisines non bloquée (est et nord : une fois par paire).
	const TSet<FBoardLink>& BlockedLinks = Board.GetLegacyBlockedLinks();
	const FIntPoint Directions[] = { FIntPoint(1, 0), FIntPoint(0, 1) };
	TArray<FIntPoint> Coords;
	for (const FBoardCell& Cell : Layout.Cells)
	{
		Coords.Add(Cell.Coord);
	}
	for (const FIntPoint& Coord : Coords)
	{
		for (const FIntPoint& Direction : Directions)
		{
			const FIntPoint Other = Coord + Direction;
			if (Layout.HasCell(Other) && !BlockedLinks.Contains(FBoardLink::Make(Coord, Other)))
			{
				Layout.AddLink(Coord, Other);
			}
		}
	}

	return Layout;
}
