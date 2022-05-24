// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BossFightCharacter.generated.h"

UCLASS(config=Game)
class ABossFightCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	ABossFightCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
	float TurnRateGamepad;

protected:

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	virtual void BeginPlay() override;

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	bool collision;
	bool Completed;
	UFUNCTION()
	void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UPROPERTY(EditDefaultsOnly)
	class UCapsuleComponent* BossFightCharacterCompCapsule;

	void FirstSkill();
	void SecondSkill();
	void ThirdSkill();
	void BasicAttack();
	void CompletedControl();
	void AbilityPointRestore();
	void AbilityPointRestoreTrigger();

	void FirstSkillCooldownReduction();
	void SecondSkillCooldownReduction();
	void ThirdSkillCooldownReduction();
	
private:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float Health = 200;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float Attack = 10;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float AbilityPoint = 100;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float AIHealth = 500;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	int FirstSkillCooldown;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	int SecondSkillCooldown;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	int ThirdSkillCooldown;
public:
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE float GetHealth() { return Health; }

	FORCEINLINE void SetAttack(float NewAttack) { Attack = NewAttack; }
	FORCEINLINE float GetAttack() { return Attack; }

	FORCEINLINE void SetAbilityPoint(float NewAbilityPoint) { AbilityPoint = NewAbilityPoint; }
	FORCEINLINE float GetAbilityPoint() { return AbilityPoint; }

	FORCEINLINE void SetAIHealth(float NewAIHealth) { AIHealth = NewAIHealth; }
	FORCEINLINE float GetAIHealth() { return AIHealth; }
};

