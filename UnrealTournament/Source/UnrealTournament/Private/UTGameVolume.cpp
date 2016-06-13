// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTGameVolume.h"
#include "UTDmgType_Suicide.h"
#include "UTGameState.h"

AUTGameVolume::AUTGameVolume(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	VolumeName = FText::GetEmpty();
	TeamIndex = 255;
	bShowOnMinimap = true;
}

void AUTGameVolume::ActorEnteredVolume(class AActor* Other)
{
	if (Other && bIsTeamSafeVolume)
	{
		AUTCharacter* P = Cast<AUTCharacter>(Other);
		AUTGameState* GS = GetWorld()->GetGameState<AUTGameState>();
		if (P != nullptr && GS != nullptr && P->PlayerState != nullptr)
		{
			if (!GS->OnSameTeam(this, P))
			{
				P->TakeDamage(1000.f, FDamageEvent(UUTDmgType_Suicide::StaticClass()), nullptr, this);
			}
			else
			{
				P->bDamageHurtsHealth = false;
			}
		}
	}
}

void AUTGameVolume::ActorLeavingVolume(class AActor* Other)
{
	if (Other && bIsTeamSafeVolume)
	{
		AUTCharacter* P = Cast<AUTCharacter>(Other);
		if (P)
		{
			P->bDamageHurtsHealth = true;
		}
	}
}

void AUTGameVolume::SetTeamForSideSwap_Implementation(uint8 NewTeamNum)
{
	TeamIndex = NewTeamNum;
}





