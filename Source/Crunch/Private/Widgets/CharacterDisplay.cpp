// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/CharacterDisplay.h"
#include "Camera/CameraComponent.h"
#include "Character/PA_CharacterDefination.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ACharacterDisplay::ACharacterDisplay()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>("Root Comp"));

	MeshCompoennt = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh Component");
	MeshCompoennt->SetupAttachment(GetRootComponent());

	ViewCameraComponent = CreateDefaultSubobject<UCameraComponent>("View Camera Component");
	ViewCameraComponent->SetupAttachment(GetRootComponent());
}

void ACharacterDisplay::ConfigureWithCharacterDefination(const UPA_CharacterDefination* CharacterDefination)
{
	if(!CharacterDefination)
		return;

	MeshCompoennt->SetSkeletalMesh(CharacterDefination->LoadDisplayMesh());
	MeshCompoennt->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	MeshCompoennt->SetAnimClass(CharacterDefination->LoadDisplayAnimationBP());
}


