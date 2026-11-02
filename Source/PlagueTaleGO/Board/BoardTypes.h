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

// un lien entre deux cases, toujours rangé dans le même ordre
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
	bool operator==(const FBoardLink& O) const { return A == O.A && B == O.B; }
	friend uint32 GetTypeHash(const FBoardLink& L) { return HashCombine(GetTypeHash(L.A), GetTypeHash(L.B)); }
};