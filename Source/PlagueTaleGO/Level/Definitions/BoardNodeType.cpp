#include "BoardNodeType.h"
#include "PlagueTaleGO/Board/BoardNode.h"
#include "UObject/UObjectIterator.h"   // TObjectIterator

UBoardNodeType::UBoardNodeType()
{
	NodeClass = ABoardNode::StaticClass();
}

#if WITH_EDITOR
void UBoardNodeType::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// TObjectIterator parcourt tous les objets de cette classe chargés en mémoire (tous les niveaux ouverts).
	// Le visuel est une donnée recalculée : pas besoin de Modify(), le niveau n'est pas marqué comme modifié.
	// Changer NodeClass, en revanche, demande de recréer les acteurs : commande BoardEditor.Resync.
	for (TObjectIterator<ABoardNode> It; It; ++It)
	{
		ABoardNode* Node = *It;
		// IsTemplate : ignore les objets par défaut des classes (CDO), qui ne sont pas des cases du niveau
		if (!IsValid(Node) || Node->IsTemplate() || Node->GetNodeType() != this)
		{
			continue;
		}
		Node->RefreshVisuals();
	}
}
#endif
