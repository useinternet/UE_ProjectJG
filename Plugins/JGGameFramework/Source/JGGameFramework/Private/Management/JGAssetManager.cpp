// Fill out your copyright notice in the Description page of Project Settings.
#include "Management/JGAssetManager.h"
#include "AssetRegistry/AssetRegistryModule.h"

DEFINE_LOG_CATEGORY(JGAssetMgr);

static TAutoConsoleVariable<bool> CVar_JGAssetMgr_ShowDetailLog(TEXT("c.JGAssetMgr.ShowDetailLog"), false, TEXT("Show JGAssetManager Detail Log"));

struct FJGResourceIDProvider
{
	uint64 IDOffset;
	TArray<uint64> RemainIDs;

public:
	FJGResourceIDProvider()
	{
		IDOffset = 0;
	}

	uint64 ReceiveID()
	{
		if (RemainIDs.IsEmpty())
		{
			return IDOffset++;
			
		}
		else
		{
			return RemainIDs.Pop();
		}
	}

	void GiveID(uint64 InID)
	{
		RemainIDs.Add(InID);
	}
};

FJGAssetRequestID FJGAssetRequestID::Invalid = FJGAssetRequestID(JGASSETID_NONE);

UJGAssetManager::UJGAssetManager(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}


FJGAssetRequestID UJGAssetManager::RequestAsyncLoad(const FSoftObjectPath& InAssetPath, const FJGResourceLoadCompeleteDelegate& InCompeleteDelegate)
{
	FJGAssetID AssetID = AddAsset(InAssetPath);
	return PushAssetRequest({ AssetID }, InCompeleteDelegate);
}

FJGAssetRequestID UJGAssetManager::RequestAsyncLoad(const TArray<FSoftObjectPath>& InAssetPaths, const FJGResourceLoadCompeleteDelegate& InCompeleteDelegate)
{
	TArray<FJGAssetID> AssetIDs = AddAssets(InAssetPaths);
	return PushAssetRequest(AssetIDs, InCompeleteDelegate);
}

UObject* UJGAssetManager::ResolveObject(const FSoftObjectPath& InResourcePath) const
{
	return InResourcePath.ResolveObject();
}

void UJGAssetManager::Init()
{
	AssetIDProvider   = MakeShared<FJGResourceIDProvider>();
	RequestIDProvider = MakeShared<FJGResourceIDProvider>();
}

void UJGAssetManager::Shutdown()
{

}

void UJGAssetManager::Tick(float DeltaTime)
{
	for(int i = 0; i < AsyncLoadCountPerTick; ++i)
	{
		FJGAssetRequest   Request;
		if(PopAssetRequest(Request) == false)
		{
			break;
		}
		
		TArray<FSoftObjectPath> TargetAssetPaths;
		for(FJGAssetID AssetID : Request.RequestedAssetIDs)
		{
			TargetAssetPaths.Add(Assets[AssetID].AssetPath);
		}

		StreamableManager.RequestAsyncLoad(TargetAssetPaths, FStreamableDelegate::CreateLambda([this](FJGAssetRequest  RequestInLambda)
		{
			for(FJGAssetID AssetID : RequestInLambda.RequestedAssetIDs)
			{
				FJGAssetData& AssetData = Assets[AssetID];
				AssetData.State  = FJGAssetData::LoadCompelete;
				AssetData.Object = AssetData.AssetPath.ResolveObject();
			}

			for(FJGResourceLoadCompeleteDelegate CompeleteDelegate : RequestInLambda.CompeleteDelegates)
			{
				CompeleteDelegate.ExecuteIfBound();
			}
			
		}, Request));
	}
}

TStatId UJGAssetManager::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UAssetManager, STATGROUP_Tickables);
}


FJGAssetID UJGAssetManager::AddAsset(const FSoftObjectPath& InAssetPath, FJGAssetData::EState InState)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	if(AssetRegistryModule.Get().PathExists(InAssetPath.ToString()) == false)
	{
		UE_LOG(JGAssetMgr, Error, TEXT("%s : Not Exist Asset"), *InAssetPath.ToString())
		return JGASSETID_NONE;
	}
	
	if(HasAsset(InAssetPath))
	{
		return FindAssetID(InAssetPath);	
	}
	
	FJGAssetID AssetID = AssetIDProvider->ReceiveID();
	
	FJGAssetData AssetData;
	AssetData.State        = FJGAssetData::EState::Loading;
	AssetData.AssetPath    = InAssetPath;
	AssetData.ID           = AssetID;
	AssetData.Object       = nullptr;

	Assets[AssetID] = AssetData;
	AssetIDByPath[AssetData.AssetPath] = AssetID;
	
	return AssetID;
}

TArray<FJGAssetID> UJGAssetManager::AddAssets(const TArray<FSoftObjectPath>& InAssetPaths, FJGAssetData::EState InState)
{
	TArray<FJGAssetID> Result;
	for(const FSoftObjectPath& AssetPath : InAssetPaths)
	{
		FJGAssetID AssetID = AddAsset(AssetPath, InState);
		Result.Add(AssetID);
	}

	return Result;
}

bool UJGAssetManager::HasAsset(const FSoftObjectPath& InAssetPath)
{
	return AssetIDByPath.Contains(InAssetPath);
}

bool UJGAssetManager::HasAsset(FJGAssetID InID)
{
	return Assets[InID].IsValid();
}

void UJGAssetManager::RemoveAsset(FJGAssetID InID)
{
	const FSoftObjectPath& AssetPath = Assets[InID].AssetPath;

	AssetIDByPath.Remove(AssetPath);
	Assets[InID].State = FJGAssetData::EState::Invalid;
	
	AssetIDProvider->GiveID(InID);
}

FJGAssetID UJGAssetManager::FindAssetID(const FSoftObjectPath& InAssetPath)
{
	if(HasAsset(InAssetPath) == false)
	{
		return JGASSETID_NONE;
	}

	return AssetIDByPath[InAssetPath];
}

FJGAssetData* UJGAssetManager::FindAsset(const FSoftObjectPath& InAssetPath)
{
	if(HasAsset(InAssetPath) == false)
	{
		return nullptr;
	}

	return FindAsset(AssetIDByPath[InAssetPath]);
}

FJGAssetData* UJGAssetManager::FindAsset(FJGAssetID InID)
{
	if(Assets[InID].IsValid() == false)
	{
		return nullptr;
	}

	return &Assets[InID];
}

FJGAssetRequestID UJGAssetManager::PushAssetRequest(const TArray<FJGAssetID>& InAssetIDs, const FJGResourceLoadCompeleteDelegate& InCompeleteDelegate)
{
	FJGAssetRequestID RequestID = RequestIDProvider->ReceiveID();
	
	FJGAssetRequest AssetRequest;
	AssetRequest.RequestID = RequestID;
	AssetRequest.RequestedAssetIDs = InAssetIDs;
	
	if(InCompeleteDelegate.IsBound())
	{
		AssetRequest.CompeleteDelegates.Add(InCompeleteDelegate);
	}
	
	AssetRequests[AssetRequest.RequestID.ID] = AssetRequest;
	StreamingRequestQueue.Push(AssetRequest.RequestID);
	
	return RequestID;
}

bool UJGAssetManager::PopAssetRequest(FJGAssetRequest& AssetRequest)
{
	if(StreamingRequestQueue.IsEmpty())
	{
		return false;
	}

	FJGAssetRequestID RequestID = StreamingRequestQueue.Pop();
	
	AssetRequest = AssetRequests[RequestID.ID];
	
	AssetRequests[AssetRequest.RequestID.ID].RequestID = JGASSETID_NONE;
	AssetRequests[AssetRequest.RequestID.ID].CompeleteDelegates.Empty();
	AssetRequests[AssetRequest.RequestID.ID].RequestedAssetIDs.Empty();

	RequestIDProvider->GiveID(RequestID.ID);
	
	return true;
}
