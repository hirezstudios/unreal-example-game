#include "RallyHereStart.h"
#include "Sound/SoundCue.h"
#include "RHUISoundTheme.h"

URHUISoundTheme::URHUISoundTheme(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{}

void URHUISoundTheme::ProcessSoundEvent(FName SoundEventName, APlayerController* SoundOwner) const
{
    if (!SoundOwner)
    {
        return;
    }

    if (const FRHSoundThemeEventMapping* Event = FindSoundEvent(SoundEventName))
    {
        SoundOwner->ClientPlaySound(Event->SoundToPlay);
    }
    else
    {
        UE_CLOG(true, RallyHereStart, Log, TEXT("%s : ProcessSoundEvent - Unable to find a binding for event: %s"), *GetNameSafe(this), *SoundEventName.ToString());
    }
}

const FRHSoundThemeEventMapping* URHUISoundTheme::FindSoundEvent(FName SoundEventName) const
{
    return SoundEventBindings.Find(SoundEventName);
}