// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "JGDataLoader.generated.h"

/**
 * 
 */

UCLASS()
class JGGAMEFRAMEWORK_API UJGDataTable : public UDataTable
{
	GENERATED_BODY()
	
};


UCLASS()
class JGGAMEFRAMEWORK_API UJGDataLoader : public UGameInstance
{
	GENERATED_BODY()


	// UGameInstance
protected:
	virtual void Init() {}
	virtual void Shutdown() {}
	// ~UGameInstance
};
