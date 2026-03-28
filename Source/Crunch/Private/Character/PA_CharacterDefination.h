// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "PA_CharacterDefination.generated.h"

class AMyCharacter;
class UGameplayAbility;

/**
 * 
 */
UCLASS()
class UPA_CharacterDefination : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
	static FPrimaryAssetType GetCharacterDefinationAssetType();

	FString GetCharacterDisplayName() const {return CharacterName; }
	UTexture2D* LoadIcon() const;
	TSubclassOf<AMyCharacter> LoadCharacterClass() const;
	TSubclassOf<UAnimInstance> LoadDisplayAnimationBP() const;
	class USkeletalMesh* LoadDisplayMesh( ) const;
	const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>* GetAbilities() const;
private:
	UPROPERTY(EditDefaultsOnly, Category="Character")
	FString CharacterName;

	UPROPERTY(EditDefaultsOnly, Category="Character")
	TSoftObjectPtr<UTexture2D> CharacterIcon;

	UPROPERTY(EditDefaultsOnly, Category="Character")
	TSoftClassPtr<AMyCharacter> CharacterClass;

	UPROPERTY(EditDefaultsOnly, Category="Character")
	TSoftClassPtr<UAnimInstance> DisplayAnimBP;
};
