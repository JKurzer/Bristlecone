// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "UBristleconeConstants.generated.h"

UCLASS(config=Game)
class UBristleconeConstants : public UObject
{
	GENERATED_BODY()
public:
	//TODO: rework this to be a bit less janky? Maybe pick a good config and hierarchy?
	UPROPERTY(Config)
	FString default_address;


};

