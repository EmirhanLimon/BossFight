// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossFightCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "AICharacter.h"
#include "GameFramework/SpringArmComponent.h"

//////////////////////////////////////////////////////////////////////////
// ABossFightCharacter

ABossFightCharacter::ABossFightCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	BossFightCharacterCompCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("BossFightCharacterCompCapsuleCpp"));
	BossFightCharacterCompCapsule->SetupAttachment(GetRootComponent());

	collision = false;
	Completed = true;

	FirstSkillCooldown = 0;
	SecondSkillCooldown = 0;
	ThirdSkillCooldown = 0;

	AbilityPointPotion = 30;
	HealthPotion = 50;

	AbilityPointPotionPiece = 5;
	HealthPotionPiece = 5;

	AbilityPointPotionCooldown = 0;
	HealthPotionCooldown = 0;
}

void ABossFightCharacter::BeginPlay()
{
	Super::BeginPlay();

	BossFightCharacterCompCapsule->OnComponentBeginOverlap.AddDynamic(this, &ABossFightCharacter::BeginOverlap);
	BossFightCharacterCompCapsule->OnComponentEndOverlap.AddDynamic(this, &ABossFightCharacter::OnOverlapEnd);
	
	AbilityPointRestoreTrigger();
}




//////////////////////////////////////////////////////////////////////////
// Input

void ABossFightCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("FirstSkill", IE_Pressed, this, &ABossFightCharacter::FirstSkill);
	PlayerInputComponent->BindAction("SecondSkill", IE_Pressed, this, &ABossFightCharacter::SecondSkill);
	PlayerInputComponent->BindAction("ThirdSkill", IE_Pressed, this, &ABossFightCharacter::ThirdSkill);
	PlayerInputComponent->BindAction("BasicAttack", IE_Pressed, this, &ABossFightCharacter::BasicAttack);

	PlayerInputComponent->BindAction("HealthPotion", IE_Pressed, this, &ABossFightCharacter::UseHealthPotion);
	PlayerInputComponent->BindAction("AbilityPointPotion", IE_Pressed, this, &ABossFightCharacter::UseAbilityPointPotion);
	
	
	PlayerInputComponent->BindAxis("Move Forward / Backward", this, &ABossFightCharacter::MoveForward);
	PlayerInputComponent->BindAxis("Move Right / Left", this, &ABossFightCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn Right / Left Mouse", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("Turn Right / Left Gamepad", this, &ABossFightCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("Look Up / Down Mouse", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Look Up / Down Gamepad", this, &ABossFightCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ABossFightCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ABossFightCharacter::TouchStopped);
}

void ABossFightCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void ABossFightCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void ABossFightCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABossFightCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void ABossFightCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ABossFightCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ABossFightCharacter::BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AAICharacter* Carp1 = Cast<AAICharacter>(OtherActor);	
	if(Carp1 && collision == false)
	{
		
		collision = true;
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Red,TEXT("asda"));		
	}
}

void ABossFightCharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	collision = false;
}


void ABossFightCharacter::FirstSkill()
{
	FTimerHandle FirstSkillTimer;
	if(collision == true && FirstSkillCooldown <= 0 && Completed == true)
	{
		
		if(GetAbilityPoint() >= 20)
		{
			GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Cyan,TEXT("1.Yetenek Kullanildi"));
			SetAbilityPoint(GetAbilityPoint() - 20);
			SetAIHealth(GetAIHealth() - 20);
			FirstSkillCooldown = 6;
			Completed = false;
			FirstSkillCooldownReduction();
			GetWorldTimerManager().SetTimer(FirstSkillTimer, this, &ABossFightCharacter::CompletedControl, 1.2f);	
		}
		
	}
}

void ABossFightCharacter::SecondSkill()
{
	FTimerHandle SecondSkillTimer;
	if(collision == true && SecondSkillCooldown <= 0 && Completed == true)
	{
		
		if(GetAbilityPoint() >= 30)
		{
			GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Cyan,TEXT("2.Yetenek Kullanildi"));
			SetAbilityPoint(GetAbilityPoint() - 30);
			SetAIHealth(GetAIHealth() - 30);
			SecondSkillCooldown = 8;
			Completed = false;
			SecondSkillCooldownReduction();
			GetWorldTimerManager().SetTimer(SecondSkillTimer, this, &ABossFightCharacter::CompletedControl, 1.4f);	
		}
	}
	
	
}

void ABossFightCharacter::ThirdSkill()
{
	FTimerHandle ThirdSkillTimer;
	if(collision == true && ThirdSkillCooldown <= 0 && Completed == true)
	{
		
		if(GetAbilityPoint() >= 40)
		{
			GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Cyan,TEXT("3.Yetenek Kullanildi"));
			SetAbilityPoint(GetAbilityPoint() - 40);
			SetAIHealth(GetAIHealth() - 40);
			ThirdSkillCooldown = 10;
			Completed = false;
			ThirdSkillCooldownReduction();
			GetWorldTimerManager().SetTimer(ThirdSkillTimer, this, &ABossFightCharacter::CompletedControl, 1.6f);	
		}	
	}
	
	
	
}

void ABossFightCharacter::BasicAttack()
{
	FTimerHandle BasicAttackTimer;
	if(collision == true  && Completed == true)
	{
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Cyan,TEXT("Basic Attack Kullanildi"));
		SetAIHealth(GetAIHealth() - 10);
		Completed = false;
		GetWorldTimerManager().SetTimer(BasicAttackTimer, this, &ABossFightCharacter::CompletedControl, 1.0f);
	}
}

void ABossFightCharacter::CompletedControl()
{
	Completed = true;
}

void ABossFightCharacter::AbilityPointRestore()
{
	FTimerHandle AbilityPointRestoreTimer;
	if(GetAbilityPoint() < 100)
	{
		SetAbilityPoint(GetAbilityPoint() + 1);
		GetWorldTimerManager().SetTimer(AbilityPointRestoreTimer, this, &ABossFightCharacter::AbilityPointRestore, 0.3f);
	}
	else
	{
		AbilityPointRestoreTrigger();
	}
	
}

void ABossFightCharacter::AbilityPointRestoreTrigger()
{
	FTimerHandle AbilityPointTriggerTimer;
	GetWorldTimerManager().SetTimer(AbilityPointTriggerTimer, this, &ABossFightCharacter::AbilityPointRestore, 0.1f);
	
}

void ABossFightCharacter::FirstSkillCooldownReduction()
{
	FTimerHandle FirstSkillReductionTimer;
	if(FirstSkillCooldown > 0)
	{
		FirstSkillCooldown -= 1;
		GetWorldTimerManager().SetTimer(FirstSkillReductionTimer, this, &ABossFightCharacter::FirstSkillCooldownReduction, 1.0f);
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Black,TEXT("aa"));
	}
}

void ABossFightCharacter::SecondSkillCooldownReduction()
{
	FTimerHandle SecondSkillReductionTimer;
	if(SecondSkillCooldown > 0)
	{
		SecondSkillCooldown -= 1;
		GetWorldTimerManager().SetTimer(SecondSkillReductionTimer, this, &ABossFightCharacter::SecondSkillCooldownReduction, 1.0f);
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Black,TEXT("aa"));
	}
}

void ABossFightCharacter::ThirdSkillCooldownReduction()
{
	FTimerHandle ThirdSkillReductionTimer;
	if(ThirdSkillCooldown > 0)
	{
		ThirdSkillCooldown -= 1;
		GetWorldTimerManager().SetTimer(ThirdSkillReductionTimer, this, &ABossFightCharacter::ThirdSkillCooldownReduction, 1.0f);
		GEngine->AddOnScreenDebugMessage(-1,1.0f,FColor::Black,TEXT("aa"));
	}
}

void ABossFightCharacter::UseHealthPotion()
{
	if(Health < 200 && HealthPotionCooldown <= 0 && HealthPotionPiece >0)
	{
		if(Health < 200 && Health >= 150)
		{
			Health = 200;
			
		}
		if(Health < 150 && Health > 0)
		{
			Health = Health + HealthPotion;
		}
		HealthPotionCooldown = 10;
		HealthPotionPiece--;
		HealthPotionCooldownReduction();
	}
	
}

void ABossFightCharacter::UseAbilityPointPotion()
{

	if(AbilityPoint < 100 && AbilityPointPotionCooldown <= 0 && AbilityPointPotionPiece >0)
	{
		if(AbilityPoint < 100 && AbilityPoint >= 70)
		{
			AbilityPoint = 100;
			
		}
		if(AbilityPoint < 70 && AbilityPoint > 0)
		{
			AbilityPoint = AbilityPoint + AbilityPointPotion;
		}
		AbilityPointPotionCooldown = 10;
		AbilityPointPotionPiece--;
		AbilityPotionCooldownReduction();
	}
}

void ABossFightCharacter::AbilityPotionCooldownReduction()
{
	FTimerHandle AbilityPointPotionTimer;
	if(AbilityPointPotionCooldown > 0)
	{
		AbilityPointPotionCooldown--;
		GetWorldTimerManager().SetTimer(AbilityPointPotionTimer, this, &ABossFightCharacter::AbilityPotionCooldownReduction, 1.0f);
	}
 
	
}

void ABossFightCharacter::HealthPotionCooldownReduction()
{
	FTimerHandle HealthPotionTimer;
	if(HealthPotionCooldown > 0)
	{
		HealthPotionCooldown--;
		GetWorldTimerManager().SetTimer(HealthPotionTimer, this, &ABossFightCharacter::HealthPotionCooldownReduction, 1.0f);
	}
	
}






