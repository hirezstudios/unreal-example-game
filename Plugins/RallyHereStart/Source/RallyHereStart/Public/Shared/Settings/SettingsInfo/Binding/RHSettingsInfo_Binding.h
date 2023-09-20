// Copyright 2016-2019 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo.h"
#include "GameFramework/RHPlayerInput.h"
#include "RHSettingsInfo_Binding.generated.h"

USTRUCT(BlueprintType)
struct FRHKeyBindInfo
{
    GENERATED_USTRUCT_BODY()

public:
    FRHKeyBindInfo()
        : Name(FName(""))
        , Scale(0.f)
        , InputType(EInputType::KBM)
        , KeyBindType(EKeyBindType::ActionMapping)
        , IsPermanentBinding(false)
    {}

    FRHKeyBindInfo(FName InName, float InScale, EInputType InInputType, EKeyBindType InKeyBindingType, bool InIsPermanentBinding)
        : Name(InName)
        , Scale(InScale)
        , InputType(InInputType)
        , KeyBindType(InKeyBindingType)
        , IsPermanentBinding(InIsPermanentBinding)
    {}

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Binding")
    FName Name;

    UPROPERTY(EditDefaultsOnly, Category = "Binding", meta = (ClampMin = "-1.0", ClampMax = "1.0", UIMin = "-1.0", UIMax = "1.0"))
    float Scale;

    UPROPERTY(VisibleAnywhere, Category = "Binding")
    EInputType InputType;

    UPROPERTY(EditDefaultsOnly, Category = "Binding")
    EKeyBindType KeyBindType;

    UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Binding")
    bool IsPermanentBinding;
};

/**
 * 
 */
UCLASS(Blueprintable)
class RALLYHERESTART_API URHSettingsInfo_Binding : public URHSettingsInfoBase
{
	GENERATED_BODY()

public:
    URHSettingsInfo_Binding(const FObjectInitializer& ObjectInitializer);

protected:
    virtual void InitializeValue_Implementation() override;

private:
    UFUNCTION()
    void OnKeyBindingsApplied(FName Name);
    UFUNCTION()
    void OnKeyBindingsSaved(FName Name);

private:
    FRHKeyBind GetKeyBind() const;

protected:
    virtual bool ApplyKeyBindValue_Implementation(FRHKeyBind InKeyBind) override;
    // Keybindings are saved as a whole group.
    virtual bool SaveKeyBindValue_Implementation(FRHKeyBind InKeyBind) override { return true; }

private:
	bool ApplyActionKeyBindInfo(const FRHKeyBindInfo& KeyBindInfoToApply, const TArray<FRHInputActionKey>& KeysToApply);
	bool ApplyAxisKeyBindInfo(const FRHKeyBindInfo& KeyBindInfoToApply, const TArray<FKey>& KeysToApply);

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Binding")
    FRHKeyBindInfo PrimaryKeyBindInfo;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Binding")
    FRHKeyBindInfo GamepadKeyBindInfo;

private:
	UFUNCTION()
	void OnSettingsReceivedFromPlayerAccount();
};
