// Copyright 2016-2020 Hi-Rez Studios, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Templates/IsEnumClass.h"
#include "Templates/EnableIf.h"
#include "Shared/Social/RHDataSocialPlayer.h"
#include "RH_FriendSubsystem.h"
#include "RHDataSocialCategory.generated.h"

class URHDataSocialCategory;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSocialCategoryPlayersUpdated, URHDataSocialCategory*, Category, const TArray<URHDataSocialPlayer*>&, Players);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSocialCategoryHeaderUpdated, URHDataSocialCategory*, Category, FText, Header);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnSocialCategoryMessageUpdated, URHDataSocialCategory*, Category, bool, Processing, FText, Header);

UENUM(BlueprintType)
enum class ERHCategoryOpenMode : uint8
{
	ClosedByDefault,
	OpenByDefault
};

/**
 * 
 */
UCLASS(BlueprintType)
class RALLYHERESTART_API URHDataSocialCategory : public UObject
{
	GENERATED_BODY()

	template<typename EnumType>
	using RequiresEnumClass = TEnableIf<TIsEnumClass<EnumType>::Value>;

public:
	using PlayerDataContainer_t = URHDataSocialPlayer*;

	URHDataSocialCategory();

	UFUNCTION(BlueprintCallable, Category="RH|Social Category")
	void SetHeaderText(const FText& Header);

	UFUNCTION(BlueprintCallable, Category="RH|Social Category")
	void SetMessageText(bool bProcessing, const FText &MessageText);

	void ForceSetSortedPlayerList(const TArray<PlayerDataContainer_t>& SortedPlayers);
	void ForceSetSortedPlayerList(TArray<PlayerDataContainer_t>&& SortedPlayers); // move

	template<typename EnumType, typename = typename RequiresEnumClass<EnumType>::Type>
	inline void SetSectionValue(EnumType Type) { eType = static_cast<uint8>(Type); }

	inline void SetSectionSubtype(const FString& Subtype) { SectionSubtype = Subtype; }
	
	UFUNCTION(BlueprintPure, Category="RH|Social Category")
	inline FText GetHeaderText() const { return HeaderText; }
	
	UFUNCTION(BlueprintPure, Category="RH|Social Category")
	inline FText GetMessageText() const { return Message; }
	
	UFUNCTION(BlueprintPure, Category="RH|Social Category")
	inline bool IsProcessing() const { return Processing; }

	UFUNCTION(BlueprintPure, Category="RH|Social Category")
	inline TArray<URHDataSocialPlayer*>& GetPlayerList() { return SortedPlayerList; }

	template<typename EnumType, typename = typename RequiresEnumClass<EnumType>::Type>
	inline EnumType GetSectionValue() const { return static_cast<EnumType>(eType); }

	UFUNCTION(BlueprintPure, Category="RH|Social Category", DisplayName="Get Section Value")
	inline uint8 BP_GetSectionValue() const { return eType; }

	UFUNCTION(BlueprintPure, Category = "RH|Social Category", DisplayName = "Get Section Subtype")
	inline FString GetSectionSubtype() const { return SectionSubtype; }

	void Reserve(int32 Count);

	TArray<URHDataSocialPlayer*> Empty(int32 Slack);

	UFUNCTION(BlueprintPure, Category="RH|Social Category")
	inline int32 Num() const { return SortedPlayerList.Num(); }

	UFUNCTION(BlueprintCallable, Category="RH|Social Category")
	void SetOpenOnUpdate(bool Value) { OpenOnUpdate = true; }

	UFUNCTION(BlueprintCallable, Category="RH|Social Category")
	inline bool TryConsumeOpenOnUpdate()
	{
		bool Temp = OpenOnUpdate;
		OpenOnUpdate = false;
		return Temp;
	}

	inline bool ShouldOpenOnUpdate() const { return OpenOnUpdate; }

	void SetSortMethod(TFunction<bool(URH_RHFriendAndPlatformFriend*, URH_RHFriendAndPlatformFriend*)> SortFunction);

	void Add(URHDataSocialPlayer* Container);
	URHDataSocialPlayer* Remove(URH_RHFriendAndPlatformFriend* Info);
	bool Contains(URH_RHFriendAndPlatformFriend* Info);

	UPROPERTY(BlueprintAssignable, Category="RH|Social Category")
	FOnSocialCategoryPlayersUpdated OnPlayersUpdated;

	UPROPERTY(BlueprintAssignable, Category="RH|Social Category")
	FOnSocialCategoryHeaderUpdated OnHeaderUpdated;

	UPROPERTY(BlueprintAssignable, Category="RH|Social Category")
	FOnSocialCategoryMessageUpdated OnMessageUpdated;

protected:
	static URH_RHFriendAndPlatformFriend* AsFriend(const URHDataSocialPlayer* Container) { return Container->GetFriend();  }

private:
	FText HeaderText;
	FText Message;
	uint8 eType;
	FString SectionSubtype;
	bool Processing;
	bool OpenOnUpdate;
	ERHCategoryOpenMode eOpenMode;

	TFunction<bool(URH_RHFriendAndPlatformFriend*,URH_RHFriendAndPlatformFriend*)> SortMethod;

	UPROPERTY()
	TArray<URHDataSocialPlayer*> SortedPlayerList;
};
