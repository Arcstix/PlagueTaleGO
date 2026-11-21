#pragma once

#include "CoreMinimal.h"
#include "InteractiveToolChange.h"
#include "PlagueTaleGO/Level/BoardLayout.h"
#include "PlagueTaleGOEditor/Mode/BoardLayoutDraft.h"

/**
 * Une étape de l'historique Ctrl+Z du brouillon : le layout avant et après une modification.
 *
 * Mécanisme UE : dans l'Interactive Tools Framework, un outil n'enregistre pas d'objets avec Modify() ;
 * il émet un « changement » (FToolCommandChange) que l'éditeur rejoue avec Revert (Ctrl+Z) ou Apply (Ctrl+Y).
 * Les layouts sont petits : copier le layout entier est plus simple et plus sûr qu'un changement « fin ».
 */
class FBoardDraftChange : public FToolCommandChange
{
public:
	FBoardDraftChange(const FBoardLayout& InBefore, const FBoardLayout& InAfter, const FString& InDescription)
		: Before(InBefore), After(InAfter), Description(InDescription)
	{
	}

	virtual void Apply(UObject* Object) override
	{
		if (UBoardLayoutDraft* Draft = Cast<UBoardLayoutDraft>(Object))
		{
			Draft->SetLayout(After);
		}
	}

	virtual void Revert(UObject* Object) override
	{
		if (UBoardLayoutDraft* Draft = Cast<UBoardLayoutDraft>(Object))
		{
			Draft->SetLayout(Before);
		}
	}

	// Le brouillon disparaît quand on quitte le mode : ses étapes d'historique deviennent sans objet
	virtual bool HasExpired(UObject* Object) const override
	{
		return !IsValid(Object);
	}

	virtual FString ToString() const override
	{
		return Description;
	}

private:
	FBoardLayout Before;
	FBoardLayout After;
	FString Description;
};
