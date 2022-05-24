// Fill out your copyright notice in the Description page of Project Settings.
#include "AICharacter.h"
#include "Components/CapsuleComponent.h"
#include "AIController.h"
#include "BossFightCharacter.h"
#include "NavigationSystem.h"
#include "Perception/PawnSensingComponent.h"
#include "Math/UnrealMathUtility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
// Sets default values
AAICharacter::AAICharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	AICharacterCompCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("AICharacterCompCapsuleCpp"));
	AICharacterCompCapsule->SetupAttachment(GetRootComponent());
	collision = false;

}

// Called when the game starts or when spawned
void AAICharacter::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(Timer, this, &AAICharacter::NewMovement, 0.01f);
	
	PawnSensing->OnSeePawn.AddDynamic(this, &AAICharacter::SeePawn);
	PawnSensing->OnHearNoise.AddDynamic(this, &AAICharacter::OnHearNoise);
	AICharacterCompCapsule->OnComponentBeginOverlap.AddDynamic(this, &AAICharacter::BeginOverlap);

	FirstSkillCooldown = 0;
	SecondSkillCooldown = 0;
	ThirdSkillCooldown = 0;
	
	AbilityPointRestoreTrigger();
}

// Called every frame
void AAICharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	

}

void AAICharacter::NewMovement()
{
	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	FNavLocation NavLoc;
	NavSys->GetRandomReachablePointInRadius(GetActorLocation(), 10000.f, NavLoc);

	AIC_Ref = Cast<AAIController>(GetController());
	if (AIC_Ref)
	{
		AIC_Ref->MoveToLocation(NavLoc);

		GetWorldTimerManager().SetTimer(Timer, this, &AAICharacter::NewMovement, 3.0f);
	}
}
void AAICharacter::SeePawn(APawn* Pawn)
{
	ABossFightCharacter* AISee1 = Cast<ABossFightCharacter>(Pawn);



	if (AISee1 && collision == false)
	{
		GetWorldTimerManager().ClearTimer(Timer);


		AIC_Ref->MoveToLocation(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), -1.0f);
		GetCharacterMovement()->MaxWalkSpeed = 700;
		GetWorldTimerManager().SetTimer(Timer, this, &AAICharacter::NewMovement, 2.0f);
		
	}

}
void AAICharacter::OnHearNoise(APawn* OtherActor, const FVector& Location, float Volume)
{
	ABossFightCharacter* AIHear1 = Cast<ABossFightCharacter>(OtherActor);
	
	if (AIHear1 && collision == false)
	{
		
		GetWorldTimerManager().ClearTimer(Timer);


		AIC_Ref->MoveToLocation(GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation(), -1);

		GetWorldTimerManager().SetTimer(Timer, this, &AAICharacter::NewMovement, 2.0f);
		GetCharacterMovement()->MaxWalkSpeed = 700;
		
	}
}


void AAICharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABossFightCharacter* Carp1 = Cast<ABossFightCharacter>(OtherActor);
	if(Carp1 && collision == false)
	{
		collision = true;

		int A=1,B=4,random;
		random = FMath::RandRange(A,B);
        if((GetAIAbilityPoint() < 20 || FirstSkillCooldown > 0) && random == 1 )
        {
	        random = 4;
        }
		if((GetAIAbilityPoint() < 30 || SecondSkillCooldown > 0) && random == 2)
		{
			random = 4;
		}
		if((GetAIAbilityPoint() < 40 || ThirdSkillCooldown > 0) && random == 3)
		{
			random = 4;
		}
		
		switch(random)
		{
		case 1:
			GetWorldTimerManager().SetTimer(Skills, this, &AAICharacter::FirstSkill, 1.2f);
			break;
		case 2:
			GetWorldTimerManager().SetTimer(Skills, this, &AAICharacter::SecondSkill, 1.4f);
			break;
		case 3:
			GetWorldTimerManager().SetTimer(Skills, this, &AAICharacter::ThirdSkill, 1.6f);
			break;
			default:
				GetWorldTimerManager().SetTimer(Skills, this, &AAICharacter::BasicHit, 1.0f);
			
		}
	}
	
		
}

void AAICharacter::FirstSkill()
{
	MainCharacter = Cast<ABossFightCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Blue,TEXT("1.Yetenek Kullanıldı"));
	CollisionControl();
	SetAIAbilityPoint( GetAIAbilityPoint() - 20);
	MainCharacter->SetHealth(MainCharacter->GetHealth() - 10);
	KillMainCharacter();
	FirstSkillCooldown = 4;
	FirstSkillCooldownReduction();
}

void AAICharacter::SecondSkill()
{
	MainCharacter = Cast<ABossFightCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Blue,TEXT("2.Yetenek Kullanıldı"));
	CollisionControl();
	SetAIAbilityPoint( GetAIAbilityPoint() - 30);
	MainCharacter->SetHealth(MainCharacter->GetHealth() - 20);
	KillMainCharacter();
	SecondSkillCooldown = 6;
	SecondSkillCooldownReduction();
}

void AAICharacter::ThirdSkill()
{
	MainCharacter = Cast<ABossFightCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Blue,TEXT("3.Yetenek Kullanıldı"));
	CollisionControl();
	SetAIAbilityPoint( GetAIAbilityPoint() - 40);
	MainCharacter->SetHealth(MainCharacter->GetHealth() - 30);
	KillMainCharacter();
	ThirdSkillCooldown = 8;
	ThirdSkillCooldownReduction();
}

void AAICharacter::BasicHit()
{
	MainCharacter = Cast<ABossFightCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	GEngine->AddOnScreenDebugMessage(-1,5.0f,FColor::Blue,TEXT("Temel Saldiri Kullanildi"));
	CollisionControl();
	MainCharacter->SetHealth(MainCharacter->GetHealth() - 5);
	KillMainCharacter();
}

void AAICharacter::KillMainCharacter()
{
	MainCharacter = Cast<ABossFightCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
	if(MainCharacter->GetHealth() <= 0)
	{
		MainCharacter->Destroy();
	}
}
void AAICharacter::CollisionControl()
{
	collision = false;
}

void AAICharacter::FirstSkillCooldownReduction()
{
	FTimerHandle FirstSkillReductionTimer;
	if(FirstSkillCooldown > 0)
	{
		FirstSkillCooldown -= 1;
		GetWorldTimerManager().SetTimer(FirstSkillReductionTimer, this, &AAICharacter::FirstSkillCooldownReduction, 1.0f);
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Black,TEXT("aa"));
	}
}

void AAICharacter::SecondSkillCooldownReduction()
{
	FTimerHandle SecondSkillReductionTimer;
	if(SecondSkillCooldown > 0)
	{
		SecondSkillCooldown -= 1;
		GetWorldTimerManager().SetTimer(SecondSkillReductionTimer, this, &AAICharacter::SecondSkillCooldownReduction, 1.0f);	
	}
	
}

void AAICharacter::ThirdSkillCooldownReduction()
{
	FTimerHandle ThirdSkillReductionTimer;
	if(ThirdSkillCooldown > 0)
	{
		ThirdSkillCooldown -= 1;
		GetWorldTimerManager().SetTimer(ThirdSkillReductionTimer, this, &AAICharacter::ThirdSkillCooldownReduction, 1.0f);
	}
	
}

void AAICharacter::AbilityPointRestore()
{
	FTimerHandle AbilityPointRestoreTimer;
	if(GetAIAbilityPoint() < 100)
	{
		SetAIAbilityPoint(GetAIAbilityPoint() + 1);
		GetWorldTimerManager().SetTimer(AbilityPointRestoreTimer, this, &AAICharacter::AbilityPointRestore, 0.3f);
	}
	else
	{
		AbilityPointRestoreTrigger();
	}
}

void AAICharacter::AbilityPointRestoreTrigger()
{
	FTimerHandle AbilityPointTriggerTimer;
	GetWorldTimerManager().SetTimer(AbilityPointTriggerTimer, this, &AAICharacter::AbilityPointRestore, 0.1f);
}














