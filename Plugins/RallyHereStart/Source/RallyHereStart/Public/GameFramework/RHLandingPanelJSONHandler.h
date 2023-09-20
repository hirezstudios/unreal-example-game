#pragma once
#include "UObject/Object.h"
#include "HttpModule.h"
#include "Json.h"
#include "Modules/ModuleManager.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Engine/TextureRenderTarget2D.h"
#include "IImageWrapperModule.h"
#include "RHLandingPanelJSONHandler.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLandingPanelLoadComplete, URHLandingPanelJSONHandler*, pHandler);

UCLASS()
class RALLYHERESTART_API URHLandingPanelJSONHandler : public UObject
{
    GENERATED_UCLASS_BODY()

public:
    virtual void Initialize(FString strName);
    FString GetName() { return strLandingPanelName;  };
    FString GetRawJSON() { return strRawJSON; }
    FString GetFileDirectory() { return strLocalDir; }
    bool IsReady() { return bImagesLoaded && bJSONReady; }
    TSharedPtr<FJsonObject> GetJsonObject() const { return MyJsonObject; }
    FString* LookUpFilePathByUrl(FString url) { return mapRemoteUrlToFilePath.Find(url); }
    UTexture2DDynamic* LookUpTextureByFilePath(FString path) { return *mapFilePathToTexture.Find(path); }
    TMap<FString, FString>& GetRemoteUrlToFilePathMap() { return mapRemoteUrlToFilePath; };
    TMap<FString, UTexture2DDynamic*>& GetFilePathToTextureMap() { return mapFilePathToTexture; };

    void StartLoadJson(FString strJsonUrl);
	
	UPROPERTY()
    FLandingPanelLoadComplete OnJsonReady;

    UPROPERTY()
    FLandingPanelLoadComplete OnImagesDownloaded;

private:
    bool bImagesLoaded = false;
    bool bJSONReady = false;

    // landing panel name
    FString strLandingPanelName;
    // url where the latest JSON can be found
    FString strJSONConfigURL;
    // raw json string
    FString strRawJSON;
    // local cached file root directory for this landing panel
    FString strLocalDir;
    // shared pointer to JSON object
    TSharedPtr<FJsonObject> MyJsonObject;
    // maps to keep track of image objects
    TArray<FString> imageURLsToDownload;
    TMap<FString, FString> mapRemoteUrlToFilePath;
    UPROPERTY()
    TMap<FString, UTexture2DDynamic*> mapFilePathToTexture;

    bool CreateVerifyDownloadDirectories();
    bool DeleteDirectory(FString dirPath);
    bool CreateVerifyFileDirectory(const FString& strDirPath);
    bool CreateVerifyFileDirectoryTree(const FString& strDirPath);

    bool LoadJsonFromFile(const FString& strFilePath, FString& JsonString);
    bool SaveJsonToFile(const FString& strFilePath, FString strJson);

    UFUNCTION()
    void RequestNewJson();

    bool FindInJson(TMap<FString, TArray<FString>>& Results, FString strNeedle, TSharedPtr<FJsonObject>& jsonObjectHayStack);
    void DeepJsonStringSearch(TMap<FString, TArray<FString>>& Results, FString strNeedle, TMap<FString, TSharedPtr<FJsonValue>>& jsonValues);

    void OnJSONRequestComplete(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void ParseJson();

    void DownloadImage(FString strImageUrl);
    void DownloadImage(TArray<FString> ImageUrls);
    void HandleImageDLResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);
    bool PrepareTexture(const uint8* contentData, int32 contentLength, FString strSaveFilePath);

};