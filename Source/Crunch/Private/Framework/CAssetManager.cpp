// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/CAssetManager.h"
#include "Character/PA_CharacterDefination.h"

UCAssetManager& UCAssetManager::Get()
{
	UCAssetManager* Singleton = Cast<UCAssetManager>(GEngine->AssetManager.Get());
	if(Singleton)
	{
		return *Singleton;
	}

	UE_LOG(LogLoad, Fatal, TEXT("Asset Manager Needs to be of the type CAssetManager"));
	return (*NewObject<UCAssetManager>());
}

void UCAssetManager::LoadCharacterDefinations(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_CharacterDefination::GetCharacterDefinationAssetType(), TArray<FName>(), LoadFinishedCallback);
}

bool UCAssetManager::GetLoadedCharacterDefinations(TArray<UPA_CharacterDefination*>& LoadedCharacterDefinations) const
{
	TArray<UObject*> LoadedObjects;
	bool bLoaded = GetPrimaryAssetObjectList(UPA_CharacterDefination::GetCharacterDefinationAssetType(), LoadedObjects);
	if(bLoaded)
	{
		for(UObject* LoadedObject : LoadedObjects)
		{
			LoadedCharacterDefinations.Add(Cast<UPA_CharacterDefination>(LoadedObject));
		}
	}

	return bLoaded;
}

void UCAssetManager::LoadShopItems(const FStreamableDelegate& LoadFinishedCallback)
{
	LoadPrimaryAssetsWithType(UPA_ShopItem::GetShopItemAssetType(), TArray<FName>(), FStreamableDelegate::CreateUObject(this, &UCAssetManager::ShopItemLoadFinished, LoadFinishedCallback));
}

bool UCAssetManager::GetLoadedShopItems(TArray<const UPA_ShopItem*>& OutItems) const
{
	if(LoadedShopItems.Num() > 0)
	{
		for(UPA_ShopItem* LoadedItem : LoadedShopItems)
		{
			if(LoadedItem)
			{
				OutItems.Add(LoadedItem);
			}
		}
		return OutItems.Num() > 0;
	}

	TArray<UObject*> LoadedObjects;
	bool bLoaded = GetPrimaryAssetObjectList(UPA_ShopItem::GetShopItemAssetType(), LoadedObjects);

	if(bLoaded)
	{
		for(UObject* ObjectLoaded : LoadedObjects)
		{
			if(UPA_ShopItem* LoadedItem = Cast<UPA_ShopItem>(ObjectLoaded))
			{
				OutItems.Add(LoadedItem);
			}
		}
	}
	return bLoaded;
}

const FItemCollection* UCAssetManager::GetCombinationForItem(const UPA_ShopItem* Item) const
{
	return CombinationMap.Find(Item);
}

const FItemCollection* UCAssetManager::GetIngredientForItem(const UPA_ShopItem* Item) const
{
	return IngredientMap.Find(Item);
}

void UCAssetManager::ShopItemLoadFinished(const FStreamableDelegate Callback)
{
	LoadedShopItems.Reset();

	TArray<const UPA_ShopItem*> ShopItems;
	if(GetLoadedShopItems(ShopItems))
	{
		LoadedShopItems.Reserve(ShopItems.Num());
		for(const UPA_ShopItem* ShopItem : ShopItems)
		{
			LoadedShopItems.Add(const_cast<UPA_ShopItem*>(ShopItem));
		}
	}

	Callback.ExecuteIfBound();
	BuildItemMaps();
}

void UCAssetManager::BuildItemMaps()
{
	TArray<const UPA_ShopItem*> LoadedItems;
	if(GetLoadedShopItems(LoadedItems))
	{
		for(const UPA_ShopItem* Item : LoadedItems)
		{
			if(Item->GetIngredients().Num() == 0)
			{
					continue;
			}
			TArray<const UPA_ShopItem*> Items;
			for(const TSoftObjectPtr<UPA_ShopItem>& Ingredient : Item->GetIngredients())
			{
				UPA_ShopItem* IngredientItem = Ingredient.LoadSynchronous();
				Items.Add(IngredientItem);
				AddToCombinationMap(IngredientItem, Item);
			}
			IngredientMap.Add(Item, FItemCollection{Items});
		}
	}
}

void UCAssetManager::AddToCombinationMap(const UPA_ShopItem* Ingredient, const UPA_ShopItem* CombinationItem)
{
	FItemCollection* Combinations = CombinationMap.Find(Ingredient);
	if(Combinations)
	{
		if(!Combinations->Contains(CombinationItem))
			Combinations->AddItem(CombinationItem);
	}
	else
	{
		CombinationMap.Add(Ingredient, FItemCollection{TArray<const UPA_ShopItem*>{CombinationItem}});
	}
}
