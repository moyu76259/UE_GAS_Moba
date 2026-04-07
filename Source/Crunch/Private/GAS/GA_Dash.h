// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "GAS/CGameplayAbility.h"
#include "GAS/CGameplayAbilityTypes.h"
#include "GA_Dash.generated.h"

class AController;
class UCharacterMovementComponent;


/**
 * 
 */
UCLASS()
class UGA_Dash : public UCGameplayAbility, public IGenericTeamAgentInterface
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override; 
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
	const FGameplayTagContainer* TargetTags = nullptr, FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;


	static FGameplayTag GetDashStartTag();

private:
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	UAnimMontage* DashMontage;
	
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	float TargetDetectionRadius = 300.f;
	
	UPROPERTY(EditDefaultsOnly, Category="GameplayCue")
	FGameplayTag LocalGameplayCueTag;
	
	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	FName TargetActorAttachSocketName = "TargetDashCenter";

	UPROPERTY(EditDefaultsOnly, Category="Targeting")
	TSubclassOf<class ATargetActor_Around> TargetActorClass;

	// 命中目标后的击退速度
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	float TargetHitPushSpeed = 3000.f;

	// 命中目标时应用的伤害效果
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DamageEffect;

	// 冲刺过程中应用的持续效果
	UPROPERTY(EditDefaultsOnly, Category = "Effects")
	TSubclassOf<UGameplayEffect> DashEffect;
	
	// 当前激活的冲刺效果句柄
	FActiveGameplayEffectHandle DashEffectHandle;

	// 推动角色前进的定时器句柄
	FTimerHandle PushForwardInputTimerHandle;

	// 推动角色沿当前方向前进
	void PushForward();
	
	// 缓存角色移动组件
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> OwnerCharacterMovementComponent;

	UPROPERTY()
	TObjectPtr<AController> OwnerController;

	bool bRestoreMoveInput = false;


	// 动画事件触发时开始冲刺逻辑
	UFUNCTION()
	void StartDash(FGameplayEventData Payload);

	// 目标检测完成回调
	UFUNCTION()
	void TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle);
};
