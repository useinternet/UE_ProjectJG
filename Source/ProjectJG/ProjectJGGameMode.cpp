// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProjectJGGameMode.h"

#include <string>

#include "ProjectJGCharacter.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Sockets.h"
#include "UObject/ConstructorHelpers.h"

AProjectJGGameMode::AProjectJGGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.bCanEverTick = true;
}

void AProjectJGGameMode::BeginPlay()
{
	Super::BeginPlay();
	// 소켓을 생성
	Socket = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(TEXT("Stream"), TEXT("Client Socket"));

	// IP를 FString으로 입력받아 저장
	FString address = TEXT("127.0.0.1");
	FIPv4Address ip;
	FIPv4Address::Parse(address, ip);

	int32 port = 8000;	// 포트는 6000번

	// 포트와 소켓을 담는 클래스
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(port);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Trying to connect.")));

	// 연결시도, 결과를 받아옴
	bool isConnetcted = Socket->Connect(*addr);
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Connect Result %d"), (int32)isConnetcted));

	std::string uuid = TCHAR_TO_ANSI(*FGuid::NewGuid().ToString());
	std::string test = "test data ahahahah jg game " + uuid;
	int ss = 0;
	Socket->Send((uint8*)test.c_str(), test.length() + 1, ss);
}

void AProjectJGGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);


	// if(GetWorld()->GetFirstPlayerController())
	// {
	// 	std::string Test = "data_test";
	// 	int32 testss = 0;
	// 	
	// 	Socket->Send((uint8*)(Test.c_str()), 10, testss);
	//
	// 	char test[10] = {};
	// 	Socket->Recv((uint8*)test, 10, testss);
	// 	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Recv Result %s"), ANSI_TO_TCHAR(test)));
	// }
}
