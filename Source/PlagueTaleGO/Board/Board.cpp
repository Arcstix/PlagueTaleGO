// Fill out your copyright notice in the Description page of Project Settings.


#include "Board.h"
#include "DrawDebugHelpers.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PlagueTaleGO/PlagueTaleGO.h"

ABoard::ABoard()
{
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

#if WITH_EDITOR
void ABoard::SetLayout(const FBoardLayout& NewLayout)
{
	// Modify() enregistre l'état actuel dans la transaction en cours : c'est ce qui rend Ctrl+Z possible
	Modify();

	Layout = NewLayout;

	// Le layout reçu peut venir du brouillon du Level Editor : ses objets de réglages
	// appartiennent à l'outil. On en fait des copies qui appartiennent au Board (sauvegardées avec le niveau).
	Layout.DeepCopySettings(this);

	RebuildLinkVisuals();
}

void ABoard::PostEditUndo()
{
	Super::PostEditUndo();
	RebuildLinkVisuals();
}
#endif

void ABoard::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	// Rejoué aussi au chargement, au déplacement du Board et après un Ctrl+Z
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
	if (!Mesh)
	{
		// Normal sur la classe C++ seule : le mesh est choisi dans BP_Board
		UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[Board] Pas de mesh sur LinkMeshes : aucun trait dessiné."));
		return;
	}

	// Longueur du mesh sur son axe X, pour l'étirer à la bonne taille
	const float MeshLength = FMath::Max(Mesh->GetBounds().BoxExtent.X * 2.f, 1.f);

	// Un trait par lien du layout. Les positions viennent de la grille (le plateau est plat),
	// pas des acteurs ABoardNode : les traits restent justes même si les acteurs ne sont pas encore générés.
	for (const FBoardLink& Link : Layout.Links)
	{
		if (!Layout.HasCell(Link.A) || !Layout.HasCell(Link.B))
		{
			continue;   // lien vers une case absente : signalé par la validation
		}

		const FVector Start = GridToWorld(Link.A);
		const FVector End   = GridToWorld(Link.B);
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

	UE_LOG(LogPlagueTaleGO, Verbose, TEXT("[Board] %d traits dessinés"), LinkMeshes->GetInstanceCount());
}
