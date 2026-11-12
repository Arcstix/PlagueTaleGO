#pragma once

#include "CoreMinimal.h"
#include "Framework/Application/IInputProcessor.h"
#include "GenericPlatform/ICursor.h"
#include "InputCoreTypes.h"
#include "BoardEditorMode.h"

/**
 * Intercepte Échap AVANT l'éditeur, pendant que le mode « Board Editor » est actif.
 *
 * Pourquoi : dans le viewport, l'éditeur associe Échap à ses propres commandes (« tout désélectionner »…),
 * traitées avant que la touche n'atteigne le mode et son outil. Un « préprocesseur d'entrées » Slate
 * reçoit chaque touche en premier ; il ne la garde (return true) que si l'outil s'en sert
 * (annuler un tracé, désélectionner une case). Sinon Échap garde son rôle habituel.
 * Équivalent Godot : _input() appelé avant _unhandled_input().
 */
class FBoardEditorInputProcessor : public IInputProcessor
{
public:
	explicit FBoardEditorInputProcessor(UBoardEditorMode* InMode)
		: Mode(InMode)
	{
	}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		if (InKeyEvent.GetKey() == EKeys::Escape && Mode.IsValid())
		{
			return Mode->HandleEscape();
		}
		return false;
	}

	virtual const TCHAR* GetDebugName() const override
	{
		return TEXT("BoardEditorInputProcessor");
	}

private:
	TWeakObjectPtr<UBoardEditorMode> Mode;
};
