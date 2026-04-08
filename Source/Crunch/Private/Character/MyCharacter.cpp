// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MyCharacter.h"
#include "Crunch/Crunch.h"
#include "Components/SkeletalMeshComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/CAbilitySystemComponent.h"
#include "GAS/CAttributeSet.h"
#include "GAS/CAbilitySystemStatics.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Widgets/OverHeadStatsGauge.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_SpringArm, ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Target, ECR_Ignore);

	CAbilitySystemComponent = CreateDefaultSubobject<UCAbilitySystemComponent>("CAbility System Component");
	CAttributeSet = CreateDefaultSubobject<UCAttributeSet>("CAttribute Set");
	OverHeadWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("Over Head Widget Component");
	OverHeadWidgetComponent->SetupAttachment(GetRootComponent());

    BindGASChangeDelegates();

	PerceptionStimuliSourceComponent = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>("Perception Source Component");
}

void AMyCharacter::ServerSideInit()
{
	
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
	CAbilitySystemComponent->ServerSideInit();
}

void AMyCharacter::ClientSideInit()
{
	CAbilitySystemComponent->InitAbilityActorInfo(this,this);
}

bool AMyCharacter::IsLocallyControlledByPlayer() const
{
	return GetController() && GetController()->IsLocalPlayerController();
}

void AMyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyCharacter, TeamID);
}

const TMap<ECAbilityInputID, TSubclassOf<UGameplayAbility>>& AMyCharacter::GetAbilities() const
{
	return CAbilitySystemComponent->GetAbilities();
}

FVector AMyCharacter::GetCaptureLocalPosition() const
{
	return HeadshotCaptureLocalPosition;
}

FRotator AMyCharacter::GetCaptureLocalRotation() const
{
	return HeadshotCaptureLocalRotation;
}

void AMyCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if(NewController && !NewController->IsPlayerController())
	{
		ServerSideInit();
	}
}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	ConfigureOverHeadStatusWidget();
	MeshRelativeTransform = GetMesh()->GetRelativeTransform();

	PerceptionStimuliSourceComponent->RegisterForSense(UAISense_Sight::StaticClass());
}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

UAbilitySystemComponent* AMyCharacter::GetAbilitySystemComponent() const
{
	return CAbilitySystemComponent;
}

void AMyCharacter::Server_SendGameplayEventToSelf_Implementation(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, EventTag, EventData);
}

bool AMyCharacter::Server_SendGameplayEventToSelf_Validate(const FGameplayTag& EventTag,
	const FGameplayEventData& EventData)
{
	return true;
}

void AMyCharacter::UpgradeAbilityWithInputID(ECAbilityInputID InputID)
{
	if(CAbilitySystemComponent)
	{
		CAbilitySystemComponent->Server_UpgradeAbilityWithID(InputID);
	}
}

void AMyCharacter::BindGASChangeDelegates()
{
	if(CAbilitySystemComponent)
	{
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetDeadStatTag()).AddUObject(this, &AMyCharacter::DeathTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetStunStatTag()).AddUObject(this, &AMyCharacter::StunTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetAimStatTag()).AddUObject(this, &AMyCharacter::AimTagUpdated);
		CAbilitySystemComponent->RegisterGameplayTagEvent(UCAbilitySystemStatics::GetFocusStatTag()).AddUObject(this, &AMyCharacter::FocusTagUpdated);

		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMoveSpeedAttribute()).AddUObject(this, &AMyCharacter::MoveSpeedUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxHealthAttribute()).AddUObject(this, &AMyCharacter::MaxHealthUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMaxManaAttribute()).AddUObject(this, &AMyCharacter::MaxManaUpdated);
		CAbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UCAttributeSet::GetMoveAccelerationAttribute()).AddUObject(this, &AMyCharacter::MoveSpeedAccelerationUpdated);
	}
}

void AMyCharacter::DeathTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if(NewCount != 0)
	{
		StartDeathSequence();
	}
	else
	{
		Respawn();
	}
}

void AMyCharacter::StunTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	if(IsDead()) return;

	if(NewCount != 0)
	{
		OnStun();
		PlayAnimMontage(StunMontage);
	}
	else
	{
		OnRecoverFromStun();
		StopAnimMontage(StunMontage);
	}
}

void AMyCharacter::AimTagUpdated(const FGameplayTag Tag, int32 NewCount)
{
	SetIsAimming(NewCount != 0);
}

void AMyCharacter::FocusTagUpdated(const FGameplayTag Tag, int32 NewCount)
{ 
	bIsFocusMode = NewCount >0;
}

void AMyCharacter::SetIsAimming(bool bIsAiming)
{
	bUseControllerRotationYaw = bIsAiming;
	GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	OnAimStateChanged(bIsAiming);
}

void AMyCharacter::OnAimStateChanged(bool bIsAimming)
{
	//override in child class
}

void AMyCharacter::MoveSpeedUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void AMyCharacter::MoveSpeedAccelerationUpdated(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxAcceleration = Data.NewValue;
}

void AMyCharacter::MaxHealthUpdated(const FOnAttributeChangeData& Data)
{
	if(IsValid(CAttributeSet))
	{
		CAttributeSet->RescaleHealth();
	}
}

void AMyCharacter::MaxManaUpdated(const FOnAttributeChangeData& Data)
{
	if(IsValid(CAttributeSet))
	{
		CAttributeSet->RescaleMana();
	}
}

void AMyCharacter::ConfigureOverHeadStatusWidget()
{
	if(!OverHeadWidgetComponent)
	{
		return;
	}
	if(IsLocallyControlledByPlayer())
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
		return; 
	}
	UOverHeadStatsGauge* OverHeadStatsGauge = Cast<UOverHeadStatsGauge>(OverHeadWidgetComponent->GetUserWidgetObject());
	if(OverHeadStatsGauge)
	{
		OverHeadStatsGauge->ConfigureWithASC(GetAbilitySystemComponent());
		OverHeadWidgetComponent->SetHiddenInGame(false);
		GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdatetimerHandle);
		GetWorldTimerManager().SetTimer(HeadStatGaugeVisibilityUpdatetimerHandle, this,
		&AMyCharacter::UpdateHeadGaugeVisibility,HeadStatGaugeVisibilityCheckUpdateGap,true);
	}
}

void AMyCharacter::UpdateHeadGaugeVisibility()
{
     APawn* LocalPlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	if(LocalPlayerPawn)
	{
		float DistSquared = FVector::DistSquared(GetActorLocation(),LocalPlayerPawn->GetActorLocation());
		OverHeadWidgetComponent->SetHiddenInGame(DistSquared > HeadStatGaugeVisibilityRangeSquared);
	}
}

void AMyCharacter::SetStatusGaugeEnabled(bool bIsEnabled)
{
	GetWorldTimerManager().ClearTimer(HeadStatGaugeVisibilityUpdatetimerHandle);
	if(bIsEnabled)
	{
		ConfigureOverHeadStatusWidget();
	}
	else
	{
		OverHeadWidgetComponent->SetHiddenInGame(true);
	}
}

void AMyCharacter::OnStun()
{
	
}

void AMyCharacter::OnRecoverFromStun()
{
	
}

bool AMyCharacter::IsDead() const
{
	const UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent();
	if(!IsValid(AbilitySystemComponent))
	{
		return false;
	}

	return AbilitySystemComponent->HasMatchingGameplayTag(UCAbilitySystemStatics::GetDeadStatTag());
}

void AMyCharacter::RespawnImmediately()
{
	if(!HasAuthority())
	{
		return;
	}

	if(UAbilitySystemComponent* AbilitySystemComponent = GetAbilitySystemComponent())
	{
		AbilitySystemComponent->RemoveActiveEffectsWithGrantedTags(FGameplayTagContainer(UCAbilitySystemStatics::GetDeadStatTag()));
	}
}

void AMyCharacter::DeathMontageFinished()
{
	if(IsDead())
		SetRagdollEnabled(true);
}

void AMyCharacter::SetRagdollEnabled(bool bIsEnabled)
{
	if(bIsEnabled)
	{
		GetMesh()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	}
	else
	{
		GetMesh()->SetSimulatePhysics(false);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		GetMesh()->SetRelativeTransform(MeshRelativeTransform);
	}
}

void AMyCharacter::PlayDeathAnimation()
{
	if(DeathMontage)
	{
		float MontageDuration =  PlayAnimMontage(DeathMontage);
		GetWorldTimerManager().SetTimer(DeathMontageTimerHandle, this, &AMyCharacter::DeathMontageFinished, MontageDuration + DeathMontageFinishTimeShift);
	}
}

void AMyCharacter::StartDeathSequence()
{
	OnDead();

	if(CAbilitySystemComponent)
	{
		CAbilitySystemComponent->CancelAllAbilities();
	}
	PlayDeathAnimation();
	SetStatusGaugeEnabled(false);
	
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_None);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetAIPerceptionStimuliSourceEnabled(false);
}

void AMyCharacter::Respawn()
{
	OnRespawn();
	SetAIPerceptionStimuliSourceEnabled(true);
	SetRagdollEnabled(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
	GetMesh()->GetAnimInstance()->StopAllMontages(0.f);
	SetStatusGaugeEnabled(true);

    if(HasAuthority() && GetController())
    {
	    TWeakObjectPtr<AActor> StartSpot = GetController()->StartSpot;
    	if(StartSpot.IsValid())
    	{
    		SetActorTransform(StartSpot->GetActorTransform());
    	}
    }
	
	
	if(CAbilitySystemComponent)
	{
		CAbilitySystemComponent->ApplyFullStatEffect();
	}
}

void AMyCharacter::OnDead()
{
	
}

void AMyCharacter::OnRespawn()
{
	
}

void AMyCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID)
{
	TeamID = NewTeamID;
}

FGenericTeamId AMyCharacter::GetGenericTeamId() const
{
	return TeamID;
}

void AMyCharacter::OnRep_TeamID()
{
	//override in child class
}

void AMyCharacter::SetAIPerceptionStimuliSourceEnabled(bool bIsEnabled)
{
	if(!PerceptionStimuliSourceComponent)
	{
		return;
	}

	if(bIsEnabled)
	{
		PerceptionStimuliSourceComponent->RegisterWithPerceptionSystem();
	}
	else
	{
		PerceptionStimuliSourceComponent->UnregisterFromPerceptionSystem();
	}
}

