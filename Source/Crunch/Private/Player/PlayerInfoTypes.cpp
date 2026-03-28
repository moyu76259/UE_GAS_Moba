// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "Network/CNetStatics.h"

FPlayerSelection::FPlayerSelection()
	: Slot{ GetInvalidSlot() }, 
	  PlayerUniqueId{ FUniqueNetIdRepl::Invalid() }, 
	  PlayerNickName{}, 
	  CharacterDefination{ nullptr }
{
}

FPlayerSelection::FPlayerSelection(uint8 InSlot, const APlayerState* InPlayerState)
	: Slot{ InSlot }, 
	  CharacterDefination{ nullptr }
{
	if(InPlayerState)
	{
		PlayerUniqueId = InPlayerState->GetUniqueId();
		PlayerNickName = InPlayerState->GetPlayerName();
	}
}

bool FPlayerSelection::IsForPlayer(const APlayerState* PlayerState) const
{
	if(!PlayerState)
		return false;

// #if WITH_EDITOR
// 	return PlayerState->GetPlayerName() == PlayerNickName;
// #else
// 	return PlayerState->GetUniqueId() == PlayerUniqueId;
// #endif
	if (PlayerUniqueId.IsValid() && PlayerState->GetUniqueId().IsValid())
	{
		return PlayerState->GetUniqueId() == PlayerUniqueId;
	}

	return PlayerState->GetPlayerName() == PlayerNickName;
	
}

bool FPlayerSelection::IsValid() const
{
	// if (!PlayerUniqueId.IsValid())
	// {
	// 	return false;
	// }
	//
	// if (Slot == GetInvalidSlot() || Slot >= (UCNetStatics::GetPlayerCountPerTeam() * 2))
	// {
	// 	return false;
	// }
	//
	// return true;
	if (Slot == GetInvalidSlot() || Slot >= (UCNetStatics::GetPlayerCountPerTeam() * 2))
	{
		return false;
	}

	if (PlayerUniqueId.IsValid())
	{
		return true;
	}
	return !PlayerNickName.IsEmpty();  //////////////////////////////////////////////////////
}

uint8 FPlayerSelection::GetInvalidSlot()
{
	return uint8(255);
}	
