// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client ? *FString::Printf(TEXT("Client%d"), GPlayInEditorID) : GetNetMode() == ENetMode::NM_Standalone ? TEXT("Standalone") : TEXT("Server")))
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)
#define AB_LOG(LogCategory, Verbosity, Format, ...) UE_LOG(LogCategory, Verbosity, TEXT("[%s] %s %s"), LOG_NETMODEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogABNetwork, Log, All);
