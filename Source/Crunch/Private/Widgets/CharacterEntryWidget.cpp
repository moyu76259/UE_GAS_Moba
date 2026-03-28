// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CharacterEntryWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Character/PA_CharacterDefination.h"

void UCharacterEntryWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	CharacterDefination = Cast<UPA_CharacterDefination>(ListItemObject);
	if(CharacterDefination)
	{
		if (!CharacterIcon)
		{
			UE_LOG(LogTemp, Error, TEXT("CharacterIcon is null in NativeOnListItemObjectSet"));
			return;
		}

		UMaterialInstanceDynamic* DynamicMat = CharacterIcon->GetDynamicMaterial();
		if (!DynamicMat)
		{
			UE_LOG(LogTemp, Error, TEXT("Dynamic material is null in NativeOnListItemObjectSet"));
			return;
		}

		DynamicMat->SetTextureParameterValue(IconTextureMatParamName, CharacterDefination->LoadIcon());
		CharacterNameText->SetText(FText::FromString(CharacterDefination->GetCharacterDisplayName()));
	}
}

void UCharacterEntryWidget::SetSelected(bool bIsSelected)
{
	if (!CharacterIcon)
	{
		UE_LOG(LogTemp, Error, TEXT("CharacterIcon is null in SetSelected"));
		return;
	}

	UMaterialInstanceDynamic* DynamicMat = CharacterIcon->GetDynamicMaterial();
	if (!DynamicMat)
	{
		UE_LOG(LogTemp, Error, TEXT("Dynamic material is null in SetSelected"));
		return;
	}

	DynamicMat->SetScalarParameterValue(SaturationMatParamName, bIsSelected ? 0.f : 1.f);
}
