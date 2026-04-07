// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbility.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CAbilitySystemStatics.generated.h"

class UGameplayAbility;
struct FGameplayAbilitySpec;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class UCAbilitySystemStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	static FGameplayTag GetBasicAttackAbilityTag();
	static FGameplayTag GetBasicAttackInputPressedTag();
	static FGameplayTag GetBasicAttackInputReleasedTag();
	static FGameplayTag GetDeadStatTag();
	static FGameplayTag GetStunStatTag();
	static FGameplayTag GetAimStatTag();
	static FGameplayTag GetFocusStatTag();
	static FGameplayTag GetCameraShakeGameplayCueTag();
	static FGameplayTag GetHealthFullStatTag();
	static FGameplayTag GetHealthEmptyStatTag();
	static FGameplayTag GetManaFullStatTag();
	static FGameplayTag GetManaEmptyStatTag();
    static FGameplayTag GetHeroRoleTag();
	static FGameplayTag GetExperienceAttributeTag();
	static FGameplayTag GetGoldAttributeTag();
	static FGameplayTag GetCrosshairTag();
	static FGameplayTag GetTargetUpdatedTag();

	static bool IsActorDead(const AActor* ActorToCheck);
	static bool IsHero(const AActor* ActorToCheck);
	static bool ActorHasTag(const AActor* ActorToCheck, const FGameplayTag& Tag);
	static bool IsAbilityAtMaxLevel(const FGameplayAbilitySpec& Spec);

	static float GetStaticCooldownDurationForAbility(const UGameplayAbility* Ability);
	static float GetStaticCostForAbility(const UGameplayAbility* Ability);

	static bool CheckAbilityCost(const FGameplayAbilitySpec& AbilitySpec, const UAbilitySystemComponent& ASC);
	static bool CheckAbilityCostStatic(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);
	static float GetManaCostFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
	static float GetCooldownDurationFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC, int AbilityLevel);
	static float GetCooldownRemainingFor(const UGameplayAbility* AbilityCDO, const UAbilitySystemComponent& ASC);

	/**
	 * 在本地触发指定的游戏提示效果（如技能特效、攻击反馈等）
	 * 
	 * @param CueTargetActor 触发游戏提示的目标Actor（如角色、技能释放者）
	 * @param HitResult 包含命中位置、法线等碰撞信息的结果对象
	 * @param GameplayCueTag 标识游戏提示类型的GameplayTag（如"Ability.Attack.Basic"）
	 */
	static void SendLocalGameplayCue(AActor* CueTargetActor, const FHitResult& HitResult, const FGameplayTag& GameplayCueTag);
};
