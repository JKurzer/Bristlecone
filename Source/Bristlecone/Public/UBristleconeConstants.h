// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "UBristleconeConstants.generated.h"

UCLASS(Config = Game)
class UBristleconeConstants : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(Config)
	FString default_address;
};

