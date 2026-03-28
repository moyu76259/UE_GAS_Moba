// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MyCharacter.h"
#include "Minion.generated.h"

/**
 * 
 */
UCLASS()
class AMinion : public AMyCharacter
{
	GENERATED_BODY()
public:
	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;

	bool IsActive() const;
	void Activate();
	void SetGoal(AActor* Goal);
private:
	void PickSkinBasedOnTeamID();

	virtual void OnRep_TeamID() override;
	
	UPROPERTY(EditAnywhere, Category="Visual")
	TMap<FGenericTeamId, USkeletalMesh*> SkinMap;

	UPROPERTY(EditAnywhere, Category="AI")
	FName GoalBlackboardKeyName = "Goal";
};
