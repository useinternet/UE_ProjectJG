// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ProjectJGGameMode.generated.h"

UCLASS(minimalapi)
class AProjectJGGameMode : public AGameModeBase
{
	GENERATED_BODY()

	class FSocket* Socket;
public:
	AProjectJGGameMode();
	
	
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
};



