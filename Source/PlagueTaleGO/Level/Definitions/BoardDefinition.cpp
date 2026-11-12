#include "BoardDefinition.h"

FText UBoardDefinition::GetDisplayText() const
{
	return DisplayName.IsEmpty() ? FText::FromString(GetName()) : DisplayName;
}

UBoardPlacementSettings* UBoardDefinition::CreateSettings(UObject* Outer) const
{
	if (!Outer || !SettingsClass || SettingsClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return nullptr;
	}
	return NewObject<UBoardPlacementSettings>(Outer, SettingsClass);
}
