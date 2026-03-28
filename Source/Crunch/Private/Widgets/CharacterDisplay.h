// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/PA_CharacterDefination.h"
#include "GameFramework/Actor.h"
#include "CharacterDisplay.generated.h"


class UPA_CharacterDefination;
UCLASS()
class ACharacterDisplay : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACharacterDisplay();
	void ConfigureWithCharacterDefination(const UPA_CharacterDefination* CharacterDefination);

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Character Display")
	class USkeletalMeshComponent* MeshCompoennt;

	UPROPERTY(VisibleDefaultsOnly, Category = "Character Display")
	class UCameraComponent* ViewCameraComponent;
};
