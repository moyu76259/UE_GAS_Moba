// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/OverHeadStatsGauge.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Widgets/ValueGauge.h"
#include "GAS/CAttributeSet.h"

void UOverHeadStatsGauge::ConfigureWithASC(UAbilitySystemComponent* AbilitySystemComponent)
{
	
	if(AbilitySystemComponent)
	{
		HealthBar->SetAndBoundTOGameplayAttribute(AbilitySystemComponent,
			UCAttributeSet::GetHealthAttribute(), UCAttributeSet::GetMaxHealthAttribute());

		ManaBar->SetAndBoundTOGameplayAttribute(AbilitySystemComponent,
			UCAttributeSet::GetManaAttribute(), UCAttributeSet::GetMaxManaAttribute());
	}
}
