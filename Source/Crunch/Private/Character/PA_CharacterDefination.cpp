// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/PA_CharacterDefination.h"
#include "Character/MyCharacter.h"

FPrimaryAssetId UPA_CharacterDefination::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(GetCharacterDefinationAssetType(), GetFName());
}

FPrimaryAssetType UPA_CharacterDefination::GetCharacterDefinationAssetType()
{
	return FPrimaryAssetType("CharacterDefination");
}

UTexture2D* UPA_CharacterDefination::LoadIcon() const
{
	CharacterIcon.LoadSynchronous();
	if(CharacterIcon.IsValid())
		return CharacterIcon.Get();

	return nullptr;
}

TSubclassOf<AMyCharacter> UPA_CharacterDefination::LoadCharacterClass() const
{
	CharacterClass.LoadSynchronous();
	if(CharacterClass.IsValid())
		return CharacterClass.Get();

	return TSubclassOf<AMyCharacter>();
	
}

TSubclassOf<UAnimInstance> UPA_CharacterDefination::LoadDisplayAnimationBP() const
{
	DisplayAnimBP.LoadSynchronous();
	if(DisplayAnimBP.IsValid())
		return DisplayAnimBP.Get();

	return TSubclassOf<UAnimInstance>();
}

USkeletalMesh* UPA_CharacterDefination::LoadDisplayMesh() const
{
	TSubclassOf<AMyCharacter> LoadedCharacterClass = LoadCharacterClass();
	if(!LoadedCharacterClass)
		return nullptr;

	ACharacter* Character = Cast<ACharacter>(LoadedCharacterClass.GetDefaultObject());
	if(!Character)
		return nullptr;

	return Character->GetMesh()->GetSkeletalMeshAsset();
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>* UPA_CharacterDefination::GetAbilities() const
{
	TSubclassOf<AMyCharacter> LoadedCharacterClass = LoadCharacterClass();
	if(!LoadedCharacterClass)
		return nullptr;
	
	AMyCharacter* Character = Cast<AMyCharacter>(LoadedCharacterClass.GetDefaultObject());
	if(!Character)
		return nullptr;

	return &(Character->GetAbilities());
}
