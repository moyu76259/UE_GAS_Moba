// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/IHttpResponse.h"
#include "Interfaces/IHttpRequest.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CGameInstance.generated.h"


DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnLoginCompleted, bool /*bWasSuccessful*/, const FString& /*PlayerNickName*/,
	const FString& /*ErrorMsg**/)
DECLARE_MULTICAST_DELEGATE_OneParam(FOnGlobalSearchCompleted, const TArray<FOnlineSessionSearchResult>& /*SearchResults*/)
DECLARE_MULTICAST_DELEGATE(FOnJoinSessionFailed);
/**
 * 
 */
UCLASS()
class UCGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	void StartMatch();
	virtual void Init() override;
/**********************************************************/
/*                        Login                           */
/**********************************************************/
public:
	bool IsLoggedIn() const;
	bool IsLoggingIn() const;
	void ClientAccountPortalLogin();

	FOnLoginCompleted OnLoginCompleted;
private:
	void ClientLogin(const FString& Type, const FString& Id, const FString& Token);
	void LoginCompleted(int NumOfLocalPlayer, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	FDelegateHandle LoggingInDelegateHandle;
/**********************************************************/
/*           Client Session Creation and Search           */
/**********************************************************/
public:
	void RequestCreateAndJoinSessison(const FName& NewSessionName);
	void CancelSessionCreation();
	void StartGlobalSessionSearch();
	bool JoinSessionWithId(const FString& SessionIdStr);

	FOnJoinSessionFailed OnJoinSessionFailed;
	FOnGlobalSearchCompleted OnGlobalSearchCompleted;
private:
	void SessionCreationRequestCompleted(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully, FGuid SessionSearchId);
	void StartFindCreatedSession(const FGuid& SessionSearchId);
	void StopAllSessionFindings();
	void StopFindingCreatedSession();
	void StopGlobalSessionSearch();
	void FindGlobalSessions();
	void GlobalSessionSearchCompleted(bool bWasSuccessful);

	FTimerHandle FindCreatedSessionTimerHandle;
	FTimerHandle FindCreatedSessionTimeoutTimerHandle;
	FTimerHandle GlobalSessionSearchTimerHandle;

	UPROPERTY(EditDefaultsOnly, Category="Session Search")
	float FindCreatedSessionSearchInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category="Session Search")
	float GlobalSessionSearchInterval = 2.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Session Search")
	float FindCreatedSessionTimeoutDuration = 60.f;

	void FindCreatedSession(FGuid SessionSearchId);
	void FindCreatedSessionTimeout();
	void FindCreateSessionCompleted(bool bWasSuccessful);
	void JoinSessionWithSearchResult(const class FOnlineSessionSearchResult& SearchResult);
	void JoinSessionCompleted(FName SessionName, EOnJoinSessionCompleteResult::Type JoinResult, int Port);

	TSharedPtr<class FOnlineSessionSearch> SessionSearch;

/**********************************************************/
/*                    Session Server                      */
/**********************************************************/
public:
	void PlayerJoined(const FUniqueNetIdRepl& UniqueId);
	void PlayerLeft(const FUniqueNetIdRepl& UniqueId);
private:
	void CreateSession();
	void OnSessionCreated(FName SessionName, bool bWasSuccessful);
	void EndSessionCompleted(FName SessionName, bool bWasSuccessful);
	FString ServerSessionName;
	int SessionServerPort;

	void TerminateSessionServer();
	
	FTimerHandle WaitPlayerjoinTimeOuthHandle;

	UPROPERTY(EditDefaultsOnly, Category="Session")
	float WaitPlayerjoinTimeOutDuration = 60.f;
	
	void WaitPlayerjoinTimeOutReached();

	TSet<FUniqueNetIdRepl> PlayerRecord;

private:
	UPROPERTY(EditDefaultsOnly, Category="Map")
	TSoftObjectPtr<UWorld> MainMenuLevel;

	UPROPERTY(EditDefaultsOnly, Category="Map")
	TSoftObjectPtr<UWorld> LobbyLevel;

	UPROPERTY(EditDefaultsOnly, Category="Map")
	TSoftObjectPtr<UWorld> GameLevel;

	void LoadLevelAndListen(TSoftObjectPtr<UWorld> Level);
};


