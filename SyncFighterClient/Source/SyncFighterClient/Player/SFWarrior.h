// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFCharacter.h"
#include "SFWarrior.generated.h"

UCLASS()
class SYNCFIGHTERCLIENT_API ASFWarrior : public ASFCharacter
{
	GENERATED_BODY()
	
public:
	virtual void EndState() override;

	virtual void ProcessBasicAttack() override;
	virtual void ProcessSkillQ() override;
	virtual void ProcessSkillE() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillQMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillEMontage;
};
