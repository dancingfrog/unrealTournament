// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTHUDWidget_SpectatorSlideOut.h"

UUTHUDWidget_SpectatorSlideOut::UUTHUDWidget_SpectatorSlideOut(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DesignedResolution = 1080;
	Position = FVector2D(0, 0);
	Size = FVector2D(500.0f, 108.0f);
	ScreenPosition = FVector2D(0.0f, 0.1f);
	Origin = FVector2D(0.0f, 0.0f);

	FlagX = 0.09;
	ColumnHeaderPlayerX = 0.16;
	ColumnHeaderScoreX = 0.7;
	ColumnHeaderArmor = 0.98f;
	ColumnY = 12;

	CellHeight = 40;
	SlideIn = 0.f;
	CenterBuffer = 10.f;
	SlideSpeed = 6.f;

	static ConstructorHelpers::FObjectFinder<UTexture2D> Tex(TEXT("Texture2D'/Game/RestrictedAssets/UI/Textures/UTScoreboard01.UTScoreboard01'"));
	TextureAtlas = Tex.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> FlagTex(TEXT("Texture2D'/Game/RestrictedAssets/UI/Textures/CountryFlags.CountryFlags'"));
	FlagAtlas = FlagTex.Object;
}

bool UUTHUDWidget_SpectatorSlideOut::ShouldDraw_Implementation(bool bShowScores)
{
	if (!bShowScores && UTHUDOwner->UTPlayerOwner && UTHUDOwner->UTPlayerOwner->UTPlayerState && UTGameState && UTHUDOwner->UTPlayerOwner->UTPlayerState->bOnlySpectator)
	{
		if (UTGameState->HasMatchEnded() || !UTGameState->HasMatchStarted() || UTGameState->IsMatchAtHalftime())
		{
			return false;
		}
		return (UTHUDOwner->UTPlayerOwner->bRequestingSlideOut || (SlideIn > 0.f));
	}
	return false;
}

void UUTHUDWidget_SpectatorSlideOut::Draw_Implementation(float DeltaTime)
{
	Super::Draw_Implementation(DeltaTime);

	if (TextureAtlas && UTGameState)
	{
		SlideIn = UTHUDOwner->UTPlayerOwner->bRequestingSlideOut ? FMath::Min(Size.X, SlideIn + DeltaTime*Size.X*SlideSpeed) : FMath::Max(0.f, SlideIn - DeltaTime*Size.X*SlideSpeed);

		int32 Place = 1;
		int32 MaxRedPlaces = UTGameState->bTeamGame ? 5 : 10;
		int32 XOffset = SlideIn - Size.X;
		float DrawOffset = 0.f;
		UTGameState->FillPlayerLists();
		for (int32 i = 0; i<UTGameState->RedPlayerList.Num(); i++)
		{
			AUTPlayerState* PlayerState = Cast<AUTPlayerState>(UTGameState->RedPlayerList[i]);
			if (PlayerState)
			{
				DrawPlayer(Place, PlayerState, DeltaTime, XOffset, DrawOffset);
				DrawOffset += CellHeight;
				Place++;
				if (Place > MaxRedPlaces)
				{
					break;
				}
			}
		}
		if (UTGameState->bTeamGame)
		{
			Place = MaxRedPlaces + 1;
			for (int32 i = 0; i<UTGameState->BluePlayerList.Num(); i++)
			{
				AUTPlayerState* PlayerState = Cast<AUTPlayerState>(UTGameState->BluePlayerList[i]);
				if (PlayerState)
				{
					DrawPlayer(Place, PlayerState, DeltaTime, XOffset, DrawOffset);
					DrawOffset += CellHeight;
					Place++;
					if (Place > 10)
					{
						break;
					}
				}
			}
		}
	}
}

void UUTHUDWidget_SpectatorSlideOut::DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset)
{
	if (PlayerState == NULL) return;

	FLinearColor DrawColor = FLinearColor::White;
	float BarOpacity = 0.3f;
	float Width = Size.X;

	FText Position = FText::Format(NSLOCTEXT("UTScoreboard", "PositionFormatText", "{0}."), FText::AsNumber(Index));
	FText PlayerName = FText::FromString(GetClampedName(PlayerState, UTHUDOwner->MediumFont, 1.f, 0.5f*Width));
	FText PlayerScore = FText::AsNumber(int32(PlayerState->Score));

	// Draw the background border.
	FLinearColor BarColor = FLinearColor::Black;
	if (PlayerState->Team)
	{
		BarColor = (PlayerState->Team->TeamIndex == 0) ? FLinearColor::Red : FLinearColor::Blue;
	}
	float FinalBarOpacity = BarOpacity;

	// FIXME Add Flag, U damage
	AUTCharacter* Character = PlayerState->GetUTCharacter();
	if (Character && (Character->Health > 0))
	{
		float LastActionTime = GetWorld()->GetTimeSeconds() - FMath::Max(Character->LastTakeHitTime, Character->LastWeaponFireTime);

		if (LastActionTime < 2.f)
		{
			float Blend = 1.f - 0.5f * LastActionTime;
			BarColor.R = BarColor.R + (1.f - BarColor.R)*Blend;
			BarColor.G = BarColor.G + (1.f - BarColor.G)*Blend;
			BarColor.B = BarColor.B + (1.f - BarColor.B)*Blend;
			FinalBarOpacity = 0.75f;
		}
	}
	DrawTexture(TextureAtlas, XOffset, YOffset, Width, 36, 149, 138, 32, 32, FinalBarOpacity, BarColor);	

	int32 FlagU = (PlayerState->CountryFlag % 8) * 32;
	int32 FlagV = (PlayerState->CountryFlag / 8) * 24;

	DrawTexture(FlagAtlas, XOffset + (Width * FlagX), YOffset + 18, 32, 24, FlagU, FlagV, 32, 24, 1.0, FLinearColor::White, FVector2D(0.0f, 0.5f));	

	// Draw the Text
	DrawText(Position, XOffset + 4.f, YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Left, ETextVertPos::Center);
	FVector2D NameSize = DrawText(PlayerName, XOffset + (Width * ColumnHeaderPlayerX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Left, ETextVertPos::Center);

	if (UTGameState && UTGameState->HasMatchStarted() && Character)
	{
		FFormatNamedArguments Args;
		Args.Add("Health", FText::AsNumber(Character->Health));
		DrawColor = FLinearColor::Green;
		DrawText(FText::Format(NSLOCTEXT("UTCharacter", "HealthDisplay", "+{Health}"), Args), XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);

		if (Character->ArmorAmount > 0)
		{
			FFormatNamedArguments Args;
			Args.Add("Armor", FText::AsNumber(Character->ArmorAmount));
			DrawColor = FLinearColor::Yellow;
			DrawText(FText::Format(NSLOCTEXT("UTCharacter", "ArmorDisplay", "A{Armor}"), Args), XOffset + (Width * ColumnHeaderArmor), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Right, ETextVertPos::Center);
		}
	}
}
