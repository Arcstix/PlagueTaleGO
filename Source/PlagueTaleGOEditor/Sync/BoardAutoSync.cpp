#include "BoardAutoSync.h"
#include "BoardLayoutSync.h"
#include "PlagueTaleGO/Board/Board.h"
#include "PlagueTaleGO/Level/Definitions/BoardNodeType.h"
#include "Editor.h"                  // GEditor
#include "EngineUtils.h"             // TActorIterator
#include "UObject/UObjectGlobals.h"  // FCoreUObjectDelegates
#include "UObject/UnrealType.h"      // FPropertyChangedEvent

FDelegateHandle FBoardAutoSync::Handle;

void FBoardAutoSync::Register()
{
	Handle = FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&FBoardAutoSync::HandleObjectPropertyChanged);
}

void FBoardAutoSync::Unregister()
{
	FCoreUObjectDelegates::OnObjectPropertyChanged.Remove(Handle);
	Handle.Reset();
}

void FBoardAutoSync::HandleObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	// Interactive = valeur en cours de modification (glissement d'un curseur) : on attend la valeur finale
	if (!Object || !GEditor || Event.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}

	// Jamais pendant une partie (PIE) : le niveau édité n'est pas celui qui tourne
	if (GEditor->PlayWorld)
	{
		return;
	}

	// MemberProperty = la propriété de premier niveau de l'objet (ex. Layout, même si on a modifié une case à l'intérieur)
	const FName MemberName = Event.GetMemberPropertyName();

	// Le NodeClass d'un type de case a changé : les cases de ce type doivent être recréées
	if (Cast<UBoardNodeType>(Object) && MemberName == GET_MEMBER_NAME_CHECKED(UBoardNodeType, NodeClass))
	{
		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (!World)
		{
			return;
		}
		for (TActorIterator<ABoard> It(World); It; ++It)
		{
			FBoardLayoutSync::SyncNodeActors(**It);
		}
	}
}
