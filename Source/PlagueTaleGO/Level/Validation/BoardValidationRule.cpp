#include "BoardValidationRule.h"
#include "PlagueTaleGO/Level/BoardLayout.h"

void UBoardValidationRule::Run(const FBoardLayout& Layout, FBoardValidationResult& OutResult) const
{
	Validate(Layout, OutResult.Messages);
}

void UBoardValidationRule::Report(TArray<FBoardValidationMessage>& OutMessages, const FText& Text,
	const TArray<FIntPoint>& Cells) const
{
	FBoardValidationMessage& Message = OutMessages.AddDefaulted_GetRef();
	Message.Severity = Severity;
	Message.Text = Text;
	Message.Cells = Cells;
	Message.RuleName = GetClass()->GetFName();
}

FString UBoardValidationRule::CoordToString(FIntPoint Coord)
{
	return FString::Printf(TEXT("(%d, %d)"), Coord.X, Coord.Y);
}

FString UBoardValidationRule::CoordsToString(const TArray<FIntPoint>& Coords)
{
	// Au-delà de quelques cases, la liste devient illisible : les cases sont de toute façon surlignées
	constexpr int32 MaxListed = 8;

	TArray<FString> Parts;
	for (int32 Index = 0; Index < Coords.Num() && Index < MaxListed; ++Index)
	{
		Parts.Add(CoordToString(Coords[Index]));
	}
	FString Result = FString::Join(Parts, TEXT(", "));
	if (Coords.Num() > MaxListed)
	{
		Result += FString::Printf(TEXT("… (+%d)"), Coords.Num() - MaxListed);
	}
	return Result;
}
