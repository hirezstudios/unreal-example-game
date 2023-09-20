#pragma once
#include "RHUISoundTheme.generated.h"

USTRUCT()
struct FRHSoundThemeEventMapping
{
    GENERATED_USTRUCT_BODY()

public:
    UPROPERTY(EditDefaultsOnly, Category = "Sound Event")
    FName SoundEventName;

    UPROPERTY(EditDefaultsOnly, Category = "Sound Event")
    class USoundCue* SoundToPlay;

	FRHSoundThemeEventMapping() : SoundEventName(NAME_None), SoundToPlay(nullptr) {}
};

UCLASS(ClassGroup=(UI), Blueprintable, Transient)
class RALLYHERESTART_API URHUISoundTheme : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    void ProcessSoundEvent(FName SoundEventName, APlayerController* SoundOwner) const;

protected:
    const FRHSoundThemeEventMapping* FindSoundEvent(FName SoundEventName) const;

    UPROPERTY(EditDefaultsOnly, Category = "Sound Event")
    TMap<FName, FRHSoundThemeEventMapping> SoundEventBindings;
};