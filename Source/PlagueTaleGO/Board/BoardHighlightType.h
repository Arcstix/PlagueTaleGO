// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BoardHighlightType.generated.h"

/**
 * Réglages d'un fondu de surbrillance sur une case (ABoardNode::PlayHighlightFade).
 * La valeur (HighlightAmount du matériau) monte de 0 à 1, puis redescend à 0.
 * PeakRatio fixe le moment du sommet, EaseExponent la douceur de la courbe.
 */
USTRUCT(BlueprintType)
struct PLAGUETALEGO_API FBoardHighlightFade
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Highlight", meta = (
		ToolTip = "Highlight color."))
	FLinearColor Color = FLinearColor(1.f, 1.f, 0.9f);

	// Durée totale du fondu (s)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Highlight", meta = (
		ClampMin = "0.0", Units = "s",
		ToolTip = "Total duration of the fade, in seconds."))
	float Duration = 0.8f;

	// Part de la durée consacrée à la montée (0,5 = sommet au milieu ; 0,2 = montée rapide, redescente lente)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Highlight", meta = (
		ClampMin = "0.0", ClampMax = "1.0",
		ToolTip = "Part of the duration spent rising to the peak, before easing out (0.5 = peak in the middle)."))
	float PeakRatio = 0.5f;

	// Force de la courbe (1 = linéaire ; plus grand = départs et arrêts plus doux)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Highlight", meta = (
		ClampMin = "1.0",
		ToolTip = "Curve strength (1 = linear, higher = smoother starts and stops)."))
	float EaseExponent = 2.f;

	// Valeur du fondu (0 à 1) à une progression donnée (0 = début, 1 = fin)
	float Evaluate(float Progress) const
	{
		const float Ratio = FMath::Clamp(PeakRatio, 0.f, 1.f);
		const float T = FMath::Clamp(Progress, 0.f, 1.f);
		const float Exponent = FMath::Max(EaseExponent, 1.f);

		if (T < Ratio)
		{
			// Montée : 0 → 1
			return FMath::InterpEaseInOut(0.f, 1.f, T / Ratio, Exponent);
		}

		// Redescente : 1 → 0
		const float FallLength = 1.f - Ratio;
		const float Alpha = FallLength > KINDA_SMALL_NUMBER ? (T - Ratio) / FallLength : 1.f;
		return FMath::InterpEaseInOut(1.f, 0.f, Alpha, Exponent);
	}
};
