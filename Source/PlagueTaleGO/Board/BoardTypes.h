#pragma once

#include "CoreMinimal.h"
#include "BoardTypes.generated.h"

UENUM(BlueprintType)
enum class EBoardPawnState : uint8
{
	Idle,
	Moving
};

// Résultat renvoyé par le Board à chaque demande de déplacement
UENUM(BlueprintType)
enum class EMoveResult : uint8
{
	Accepted,
	NotAdjacent,
	NoOrigin,
	Blocked,
	Occupied,
	AlreadyMoving
};

// Direction sur la grille, dans le repère du Board :
// +X = Nord (flèche rouge de l'aperçu de grille), +Y = Est (flèche verte).
UENUM(BlueprintType)
enum class EBoardDirection : uint8
{
	North	UMETA(DisplayName = "Nord (+X)"),
	East	UMETA(DisplayName = "Est (+Y)"),
	South	UMETA(DisplayName = "Sud (-X)"),
	West	UMETA(DisplayName = "Ouest (-Y)")
};

namespace BoardDirection
{
	// Décalage de grille correspondant à la direction (ex. North → (1, 0))
	inline FIntPoint ToGridOffset(EBoardDirection Direction)
	{
		switch (Direction)
		{
		case EBoardDirection::East:
			return FIntPoint(0, 1);
		case EBoardDirection::South:
			return FIntPoint(-1, 0);
		case EBoardDirection::West:
			return FIntPoint(0, -1);
		default:
			return FIntPoint(1, 0);
		}
	}

	// Rotation (Yaw, en degrés) d'un acteur qui regarde dans cette direction
	inline float ToYaw(EBoardDirection Direction)
	{
		return 90.f * static_cast<uint8>(Direction);
	}
}

// Un lien explicite entre deux cases voisines, toujours rangé dans le même ordre.
// Deux cases ne sont reliées QUE si un lien existe (tracé dans le Level Editor).
USTRUCT(BlueprintType)
struct FBoardLink
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere) FIntPoint A;
	UPROPERTY(EditAnywhere) FIntPoint B;

	static FBoardLink Make(FIntPoint P, FIntPoint Q)
	{   // ordre canonique : (1,2)-(1,3) et (1,3)-(1,2) sont le même lien
		return (P.X < Q.X || (P.X == Q.X && P.Y < Q.Y)) ? FBoardLink{P, Q} : FBoardLink{Q, P};
	}

	// Vrai si P et Q sont voisines orthogonales : exactement 1 case d'écart, pas de diagonale
	static bool AreAdjacent(FIntPoint P, FIntPoint Q)
	{
		const FIntPoint Delta = Q - P;
		return FMath::Abs(Delta.X) + FMath::Abs(Delta.Y) == 1;
	}

	// Vrai si P est une des deux extrémités du lien
	bool Involves(FIntPoint P) const { return A == P || B == P; }

	// L'autre extrémité du lien (suppose Involves(P))
	FIntPoint GetOther(FIntPoint P) const { return A == P ? B : A; }
	bool operator==(const FBoardLink& O) const { return A == O.A && B == O.B; }
	friend uint32 GetTypeHash(const FBoardLink& L) { return HashCombine(GetTypeHash(L.A), GetTypeHash(L.B)); }
};