#include "BoardAudioLibrary.h"

const FBoardSoundEntry* UBoardAudioLibrary::FindEntry(FGameplayTag Event) const
{
	const FBoardSoundEntry* Entry = Sounds.Find(Event);
	return (Entry && Entry->HasSound()) ? Entry : nullptr;
}
