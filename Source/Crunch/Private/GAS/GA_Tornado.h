// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GAS/CGameplayAbility.h"
#include "GA_Tornado.generated.h"


/**
 * 
 */
UCLASS()
class UGA_Tornado : public UCGameplayAbility
{
	GENERATED_BODY()
public:	
	// 激活技能
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;	
	
private:
	// 命中时应用的伤害效果
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	TSubclassOf<UGameplayEffect> HitDamageEffect;

	// 命中时的击退速度
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float HitPushSpeed = 3000.f;

	// 龙卷风技能动画
	UPROPERTY(EditDefaultsOnly, Category = "Anim")
	TObjectPtr<UAnimMontage> TornadoMontage;

	// 龙卷风持续时长（秒）
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float TornadoDuration = 4.f;

	// 伤害事件回调（当动画事件触发时调用）
	UFUNCTION()
	void TornadoDamageEventReceived(FGameplayEventData Payload);
};
