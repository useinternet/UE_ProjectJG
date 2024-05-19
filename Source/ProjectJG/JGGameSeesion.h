// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "JGGameSeesion.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTJG_API AJGGameSeesion : public AGameSession
{
	GENERATED_BODY()
	

protected:
	virtual void BeginPlay() override;
};
