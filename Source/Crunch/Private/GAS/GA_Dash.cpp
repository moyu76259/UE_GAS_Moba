// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/GA_Dash.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/TargetActor_Around.h"
#include "AbilitySystemComponent.h"
#include "GAS/CGameplayAbility.h"
#include "GameFramework/Controller.h"
#include "GameFramework/Pawn.h"





void UGA_Dash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if(!K2_CommitAbility() || !DashMontage)
	{
		K2_EndAbility();
		return;
	}

	if(HasAuthorityOrPredictionKey(ActorInfo, &ActivationInfo))
	{
		// 创建并播放冲刺动画蒙太奇
		UAbilityTask_PlayMontageAndWait* PlayDashMontage = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this,NAME_None, DashMontage);
		// 绑定动画结束/中断事件到能力结束
		PlayDashMontage->OnBlendOut.AddDynamic(this, &UGA_Dash::K2_EndAbility);
		PlayDashMontage->OnCancelled.AddDynamic(this, &UGA_Dash::K2_EndAbility);
		PlayDashMontage->OnInterrupted.AddDynamic(this, &UGA_Dash::K2_EndAbility);
		PlayDashMontage->OnCompleted.AddDynamic(this, &UGA_Dash::K2_EndAbility);
		PlayDashMontage->ReadyForActivation();

		// 等待动画中的冲刺开始事件
		UAbilityTask_WaitGameplayEvent* WaitDashStartEvent = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, GetDashStartTag());
		WaitDashStartEvent->EventReceived.AddDynamic(this, &UGA_Dash::StartDash);
		WaitDashStartEvent->ReadyForActivation();
	}
}

void UGA_Dash::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 获取能力系统组件
	UAbilitySystemComponent* OwnerAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	// 移除冲刺效果
	if (OwnerAbilitySystemComponent && DashEffectHandle.IsValid())
	{
		OwnerAbilitySystemComponent->RemoveActiveGameplayEffect(DashEffectHandle);
	}

	if (OwnerController && bRestoreMoveInput)
	{
		OwnerController->SetIgnoreMoveInput(false);
	}

	OwnerController = nullptr;
	bRestoreMoveInput = false;
	OwnerCharacterMovementComponent = nullptr;
	DashEffectHandle = FActiveGameplayEffectHandle();
	
	// 清除推进定时器
	if (PushForwardInputTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(PushForwardInputTimerHandle);
	}
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

bool UGA_Dash::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags,
	FGameplayTagContainer* OptionalRelevantTags) const
{
	
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return false;
	}

	const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect();
	if (!CooldownEffect)
	{
		return true;
	}

	FGameplayEffectQuery CooldownQuery;
	CooldownQuery.EffectDefinition = CooldownEffect->GetClass();

	for (const float RemainingTime : ActorInfo->AbilitySystemComponent->GetActiveEffectsTimeRemaining(CooldownQuery))
	{
		if (RemainingTime > 0.f)
		{
			return false;
		}
	}

	return true;
}

FGameplayTag UGA_Dash::GetDashStartTag()
{
	return FGameplayTag::RequestGameplayTag("ability.dash.start");
}

void UGA_Dash::PushForward()
{
	// 如果存在移动组件，则沿角色前方持续推动
	if (OwnerCharacterMovementComponent)
	{
		// 获取角色前方向量
		FVector ForwardActor = GetAvatarActorFromActorInfo()->GetActorForwardVector();
		// 添加移动输入
		OwnerCharacterMovementComponent->AddInputVector(ForwardActor, true);
		// 设置下一帧继续推动(循环递归调用)
		PushForwardInputTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGA_Dash::PushForward);
	}
}

void UGA_Dash::StartDash(FGameplayEventData Payload)
{
	// 在服务端应用冲刺效果
	if (K2_HasAuthority())
	{
		if (DashEffect)
		{
			DashEffectHandle = BP_ApplyGameplayEffectToOwner(DashEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		}
	}

	// 本地控制角色：启动连续推进
	if (IsLocallyControlled())
	{
		OwnerCharacterMovementComponent = GetAvatarActorFromActorInfo()->GetComponentByClass<UCharacterMovementComponent>();

		if (APawn* OwnerPawn = Cast<APawn>(GetAvatarActorFromActorInfo()))
		{
			if (AController* Controller = OwnerPawn->GetController())
			{
				OwnerController = Controller;
				bRestoreMoveInput = !Controller->IsMoveInputIgnored();
				Controller->SetIgnoreMoveInput(true);
			}
		}

		PushForwardInputTimerHandle = GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UGA_Dash::PushForward);
	}

	// 创建目标检测任务
	UAbilityTask_WaitTargetData* WaitTargetData = UAbilityTask_WaitTargetData::WaitTargetData(
		this, 
		NAME_None, 
		EGameplayTargetingConfirmation::CustomMulti,  // 自定义确认方式
		TargetActorClass
	);
	
	// 绑定目标检测完成回调
	WaitTargetData->ValidData.AddDynamic(this, &UGA_Dash::TargetReceived);
	WaitTargetData->ReadyForActivation();

	// 生成目标检测器
	AGameplayAbilityTargetActor* TargetActor;
	WaitTargetData->BeginSpawningActor(this, TargetActorClass, TargetActor);

	// 配置目标检测器
	ATargetActor_Around* TargetActorAround = Cast<ATargetActor_Around>(TargetActor);
	if (TargetActorAround)
	{
		// 设置检测半径、队伍过滤和视觉提示
		TargetActorAround->ConfigureDetection(TargetDetectionRadius, GetOwnerTeamId(), LocalGameplayCueTag);
	}

	// 完成生成
	WaitTargetData->FinishSpawningActor(this, TargetActor);

	// 将检测器附加到角色骨骼
	if (TargetActorAround)
	{
		TargetActorAround->AttachToComponent(
			GetOwningComponentFromActorInfo(), 
			FAttachmentTransformRules::SnapToTargetNotIncludingScale, 
			TargetActorAttachSocketName
		);
	}
}

void UGA_Dash::TargetReceived(const FGameplayAbilityTargetDataHandle& TargetDataHandle)
{
	// 服务端处理：对目标应用效果
	if (K2_HasAuthority())
	{
		// 应用伤害效果
		BP_ApplyGameplayEffectToTarget(TargetDataHandle, DamageEffect, GetAbilityLevel(CurrentSpecHandle, CurrentActorInfo));
		// 击退目标
		PushTargetsFromOwnerLocation(TargetDataHandle, TargetHitPushSpeed);
	}
}