// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SFCharacter.h"
#include "SFMage.generated.h"

UCLASS()
class SYNCFIGHTERCLIENT_API ASFMage : public ASFCharacter
{
	GENERATED_BODY()
	
public:
	virtual void EndState() override;

	virtual void ProcessBasicAttack() override;
	virtual void ProcessSkillQ() override;
	virtual void ProcessSkillE() override;

	// --- [투사체(마법) 블루프린트 할당 칸] ---
	UPROPERTY(EditAnywhere, Category = "Combat|Magic")
	TSubclassOf<class AActor> BasicFireballClass;

	UPROPERTY(EditAnywhere, Category = "Combat|Magic")
	TSubclassOf<class AActor> SkillQMagicClass; // 얼음 속박 or 거대 화염구

	// --- [마법 시전 애니메이션 칸] ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillQMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Montages")
	UAnimMontage* SkillEMontage;

public:
	UFUNCTION(BlueprintCallable, Category = "Combat|Magic")
	void FireMagic(FName SocketName);
};
