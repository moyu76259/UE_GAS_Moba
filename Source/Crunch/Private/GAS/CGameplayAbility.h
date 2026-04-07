// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GenericTeamAgentInterface.h"
#include "CGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class UCGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	UCGameplayAbility();

	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
protected:
	AActor* GetAimTarget(float AimDistance, ETeamAttitude::Type TeamAttitude) const;
	class UAnimInstance* GetOwnerAnimInstance() const;

	TArray<FHitResult> GetHitResultsFromSweepLocationTargetData(const FGameplayAbilityTargetDataHandle& TargetDataHandle,
		float SphereSweepRadius = 30.f, ETeamAttitude::Type TargetTeam = ETeamAttitude::Hostile, bool bDrawDebug = false, bool bIgnoreSelf = true) const;

	UFUNCTION()
	FORCEINLINE bool ShouldDrawDebug() const { return bShouldDrawDebug; }

	// 推动自己（如击退/击飞）
	void PushSelf(const FVector& PushVel);
	// 推动目标
	void PushTarget(AActor* Target, const FVector& PushVel);
	// 推动多个目标
	void PushTargets(const TArray<AActor*>& Targets, const FVector& PushVel);
	// 推动TargetData中的所有目标
	void PushTargets(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& PushVel);

	// 从拥有者位置推动目标
	void PushTargetsFromOwnerLocation(const TArray<AActor*>& Targets, float PushSpeed);
	void PushTargetsFromOwnerLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, float PushSpeed);

	// 从指定位置推动目标
	void PushTargetsFromLocation(const FGameplayAbilityTargetDataHandle& TargetDataHandle, const FVector& FromLocation, float PushSpeed);
	void PushTargetsFromLocation(const TArray<AActor*>& Targets, const FVector& FromLocation, float PushSpeed);
	void PlayMontageLocally(UAnimMontage* MontageToPlay);
	void StopMontageAfterCurrentSection(UAnimMontage* MontageToStop);
	FGenericTeamId GetOwnerTeamId() const;

	bool IsActorTeamAttitudeIs(const AActor* OtherActor, ETeamAttitude::Type TeamAttitude) const;

	ACharacter* GetOwningAvatarCharacter();

	void ApplyGameplayEffectToHitResultActor(const FHitResult& HitResult, TSubclassOf<UGameplayEffect> GameplayEffect, int Level = 1);
	void SendLocalGameplayEvent(const FGameplayTag& EventTag, const FGameplayEventData& EventData);
private:
	UPROPERTY(EditAnywhere, Category="Debug")
	bool bShouldDrawDebug = false;

	UPROPERTY()
	class ACharacter* AvatarCharacter;
};
