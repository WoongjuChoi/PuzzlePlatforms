// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "InGameMenu.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"
#include "Online/OnlineSessionNames.h"

const static FName SESSION_NAME = TEXT("My Session Game");
const static FName SERVER_NAME_SETTINGS_KEY = TEXT("ServerName");

UPuzzlePlatformsGameInstance::UPuzzlePlatformsGameInstance(const FObjectInitializer& ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/MenuSystem/WBP_MainMenu.WBP_MainMenu_C'"));
	if (MenuBPClass.Class == nullptr)
		return;

	MenuClass = MenuBPClass.Class;

	static ConstructorHelpers::FClassFinder<UUserWidget> InGameMenuBPClass(TEXT("/Script/UMGEditor.WidgetBlueprint'/Game/MenuSystem/WBP_InGameMenu.WBP_InGameMenu_C'"));
	if (InGameMenuBPClass.Class == nullptr)
		return;

	InGameMenuClass = InGameMenuBPClass.Class;
}

void UPuzzlePlatformsGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString());
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnCreateSessionComplete);
			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnDestroySessionComplete);
			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnFindSessionComplete);
			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &UPuzzlePlatformsGameInstance::OnJoinSessionComplete);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Found no subsystem"));
	}
}

void UPuzzlePlatformsGameInstance::LoadMenuWidget()
{
	if (MenuClass == nullptr)
		return;
 
	Menu = CreateWidget<UMainMenu>(this, MenuClass);
	if (Menu == nullptr)
		return;

	Menu->Setup();
	Menu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::InGameLoadMenu()
{
	if (InGameMenuClass == nullptr)
		return;

	InGameMenu = CreateWidget<UInGameMenu>(this, InGameMenuClass);
	if (InGameMenu == nullptr)
		return;

	InGameMenu->Setup();
	InGameMenu->SetMenuInterface(this);
}

void UPuzzlePlatformsGameInstance::Host(FString ServerName)
{
	DesiredServerName = ServerName;
	if (SessionInterface.IsValid())
	{
		auto ExistingSession = SessionInterface->GetNamedSession(SESSION_NAME);
		if (ExistingSession == nullptr)
		{
			CreateSession();
		}
		else
		{
			SessionInterface->DestroySession(SESSION_NAME);
		}
	}
}

void UPuzzlePlatformsGameInstance::Join(uint32 Index)
{
	if (SessionInterface.IsValid() == false)
		return;
	if (SessionSearch.IsValid() == false)
		return;
	if (IsValid(Menu) == false)
		return;

	SessionInterface->JoinSession(0, SESSION_NAME, SessionSearch->SearchResults[Index]);
}

void UPuzzlePlatformsGameInstance::LoadMainMenu()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
		return;

	PlayerController->ClientTravel("/Game/MenuSystem/MainMenu", ETravelType::TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	if (SessionSearch.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Starting Find Session"));
		SessionSearch->bIsLanQuery = false;
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
				
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UPuzzlePlatformsGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (Success == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not create session"));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, TEXT("Hosting"));

	UWorld* World = GetWorld();
	if (World != nullptr)
	{
		World->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
	}
}

void UPuzzlePlatformsGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	if (Success == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not destroy session"));
		return;
	}
	
	CreateSession();
}

void UPuzzlePlatformsGameInstance::OnFindSessionComplete(bool Success)
{
	if (Success && SessionSearch.IsValid() && Menu != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Finished Find Session"));

		TArray<FServerData> ServerNames;
		for (const  FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session names : %s"), *SearchResult.GetSessionIdStr());

			FServerData Data;
			Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
			Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections; // NumOpenPublicConnections는 사용 가능한 (비어있는) 연결 수 
			Data.HostUserName = SearchResult.Session.OwningUserName;
			FString ServerName;
			if (SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, ServerName))
			{
				Data.Name = ServerName;
			}
			else
			{
				Data.Name = "Could not find name.";
			}
			ServerNames.Add(Data);
		}

		Menu->SetServerList(ServerNames);
	}
}

void UPuzzlePlatformsGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid() == false)
		return;
	
	FString Address;
	if (SessionInterface->GetResolvedConnectString(SessionName, Address) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Could not get connect string."));
		return;
	}
	
	GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (PlayerController == nullptr)
		return;
	
	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
	if (SessionInterface.IsValid() == false)
		return;
	
	FOnlineSessionSettings SessionSettings;

	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		SessionSettings.bIsLANMatch = true;
	}
	else
	{
		SessionSettings.bIsLANMatch = false; // LAN매칭을 제외하고 세션 생성
	}
	
	SessionSettings.NumPublicConnections = 2; // 최대 2명의 플레이어가 접속 가능
	SessionSettings.bShouldAdvertise = true; // 세션 광고 여부. true이면 이 세션은 네트워크 상에서 검색 및 탐색이 가능해진다.
	SessionSettings.bUsesPresence = true; // 이 옵션이 true이면 로비 세션을 생성하고, false이면 인터넷 세션을 생성한다. 프레젠스를 사용하는 설정.
	SessionSettings.bUseLobbiesIfAvailable = true; // 온라인 서비스(스팀 등)에서 로비 기능이 지원되는 경우 이를 사용할지 여부.
	SessionSettings.bAllowJoinViaPresence = true; // 지역제한을 호스트의 지역으로 제한하는 옵션.
	SessionSettings.bAllowJoinInProgress = true; // 세션을 생성한 후 시작하기 전까지의 상태에서도 Join이 가능하도록 하는 옵션.
	SessionSettings.bIsDedicated = false; // 데디케이트 서버인지 여부

	SessionSettings.Set(SERVER_NAME_SETTINGS_KEY, DesiredServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	UE_LOG(LogTemp, Warning, TEXT("CreateSession"));
	
	SessionInterface->CreateSession(0, SESSION_NAME , SessionSettings);
}
