// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePlatformsGameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "InGameMenu.h"
#include "Blueprint/UserWidget.h"
#include "MenuSystem/MainMenu.h"

const static FName SESSION_NAME = TEXT("My Session Game");

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

void UPuzzlePlatformsGameInstance::Host()
{
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

void UPuzzlePlatformsGameInstance::Join(const FString& Address)
{
	if (IsValid(Menu) == false)
		return;

	Menu->SetServerList({"Test1", "Test2"});
	
	// GEngine->AddOnScreenDebugMessage(0, 5.f, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));
	//
	// APlayerController* PlayerController = GetFirstLocalPlayerController();
	// if (PlayerController == nullptr) return;
	//
	// PlayerController->ClientTravel(Address, TRAVEL_Absolute);
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
		SessionSearch->bIsLanQuery = true; 
				
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

		TArray<FString> ServerNames;
		for (const  FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found session names : %s"), *SearchResult.GetSessionIdStr());
			ServerNames.Add(SearchResult.GetSessionIdStr());
		}

		Menu->SetServerList(ServerNames);
	}
}

void UPuzzlePlatformsGameInstance::CreateSession()
{
	if (SessionInterface.IsValid() == false)
		return;
	
	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = true; // LAN에서만 탐색 가능
	SessionSettings.NumPublicConnections = 2; // 최대 2명의 플레이어가 접속 가능
	SessionSettings.bShouldAdvertise = true; // 세션 광고 여부. true이면 이 세션은 네트워크 상에서 검색 및 탐색이 가능해진다.
	SessionInterface->CreateSession(0, SESSION_NAME , SessionSettings);
}