// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "JGBattleSystem.generated.h"

class IJGBattleObject;
/**
 * 
 */
UCLASS()
class JGGAMEFRAMEWORK_API UJGBattleSystem : public UGameInstance
{
	GENERATED_BODY()

public:
	void RegisterBattleObject(IJGBattleObject* InBattleObject);
	void UnRegisterBattleObject(IJGBattleObject* InBattleObject);

	// 데미지 주고 받는건 서버에서
};
