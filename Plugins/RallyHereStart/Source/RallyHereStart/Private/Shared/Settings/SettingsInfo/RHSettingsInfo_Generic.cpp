// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "Shared/Settings/SettingsInfo/RHSettingsInfo_Generic.h"
#include "Shared/HUD/RHHUDCommon.h"
#include "DataFactories/RHSettingsDataFactory.h"

void URHSettingsInfo_Generic::InitializeValue_Implementation()
{
	Super::InitializeValue_Implementation();

	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		FSettingDelegateStruct SettingDelegateStruct;
		SettingDelegateStruct.SettingApplied.BindUFunction(this, FName("OnSettingApplied"));
		SettingDelegateStruct.SettingSaved.BindUFunction(this, FName("OnSettingSaved"));
		pRHSettingsDataFactory->BindSettingCallbacks(Name, SettingDelegateStruct);

		// It's possible that the setting is loaded without notify (console GameSaveData).
		// After the settings are received from the account, go ahead and update our state to match.
		pRHSettingsDataFactory->OnSettingsReceivedFromPlayerAccount.AddUniqueDynamic(this, &URHSettingsInfo_Generic::OnSettingSaved);

		switch (RHSettingType)
		{
		case ERHSettingType::Bool:
			bool OutDesiredBool;
			if (pRHSettingsDataFactory->GetSettingAsBool(Name, OutDesiredBool))
			{
				OnValueBoolSaved(OutDesiredBool);
			}
			break;
		case ERHSettingType::Int:
			int32 OutDesiredInt;
			if (pRHSettingsDataFactory->GetSettingAsInt(Name, OutDesiredInt))
			{
				OnValueIntSaved(OutDesiredInt);
			}
			break;
		case ERHSettingType::Float:
			float OutDesiredFloat;
			if (pRHSettingsDataFactory->GetSettingAsFloat(Name, OutDesiredFloat))
			{
				OnValueFloatSaved(OutDesiredFloat);
			}
			break;
		}
	}
}

void URHSettingsInfo_Generic::RevertSettingToDefault_Implementation()
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		pRHSettingsDataFactory->RevertSettingToDefault(Name);
	}
}

bool URHSettingsInfo_Generic::ApplyBoolValue_Implementation(bool InBool)
{
	if (RHSettingType == ERHSettingType::Bool)
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
		{
			pRHSettingsDataFactory->ApplySettingAsBool(Name, InBool);
			return true;
		}
	}
	return false;
}

bool URHSettingsInfo_Generic::ApplyIntValue_Implementation(int32 InInt)
{
	if (RHSettingType == ERHSettingType::Int)
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
		{
			pRHSettingsDataFactory->ApplySettingAsInt(Name, InInt);
			return true;
		}
	}
	return false;
}

bool URHSettingsInfo_Generic::ApplyFloatValue_Implementation(float InFloat)
{
	if (RHSettingType == ERHSettingType::Float)
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
		{
			pRHSettingsDataFactory->ApplySettingAsFloat(Name, InFloat);
			return true;
		}
	}
	return false;
}

bool URHSettingsInfo_Generic::SaveBoolValue_Implementation(bool InBool)
{
	if (RHSettingType == ERHSettingType::Bool)
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
		{
			pRHSettingsDataFactory->SaveSettingAsBool(Name);
			return true;
		}
	}
	return false;
}

bool URHSettingsInfo_Generic::SaveIntValue_Implementation(int32 InInt)
{
	if (RHSettingType == ERHSettingType::Int)
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
		{
			pRHSettingsDataFactory->SaveSettingAsInt(Name);
			return true;
		}
	}
	return false;
}

bool URHSettingsInfo_Generic::SaveFloatValue_Implementation(float InFloat)
{
	if (RHSettingType == ERHSettingType::Float)
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
		{
			pRHSettingsDataFactory->SaveSettingAsFloat(Name);
			return true;
		}
	}
	return false;
}

void URHSettingsInfo_Generic::OnSettingApplied()
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		switch (RHSettingType)
		{
		case ERHSettingType::Bool:
			bool OutDesiredBool;
			if (pRHSettingsDataFactory->GetSettingAsBool(Name, OutDesiredBool))
			{
				OnValueApplied(FSettingsUnion(OutDesiredBool));
			}
			break;
		case ERHSettingType::Int:
			int32 OutDesiredInt;
			if (pRHSettingsDataFactory->GetSettingAsInt(Name, OutDesiredInt))
			{
				OnValueApplied(FSettingsUnion(OutDesiredInt));
			}
			break;
		case ERHSettingType::Float:
			float OutDesiredFloat;
			if (pRHSettingsDataFactory->GetSettingAsFloat(Name, OutDesiredFloat))
			{
				OnValueApplied(FSettingsUnion(OutDesiredFloat));
			}
			break;
		}
	}
}

void URHSettingsInfo_Generic::OnSettingSaved()
{
	if (URHSettingsDataFactory* const pRHSettingsDataFactory = GetRHSettingsDataFactory())
	{
		switch (RHSettingType)
		{
		case ERHSettingType::Bool:
			bool OutDesiredBool;
			if (pRHSettingsDataFactory->GetSettingAsBool(Name, OutDesiredBool))
			{
				OnValueSaved(FSettingsUnion(OutDesiredBool));
			}
			break;
		case ERHSettingType::Int:
			int32 OutDesiredInt;
			if (pRHSettingsDataFactory->GetSettingAsInt(Name, OutDesiredInt))
			{
				OnValueSaved(FSettingsUnion(OutDesiredInt));
			}
			break;
		case ERHSettingType::Float:
			float OutDesiredFloat;
			if (pRHSettingsDataFactory->GetSettingAsFloat(Name, OutDesiredFloat))
			{
				OnValueSaved(FSettingsUnion(OutDesiredFloat));
			}
			break;
		}
	}
}

URHSettingsDataFactory* URHSettingsInfo_Generic::GetRHSettingsDataFactory() const
{
	if (class ARHHUDCommon* const pRHHUD = GetRHHUD())
	{
		if (URHSettingsDataFactory* const pRHSettingsDataFactory = pRHHUD->GetSettingsDataFactory())
		{
			return pRHSettingsDataFactory;
		}
	}
	return nullptr;
}
