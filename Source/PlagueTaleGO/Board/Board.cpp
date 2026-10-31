// Fill out your copyright notice in the Description page of Project Settings.


#include "Board.h"
#include "BoardNode.h"
#include "EngineUtils.h"
#include "DrawDebugHelpers.h" 
#include "Components/InstancedStaticMeshComponent.h"
#include "PlagueTaleGO/PlagueTaleGO.h"

#if WITH_EDITOR
#include "Editor.h"            // GEditor
#include "Engine/Selection.h"  // USelection
#endif

ABoard::ABoard()
{
	PrimaryActorTick.bCanEverTick = false;
	// Le Tick sert uniquement à dessiner la grille dans l'éditeur
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	LinkMeshes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("LinkMeshes"));
	LinkMeshes->SetupAttachment(SceneRoot);
	// Les traits ne doivent pas intercepter le clic destiné aux cases
	LinkMeshes->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABoard::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

#if WITH_EDITOR
	DrawGridPreview();
#endif
}

bool ABoard::ShouldTickIfViewportsOnly() const
{
	const UWorld* World = GetWorld();
	return World && World->WorldType == EWorldType::Editor;
}

FIntPoint ABoard::WorldToGrid(const FVector& Location) const
{
	const float Size = FMath::Max(CellSize, 1.f);           // évite une division par 0
	const FVector Local = Location - GetActorLocation();    // position relative au Board

	return FIntPoint(
		FMath::RoundToInt(Local.X / Size),
		FMath::RoundToInt(Local.Y / Size));
}

FVector ABoard::GridToWorld(FIntPoint Coord) const
{
	const FVector Origin = GetActorLocation();

	return FVector(
		Origin.X + Coord.X * CellSize,
		Origin.Y + Coord.Y * CellSize,
		Origin.Z);                                           // hauteur du Board
}

bool ABoard::IsLinkBlocked(FIntPoint A, FIntPoint B) const
{
	return BlockedLinks.Contains(FBoardLink::Make(A, B));
}

void ABoard::ToggleLinkBetweenSelectedNodes()
{
#if WITH_EDITOR
	if (!GEditor) return;

	TArray<ABoardNode*> Selected;
	GEditor->GetSelectedActors()->GetSelectedObjects<ABoardNode>(Selected);
	if (Selected.Num() != 2) return;           // TODO : message d'erreur

	// … vérifier qu'elles sont voisines, puis :
	Modify();                                   // pour Ctrl+Z
	const FBoardLink Link = FBoardLink::Make(Selected[0]->GetGridCoord(), Selected[1]->GetGridCoord());
	if (BlockedLinks.Contains(Link)) BlockedLinks.Remove(Link);
	else                             BlockedLinks.Add(Link);

	RebuildLinkVisuals();
#endif
}



void ABoard::ValidateBoard()
{
}

void ABoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
	// Rejoué aussi quand on modifie une propriété (ex. BlockedLinks) dans le panneau Details
	RebuildLinkVisuals();
}

void ABoard::BeginPlay()
{
	Super::BeginPlay();

	// Sécurité : garantit des traits à jour dans une build finale
	RebuildLinkVisuals();

	// En jeu, la grille n'est pas dessinée : inutile de garder le Tick
	SetActorTickEnabled(false);
}

#if WITH_EDITOR
void ABoard::DrawGridPreview() const
{
	UWorld* World = GetWorld();
	if (!bShowGrid || !World || World->WorldType != EWorldType::Editor)
	{
		return;
	}
	if (bOnlyWhenSelected && !IsSelected())
	{
		return;
	}

	const float Size = FMath::Max(CellSize, 1.f);
	const int32 NX = FMath::Max(GridExtent.X, 1);
	const int32 NY = FMath::Max(GridExtent.Y, 1);
	const FVector Origin = GetActorLocation() + FVector(0.f, 0.f, GridZOffset);

	// Les centres des cases sont sur les multiples de CellSize,
	// donc les bords sont à mi-chemin : (i - 0.5) * CellSize
	const float MinX = (-NX - 0.5f) * Size;
	const float MaxX = ( NX + 0.5f) * Size;
	const float MinY = (-NY - 0.5f) * Size;
	const float MaxY = ( NY + 0.5f) * Size;

	// LifeTime = -1 : la ligne ne dure qu'une image, elle est redessinée au Tick suivant
	constexpr bool bPersistent = false;
	constexpr float LifeTime = -1.f;
	constexpr float Thickness = 1.5f;

	// 1. Bords des cellules : lignes parallèles à Y…
	for (int32 i = -NX; i <= NX + 1; ++i)
	{
		const float X = (i - 0.5f) * Size;
		DrawDebugLine(World, Origin + FVector(X, MinY, 0.f), Origin + FVector(X, MaxY, 0.f),
			GridColor, bPersistent, LifeTime, 0, Thickness);
	}
	// … et parallèles à X
	for (int32 j = -NY; j <= NY + 1; ++j)
	{
		const float Y = (j - 0.5f) * Size;
		DrawDebugLine(World, Origin + FVector(MinX, Y, 0.f), Origin + FVector(MaxX, Y, 0.f),
			GridColor, bPersistent, LifeTime, 0, Thickness);
	}

	// 2. Un point au centre de chaque case (là où les ABoardNode vont se caler)
	for (int32 x = -NX; x <= NX; ++x)
	{
		for (int32 y = -NY; y <= NY; ++y)
		{
			DrawDebugPoint(World, Origin + FVector(x * Size, y * Size, 0.f), 6.f,
				GridColor, bPersistent, LifeTime);
		}
	}

	// 3. Repère sur la case (0, 0) : flèche rouge = +X, flèche verte = +Y
	DrawDebugPoint(World, Origin, 14.f, FColor::Yellow, bPersistent, LifeTime);
	DrawDebugDirectionalArrow(World, Origin, Origin + FVector(Size * 0.5f, 0.f, 0.f), 25.f,
		FColor::Red, bPersistent, LifeTime, 0, 3.f);
	DrawDebugDirectionalArrow(World, Origin, Origin + FVector(0.f, Size * 0.5f, 0.f), 25.f,
		FColor::Green, bPersistent, LifeTime, 0, 3.f);
}
#endif

void ABoard::RebuildLinkVisuals()
{
	if (!LinkMeshes)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Board] LinkMeshes est nul"));
		return;
	}
	LinkMeshes->ClearInstances();

	UStaticMesh* Mesh = LinkMeshes->GetStaticMesh();
	UWorld* World = GetWorld();
	if (!Mesh || !World)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Board] Pas de mesh sur LinkMeshes (Mesh=%s)"), *GetNameSafe(Mesh));
		return;   // aucun mesh choisi dans le Blueprint : rien à dessiner
	}

	// Longueur du mesh sur son axe X, pour l'étirer à la bonne taille
	const float MeshLength = FMath::Max(Mesh->GetBounds().BoxExtent.X * 2.f, 1.f);

	// 1. Toutes les cases du niveau, rangées par coordonnées.
	//    (On recalcule les coordonnées depuis la position : en éditeur,
	//     le subsystem n'existe pas et GridCoord peut ne pas être encore à jour.)
	TMap<FIntPoint, ABoardNode*> Nodes;
	for (TActorIterator<ABoardNode> It(World); It; ++It)
	{
		Nodes.Add(WorldToGrid(It->GetActorLocation()), *It);
	}

	UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Board] %d cases trouvées"), Nodes.Num());

	// 2. Pour chaque case, seulement les voisins EST et NORD :
	//    chaque lien n'est ainsi dessiné qu'une fois.
	const FIntPoint Directions[] = { {1, 0}, {0, 1} };

	for (const TPair<FIntPoint, ABoardNode*>& Pair : Nodes)
	{
		UE_LOG(LogPlagueTaleGO, Warning, TEXT("   %s -> (%d, %d)"), *GetNameSafe(Pair.Value), Pair.Key.X, Pair.Key.Y);
		for (const FIntPoint& Dir : Directions)
		{
			const FIntPoint OtherCoord = Pair.Key + Dir;
			ABoardNode* const* Other = Nodes.Find(OtherCoord);
			if (!Other || IsLinkBlocked(Pair.Key, OtherCoord))
			{
				continue;
			}

			const FVector Start = Pair.Value->GetActorLocation();
			const FVector End   = (*Other)->GetActorLocation();
			const FVector Delta = End - Start;
			const float Length  = Delta.Size() - 2.f * LinkGap;
			if (Length <= KINDA_SMALL_NUMBER)
			{
				continue;   // cases trop proches pour dessiner un trait
			}

			const FVector Mid = (Start + End) * 0.5f + FVector(0.f, 0.f, LinkZOffset);
			const FTransform InstanceTransform(
				Delta.Rotation(),
				Mid,
				FVector(Length / MeshLength, LinkThickness, LinkThickness));

			LinkMeshes->AddInstance(InstanceTransform, /*bWorldSpace*/ true);
		}
	}
	UE_LOG(LogPlagueTaleGO, Warning, TEXT("[Board] %d traits dessinés"), LinkMeshes->GetInstanceCount());
}
