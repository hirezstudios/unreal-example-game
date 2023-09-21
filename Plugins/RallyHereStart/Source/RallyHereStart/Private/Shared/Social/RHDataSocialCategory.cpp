// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#include "RallyHereStart.h"
#include "RH_PlayerInfoSubsystem.h"
#include "Shared/Social/RHDataSocialCategory.h"

#include "RH_FriendSubsystem.h"
#include "Algo/IsSorted.h"

URHDataSocialCategory::URHDataSocialCategory()
	: UObject(), HeaderText(FText::GetEmpty()), SortMethod(), SortedPlayerList()
{
	// default
	SortMethod = [](URH_RHFriendAndPlatformFriend* A, URH_RHFriendAndPlatformFriend* B)
	{
		return A->GetRHPlayerUuid() < B->GetRHPlayerUuid();
	};
}

void URHDataSocialCategory::SetHeaderText(const FText& Header)
{
	if (HeaderText.EqualTo(Header))
	{
		return;
	}

	HeaderText = Header;
	OnHeaderUpdated.Broadcast(this, HeaderText);
}

void URHDataSocialCategory::SetMessageText(bool bProcessing, const FText& MessageText)
{
	if (Message.EqualTo(MessageText) && Processing == bProcessing)
	{
		return;
	}

	Message = MessageText;
	Processing = bProcessing;
	OnMessageUpdated.Broadcast(this, Processing, Message);
}

void URHDataSocialCategory::ForceSetSortedPlayerList(const TArray<PlayerDataContainer_t>& SortedPlayers)
{
	check(Algo::IsSortedBy(
		SortedPlayers,
		[](URHDataSocialPlayer* D) { return D->GetFriend(); }, 
		SortMethod
	));

	SortedPlayerList = SortedPlayers;
	OnPlayersUpdated.Broadcast(this, SortedPlayerList);
}

void URHDataSocialCategory::ForceSetSortedPlayerList(TArray<PlayerDataContainer_t>&& SortedPlayers)
{
	check(Algo::IsSortedBy(
		SortedPlayers,
		[](URHDataSocialPlayer* D) { return D->GetFriend(); },
		SortMethod
	));

	SortedPlayerList = MoveTemp(SortedPlayers);
	OnPlayersUpdated.Broadcast(this, SortedPlayerList);
}

void URHDataSocialCategory::Reserve(int32 Count)
{
	SortedPlayerList.Reserve(Count);
}

TArray<URHDataSocialPlayer*> URHDataSocialCategory::Empty(int32 Slack)
{
	TArray<URHDataSocialPlayer*> Temp;

	Swap(Temp, SortedPlayerList);
	SortedPlayerList.Reserve(Slack);

	return Temp;
}

void URHDataSocialCategory::SetSortMethod(TFunction<bool(URH_RHFriendAndPlatformFriend*,URH_RHFriendAndPlatformFriend*)> SortFunction)
{
	SortMethod = SortFunction;
	if (SortedPlayerList.Num() > 0)
	{
		SortedPlayerList.Sort(SortMethod);
		OnPlayersUpdated.Broadcast(this, GetPlayerList());
	}
}

void URHDataSocialCategory::Add(URHDataSocialPlayer* Container)
{
	const int32 Index = Algo::LowerBoundBy(SortedPlayerList, Container->GetFriend(), &AsFriend, SortMethod);

	if (Index >= SortedPlayerList.Num())
	{
		SortedPlayerList.Emplace(Container);
	}
	else if (SortedPlayerList[Index] != Container)
	{
		SortedPlayerList.EmplaceAt(Index, Container);
	}
}

URHDataSocialPlayer* URHDataSocialCategory::Remove(URH_RHFriendAndPlatformFriend* Friend)
{
	const int32 Index = Algo::LowerBoundBy(SortedPlayerList, Friend, &AsFriend, SortMethod);
	if (SortedPlayerList.IsValidIndex(Index) && SortedPlayerList[Index]->GetFriend() == Friend)
	{
		URHDataSocialPlayer* Entry = SortedPlayerList[Index];
		SortedPlayerList.RemoveAt(Index, 1);
		return Entry;
	}
	return nullptr;
}

bool URHDataSocialCategory::Contains(URH_RHFriendAndPlatformFriend* Friend)
{
	const int32 Index = Algo::LowerBoundBy(SortedPlayerList, Friend, &AsFriend, SortMethod);
	return SortedPlayerList.IsValidIndex(Index) && SortedPlayerList[Index]->GetFriend() == Friend;
}