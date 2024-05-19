// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Engine/StreamableManager.h"
#include "JGAssetManager.generated.h"

/**
 * 
 */

DECLARE_LOG_CATEGORY_EXTERN(JGAssetMgr, Log, All);

struct FJGResourceIDProvider;

using FJGAssetID = uint64;

enum { JGASSETID_NONE = INDEX_NONE };

DECLARE_DELEGATE(FJGResourceLoadCompeleteDelegate)

USTRUCT()
struct FJGAssetData
{
	GENERATED_BODY()

	enum EState
	{
		Invalid,
		Loading,
		LoadCompelete,
		LoadFail,
	};
	
	EState   State;
	FJGAssetID ID;

	UPROPERTY()
	UObject* Object;

	UPROPERTY()
	FSoftObjectPath AssetPath;
	
	FJGAssetData(EState InState = EState::Invalid)
	{
		ID = JGASSETID_NONE;
		State = InState;
		Object   = nullptr;
	}

	bool IsValid() const
	{
		return ::IsValid(Object) && State != EState::Invalid && AssetPath.IsValid() && ID != JGASSETID_NONE;
	}
};

USTRUCT()
struct FJGAssetRequestID
{
	GENERATED_BODY()
	
	static FJGAssetRequestID Invalid;

	friend class UJGAssetManager;
	
private:
	uint64 ID = 0;
	TArray<FJGResourceLoadCompeleteDelegate> LoadCompeleteDelegates;
	
public:
	FJGAssetRequestID(uint64 InID = JGASSETID_NONE)
	{
		ID = InID;
	}
};

USTRUCT()
struct FJGAssetRequest
{
	GENERATED_BODY()
	
	FJGAssetRequestID  RequestID;
	TArray<FJGAssetID> RequestedAssetIDs;
	TArray<FJGResourceLoadCompeleteDelegate> CompeleteDelegates;
};

UCLASS()
class JGGAMEFRAMEWORK_API UJGAssetManager : public UGameInstance, public FTickableGameObject
{
	GENERATED_BODY()


private:
	static const int32 AsyncLoadCountPerTick = 10;
	
	FStreamableManager StreamableManager;
	
	TArray<FJGAssetRequestID> StreamingRequestQueue;
	TSharedPtr<FJGResourceIDProvider> AssetIDProvider;
	TSharedPtr<FJGResourceIDProvider> RequestIDProvider;
	TMap<FSoftObjectPath, FJGAssetID> AssetIDByPath;
	
	TArray<FJGAssetData>    Assets;
	TArray<FJGAssetRequest> AssetRequests;
public:
	UJGAssetManager(const FObjectInitializer& ObjectInitializer);


	
public:
	// 리소스 로드 요청
	FJGAssetRequestID RequestAsyncLoad(const FSoftObjectPath& InAssetPath, const FJGResourceLoadCompeleteDelegate& InCompeleteDelegate);
	FJGAssetRequestID RequestAsyncLoad(const TArray<FSoftObjectPath>& InAssetPaths, const FJGResourceLoadCompeleteDelegate& InCompeleteDelegate);

	// 유틸 함수
	UObject* ResolveObject(const FSoftObjectPath& InResourcePath) const;

protected:

	// UGameInstance
	virtual void Init() override;
	virtual void Shutdown() override;
	// ~UGameInstance

	// FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	// ~FTickableGameObject

private:
	FJGAssetID AddAsset(const FSoftObjectPath& InAssetPath, FJGAssetData::EState InState = FJGAssetData::EState::Loading);
	TArray<FJGAssetID> AddAssets(const TArray<FSoftObjectPath>& InAssetPaths, FJGAssetData::EState InState = FJGAssetData::EState::Loading);
	bool HasAsset(const FSoftObjectPath& InAssetPath);
	bool HasAsset(FJGAssetID InID);
	void RemoveAsset(FJGAssetID InID);
	FJGAssetID FindAssetID(const FSoftObjectPath& InAssetPath);
	FJGAssetData* FindAsset(const FSoftObjectPath& InAssetPath);
	FJGAssetData* FindAsset(FJGAssetID InID);
	
	FJGAssetRequestID PushAssetRequest(const TArray<FJGAssetID>& InAssetIDs, const FJGResourceLoadCompeleteDelegate& InCompeleteDelegate);
	bool PopAssetRequest(FJGAssetRequest& AssetRequest);
};