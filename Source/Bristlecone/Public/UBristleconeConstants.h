// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Engine/PlatformSettings.h"
#include "Engine/DeveloperSettings.h"
#include "UBristleconeConstants.generated.h"
//https://www.tomlooman.com/unreal-engine-developer-settings/ Welp, I've done SOMETHING wrong.
UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Bristlecone Settings"))
class BRISTLECONE_API UBristleconeConstants : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta= (DisplayName = "Reflector IP"))
	FString default_address_c;

	UPROPERTY(EditAnywhere, Config, Category = "Bristlecone")
	bool log_receive_c;

};

