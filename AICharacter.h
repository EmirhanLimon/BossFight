// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "AIController.h"
#include "CoreMinimal.h"
#include "BossFightCharacter.h"
#include "GameFramework/Character.h"
#include "AICharacter.generated.h"


class UPawnSensingComponent;

UCLASS()
class BOSSFIGHT_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAICharacter();

	void FirstSkill();
	void SecondSkill();
	void ThirdSkill();
	void BasicHit();
    
	void FirstSkillCooldownReduction();
	void SecondSkillCooldownReduction();
	void ThirdSkillCooldownReduction();
	
    void AbilityPointRestore();
	void AbilityPointRestoreTrigger();
	UFUNCTION()
	    void CollisionControl();
    UFUNCTION()
	    void KillMainCharacter();
	UFUNCTION()
		void NewMovement();
	UFUNCTION()
		void SeePawn(APawn* Pawn);
	UFUNCTION()
		void OnHearNoise(APawn* OtherActor, const FVector& Location, float Volume);
	UFUNCTION()
		void BeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
	UPawnSensingComponent* PawnSensing;
	UPROPERTY()
	AAIController* AIC_Ref;
	UPROPERTY()
	FTimerHandle Timer;
	UPROPERTY()
	FTimerHandle Skills;
	UPROPERTY(EditDefaultsOnly)
	class UCapsuleComponent* AICharacterCompCapsule;
	ABossFightCharacter*MainCharacter;
	bool collision;

	
	
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	int FirstSkillCooldown;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	int SecondSkillCooldown;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	int ThirdSkillCooldown;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float AIHealth = 100;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float AIAttack = 10;
	UPROPERTY(EditDefaultsOnly,BlueprintReadWrite,meta=(AllowPrivateAccess="true"))
	float AIAbilityPoint = 100;
public:
	FORCEINLINE void SetAIHealth(float AINewHealth) { AIHealth = AINewHealth; }
	FORCEINLINE float GetAIHealth() { return AIHealth; }

	FORCEINLINE void SetAIAttack(float AINewAttack) { AIAttack = AINewAttack; }
	FORCEINLINE float GetAIAttack() { return AIAttack; }

	FORCEINLINE void SetAIAbilityPoint(float NewAIAbilityPoint) { AIAbilityPoint = NewAIAbilityPoint ;}
	FORCEINLINE float GetAIAbilityPoint() {return AIAbilityPoint; }

	

};
