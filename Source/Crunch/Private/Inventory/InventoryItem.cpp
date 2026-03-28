// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItem.h"
#include "AbilitySystemComponent.h"
#include "Inventory/PA_ShopItem.h"
#include "GameplayEffect.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/CAbilitySystemStatics.h"
#include "GAS/CAttributeSet.h"


FInventoryItemHandle::FInventoryItemHandle(uint32 Id)
	:HandleId {Id}
{
}

FInventoryItemHandle::FInventoryItemHandle()
{
	
}

FInventoryItemHandle FInventoryItemHandle::InvalidHandle()
{
	static FInventoryItemHandle InvalidHandle = FInventoryItemHandle();
	return InvalidHandle;
}

FInventoryItemHandle FInventoryItemHandle::CreateHandle()
{
	return FInventoryItemHandle(GenerateNextId());
}

bool FInventoryItemHandle::IsValid() const
{
	return HandleId != GetInvalidId();
}

uint32 FInventoryItemHandle::GenerateNextId()
{
	static uint32 StaticId = 1;
	return StaticId++;
}

uint32 FInventoryItemHandle::GetInvalidId()
{
	return 0;
}

bool operator==(const FInventoryItemHandle& Lhs, const FInventoryItemHandle& Rhs)
{
	return Lhs.GetHandleId() == Rhs.GetHandleId();
}

uint32 GetTypeHash(const FInventoryItemHandle& Key)
{
	return Key.GetHandleId();
}

bool UInventoryItem::AddStackCount()
{
	if(IsStackFull())
	{
		return false;
	}

	++StackCount;
	return true;
}

bool UInventoryItem::ReduceStackCount()
{
	--StackCount;
	if(StackCount <= 0)
	{
		return false;
	}
	return true;
}

bool UInventoryItem::SetStackCount(int NewStackCount)
{
	if(NewStackCount <= GetShopItem()->GetMaxStackCount())
	{
		StackCount = NewStackCount;
		return true;
	}
	return false;
}

UInventoryItem::UInventoryItem()
	:StackCount{1}
{
	
}

bool UInventoryItem::IsValid() const
{
	return ShopItem != nullptr;
}

void UInventoryItem::InitItem(const FInventoryItemHandle& NewHandle, const UPA_ShopItem* NewShopItem,
	UAbilitySystemComponent* AbilitySystemComponent)
{
	Handle = NewHandle;
	ShopItem = NewShopItem;

	OwnerAbilitySystemComponent = AbilitySystemComponent;
	if(OwnerAbilitySystemComponent)
		OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).AddUObject(this, &UInventoryItem::ManaUpdated);
	ApplyGASModifications();
}

bool UInventoryItem::TryActivateGrantedAbility()
{
	if(!GrantedAbilitySpecHandle.IsValid())
		return false;

	if(OwnerAbilitySystemComponent && OwnerAbilitySystemComponent->TryActivateAbility(GrantedAbilitySpecHandle))
		return true;

	return false;
}

void UInventoryItem::ApplyConsumeEffect()
{
	if(!ShopItem)
		return;

	TSubclassOf<UGameplayEffect> ConsumeEffect = ShopItem->GetConsumeEffect();
	if(!ConsumeEffect)
		return;

	 OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(ConsumeEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
}

void UInventoryItem::RemoveGASModifications()
{
	if(!OwnerAbilitySystemComponent)
		return;

	OwnerAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetManaAttribute()).RemoveAll(this);
	if(OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
	{
		if(AppliedEquipedEffectHandle.IsValid())
			OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(AppliedEquipedEffectHandle);

		if(GrantedAbilitySpecHandle.IsValid())
			OwnerAbilitySystemComponent->SetRemoveAbilityOnEnd(GrantedAbilitySpecHandle);
	}
}

void UInventoryItem::ApplyGASModifications()
{
	if(!GetShopItem() || !OwnerAbilitySystemComponent)
		return;

	if(!OwnerAbilitySystemComponent->GetOwner() || !OwnerAbilitySystemComponent->GetOwner()->HasAuthority())
		return;

	TSubclassOf<UGameplayEffect> EquipEffect = GetShopItem()->GetEquippedEffect();
	if(EquipEffect)
	{
		AppliedEquipedEffectHandle = OwnerAbilitySystemComponent->BP_ApplyGameplayEffectToSelf(EquipEffect, 1, OwnerAbilitySystemComponent->MakeEffectContext());
	}

	TSubclassOf<UGameplayAbility> GrantedAbility = GetShopItem()->GetGrantedAbility();
	if(GrantedAbility)
	{
		GrantedAbilitySpecHandle = OwnerAbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(GrantedAbility));
		
	}
}

void UInventoryItem::ManaUpdated(const FOnAttributeChangeData& ChangeData)
{
	OnAbilityCanCastUpdated.Broadcast(CanCastAbility());
}

void UInventoryItem::SetSlot(int NewSlot)
{
	Slot = NewSlot;
}

float UInventoryItem::GetAbilityCooldownTimeRemaining() const
{
	if(!ISGrantingAnyAbility())
		return 0.f;

	return UCAbilitySystemStatics::GetCooldownRemainingFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent);
}

float UInventoryItem::GetAbilityCooldownDuration() const
{
	if(!ISGrantingAnyAbility())
		return 0.f;

	return UCAbilitySystemStatics::GetCooldownDurationFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent, 1);
}

float UInventoryItem::GetAbilityManaCost() const
{
	if(!ISGrantingAnyAbility())
		return 0.f;

	return UCAbilitySystemStatics::GetManaCostFor(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent, 1);
}

bool UInventoryItem::CanCastAbility() const
{
	if(!ISGrantingAnyAbility() || !OwnerAbilitySystemComponent)
		return false;

	FGameplayAbilitySpec* Spec = OwnerAbilitySystemComponent->FindAbilitySpecFromHandle(GrantedAbilitySpecHandle);
	if(Spec)
	{
		return UCAbilitySystemStatics::CheckAbilityCost(*Spec, *OwnerAbilitySystemComponent);
	}

	return UCAbilitySystemStatics::CheckAbilityCostStatic(GetShopItem()->GetGrantedAbilityCDO(), *OwnerAbilitySystemComponent);
}

bool UInventoryItem::IsStackFull() const
{
	return StackCount >= GetShopItem()->GetMaxStackCount();
}

bool UInventoryItem::IsForItem(const UPA_ShopItem* Item) const
{
	if(!Item)
		return false;

	return GetShopItem() == Item;
}

bool UInventoryItem::IsGrantintAbility(TSubclassOf<UGameplayAbility> AbilityClass) const
{
	if(!ShopItem)
		return false;

	TSubclassOf<UGameplayAbility> GrantedAbility = ShopItem->GetGrantedAbility();
	return GrantedAbility == AbilityClass;
}

bool UInventoryItem::ISGrantingAnyAbility() const
{
	if(!ShopItem)
		return false;

	return ShopItem->GetGrantedAbility() != nullptr;
}

