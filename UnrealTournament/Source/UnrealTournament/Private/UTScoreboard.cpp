// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTScoreboard.h"
#include "UTHUDWidget_Spectator.h"
#include "StatNames.h"
#include "UTPickupWeapon.h"
#include "UTWeapon.h"
#include "UTWeap_Enforcer.h"
#include "UTWeap_ImpactHammer.h"

UUTScoreboard::UUTScoreboard(const class FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	DesignedResolution = 1080;
	Position = FVector2D(0, 0);
	Size = FVector2D(1440.0f, 1080.0f);
	ScreenPosition = FVector2D(0.5f, 0.5f);
	Origin = FVector2D(0.5f, 0.5f);

	NumPages = 2;
	ColumnHeaderPlayerX = 0.125;
	ColumnHeaderScoreX = 0.7;
	ColumnHeaderDeathsX = 0.84;
	ColumnHeaderPingX = 0.96;
	ColumnHeaderY = 8;
	ColumnY = 12;
	ColumnMedalX = 0.6;
	FooterPosY = 1020.f;
	CellHeight = 40;

	FlagX = 0.065;

	BadgeNumberUVs.Add(FVector2D(248,183));
	BadgeNumberUVs.Add(FVector2D(283,183));
	BadgeNumberUVs.Add(FVector2D(318,183));
	BadgeNumberUVs.Add(FVector2D(353,183));
	BadgeNumberUVs.Add(FVector2D(388,183));
	BadgeNumberUVs.Add(FVector2D(423,183));
	BadgeNumberUVs.Add(FVector2D(458,183));
	BadgeNumberUVs.Add(FVector2D(248,219));
	BadgeNumberUVs.Add(FVector2D(283,219));

	BadgeUVs.Add(FVector2D(388,219));
	BadgeUVs.Add(FVector2D(353,219));
	BadgeUVs.Add(FVector2D(318,219));

	CenterBuffer = 10;

	static ConstructorHelpers::FObjectFinder<UTexture2D> Tex(TEXT("Texture2D'/Game/RestrictedAssets/UI/Textures/UTScoreboard01.UTScoreboard01'"));
	TextureAtlas = Tex.Object;

	ValueColumn = 0.5f;
	ScoreColumn = 0.75f;
	bHighlightStatsLineTopValue = false;
	BestWeaponIndex = -1;
}

void UUTScoreboard::AdvancePage(int32 Increment)
{
	// @TODO FIXMESTEVE hack for progressing through players for scoring breakdown
	if ((NumPages > 1) && (UTHUDOwner->ScoreboardPage == NumPages-1) && UTHUDOwner->UTPlayerOwner && (GetWorld()->GameState->PlayerArray.Num() > 0) 
		&& ((Increment > 0) || (UTHUDOwner->UTPlayerOwner->CurrentlyViewedScorePS != GetWorld()->GameState->PlayerArray[0])))
	{
		UTHUDOwner->UTPlayerOwner->SetViewedScorePS(GetNextScoringPlayer(Increment), UTPlayerOwner->CurrentlyViewedStatsTab);
		return;
	}
	UTHUDOwner->ScoreboardPage = uint32(FMath::Clamp<int32>(int32(UTHUDOwner->ScoreboardPage) + Increment, 0, NumPages - 1));
	BestWeaponIndex = -1;
	PageChanged();
}

void UUTScoreboard::SetScoringPlaysTimer(bool bEnableTimer)
{
}

void UUTScoreboard::OpenScoringPlaysPage()
{
}

AUTPlayerState* UUTScoreboard::GetNextScoringPlayer(int32 dir)
{
	int32 CurrentIndex = -1;
	AUTPlayerState* ScoreBreakdownPS = UTHUDOwner->UTPlayerOwner->CurrentlyViewedScorePS;
	if (!ScoreBreakdownPS)
	{
		ScoreBreakdownPS = UTHUDOwner->GetScorerPlayerState();
	}
	// Find index of current viewtarget's PlayerState
	for (int32 i = 0; i<GetWorld()->GameState->PlayerArray.Num(); i++)
	{
		if (ScoreBreakdownPS == GetWorld()->GameState->PlayerArray[i])
		{
			CurrentIndex = i;
			break;
		}
	}

	int32 NewIndex;
	for (NewIndex = CurrentIndex + dir; (NewIndex >= 0) && (NewIndex<GetWorld()->GameState->PlayerArray.Num()); NewIndex = NewIndex + dir)
	{
		AUTPlayerState* const PlayerState = Cast<AUTPlayerState>(GetWorld()->GameState->PlayerArray[NewIndex]);
		if (PlayerState != NULL)
		{
			return PlayerState;
		}
	}

	// wrap around
	CurrentIndex = (NewIndex < 0) ? GetWorld()->GameState->PlayerArray.Num() : -1;
	for (NewIndex = CurrentIndex + dir; (NewIndex >= 0) && (NewIndex<GetWorld()->GameState->PlayerArray.Num()); NewIndex = NewIndex + dir)
	{
		AUTPlayerState* const PlayerState = Cast<AUTPlayerState>(GetWorld()->GameState->PlayerArray[NewIndex]);
		if (PlayerState != NULL)
		{
			return PlayerState;
		}
	}
	return ScoreBreakdownPS;
}

void UUTScoreboard::SetPage(int32 NewPage)
{
	UTHUDOwner->ScoreboardPage = FMath::Min<int32>(NewPage, NumPages - 1);
	PageChanged();
}

void UUTScoreboard::PageChanged_Implementation()
{
}

void UUTScoreboard::PreDraw(float DeltaTime, AUTHUD* InUTHUDOwner, UCanvas* InCanvas, FVector2D InCanvasCenter)
{
	Super::PreDraw(DeltaTime, InUTHUDOwner, InCanvas, InCanvasCenter);

	ActualPlayerCount=0;
	if (UTGameState)
	{
		for (int32 i = 0; i < UTGameState->PlayerArray.Num(); i++)
		{
			if (UTGameState->PlayerArray[i] && !UTGameState->PlayerArray[i]->bOnlySpectator)
			{
				ActualPlayerCount++;
			}
		}
	}
}

void UUTScoreboard::Draw_Implementation(float RenderDelta)
{
	Super::Draw_Implementation(RenderDelta);

	float YOffset = 0.0f;
	DrawGamePanel(RenderDelta, YOffset);
	if (UTHUDOwner->ScoreboardPage > 0)
	{
		DrawScoringStats(RenderDelta, YOffset);
	}
	else
	{
		DrawTeamPanel(RenderDelta, YOffset);
		DrawScorePanel(RenderDelta, YOffset);
	}
	DrawServerPanel(RenderDelta, FooterPosY);
}

void UUTScoreboard::DrawGamePanel(float RenderDelta, float& YOffset)
{
	// Draw the Background
	DrawTexture(TextureAtlas,0,0, Size.X, 128, 4,2,124, 128, 1.0);

	// Draw the Logo
	DrawTexture(TextureAtlas, 165, 60, 301, 98, 162,14,301, 98.0, 1.0f, FLinearColor::White, FVector2D(0.5, 0.5));
	
	// Draw the Spacer Bar
	DrawTexture(TextureAtlas, 325, 60, 4, 99, 488,13,4,99, 1.0f, FLinearColor::White, FVector2D(0.0, 0.5));
	FText MapName = UTHUDOwner ? FText::FromString(UTHUDOwner->GetWorld()->GetMapName().ToUpper()) : FText::GetEmpty();
	FText GameName = FText::GetEmpty();
	if (UTGameState && UTGameState->GameModeClass)
	{
		AUTGameMode* DefaultGame = UTGameState->GameModeClass->GetDefaultObject<AUTGameMode>();
		if (DefaultGame) 
		{
			GameName = FText::FromString(DefaultGame->DisplayName.ToString().ToUpper());
		}
	}
	if (!GameName.IsEmpty())
	{
		DrawText(GameName, 355, 28, UTHUDOwner->LargeFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Left, ETextVertPos::Center); // 470
	}
	if (!MapName.IsEmpty())
	{
		DrawText(MapName, 355, 80, UTHUDOwner->LargeFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Left, ETextVertPos::Center); // 470
	}

	DrawGameOptions(RenderDelta, YOffset);
	YOffset += 128;	// The size of this zone.
}

void UUTScoreboard::DrawGameOptions(float RenderDelta, float& YOffset)
{
	if (UTGameState)
	{
		if (UTGameState->GoalScore > 0)
		{
			// Draw Game Text
			FText Score = FText::Format(NSLOCTEXT("UTScoreboard","GoalScoreFormat","First to {0} Frags"), FText::AsNumber(UTGameState->GoalScore));
			DrawText(Score, Size.X * 0.985, 28, UTHUDOwner->MediumFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Right, ETextVertPos::Center);
		}

		FText StatusText = UTGameState->GetGameStatusText();
		if (!StatusText.IsEmpty())
		{
			DrawText(StatusText, Size.X * 0.985, 90, UTHUDOwner->MediumFont, 1.0, 1.0, FLinearColor::Yellow, ETextHorzPos::Right, ETextVertPos::Center);
		}
		else
		{
			float RemainingTime = UTGameState ? UTGameState->GetClockTime() : 0.f;
			FText Timer = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), RemainingTime, true, true, true);
			DrawText(Timer, Size.X * 0.985, 88, UTHUDOwner->NumberFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Right, ETextVertPos::Center);
		}
	}
}

void UUTScoreboard::DrawTeamPanel(float RenderDelta, float& YOffset)
{
	YOffset += 39.0; // A small gap
}

void UUTScoreboard::DrawScorePanel(float RenderDelta, float& YOffset)
{
	if (bIsInteractive)
	{
		SelectionStack.Empty();
	}
	if (UTGameState)
	{
		float DrawY = YOffset;
		DrawScoreHeaders(RenderDelta, DrawY);
		DrawPlayerScores(RenderDelta, DrawY);
	}
}

void UUTScoreboard::DrawScoreHeaders(float RenderDelta, float& YOffset)
{
	float Width = (Size.X * 0.5) - CenterBuffer;  
	float Height = 23;

	FText CH_PlayerName = NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerName", "PLAYER");
	FText CH_Score = NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerScore", "SCORE");
	FText CH_Deaths = NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerDeaths", "DEATHS");
	FText CH_Ping = (GetWorld()->GetNetMode() == NM_Standalone) ? NSLOCTEXT("UTScoreboard", "ColumnHeader_BotSkill", "SKILL") : NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerPing", "PING");
	FText CH_Ready = NSLOCTEXT("UTScoreboard", "ColumnHeader_Ready", "");

	int32 ColumnCnt = ((UTGameState && UTGameState->bTeamGame) || ActualPlayerCount > 16) ? 2 : 1;
	float XOffset = ColumnCnt > 1 ? 0 : (Size.X * 0.5) - (Width * 0.5);

	for (int32 i = 0; i < ColumnCnt; i++)
	{
		// Draw the background Border
		DrawTexture(TextureAtlas, XOffset, YOffset, Width, Height, 149, 138, 32, 32, 1.0, FLinearColor(0.72f, 0.72f, 0.72f, 0.85f));

		DrawText(CH_PlayerName, XOffset + (Width * ColumnHeaderPlayerX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Left, ETextVertPos::Center);

		if (UTGameState && UTGameState->HasMatchStarted())
		{
			DrawText(CH_Score, XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
			DrawText(CH_Deaths, XOffset + (Width * ColumnHeaderDeathsX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
		}
		else
		{
			DrawText(CH_Ready, XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
		}
		DrawText(CH_Ping, XOffset + (Width * ColumnHeaderPingX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);

		XOffset = Size.X - Width;
	}

	YOffset += Height + 4;
}

void UUTScoreboard::DrawPlayerScores(float RenderDelta, float& YOffset)
{
	if (!UTGameState)
	{
		return;
	}

	int32 Place = 1;
	int32 NumSpectators = 0;
	int32 XOffset = ActualPlayerCount > 16 ? 0 : (Size.X * 0.5) - ( ((Size.X * 0.5) - CenterBuffer) * 0.5);
	float DrawOffset = YOffset;
	for (int32 i=0; i<UTGameState->PlayerArray.Num(); i++)
	{
		AUTPlayerState* PlayerState = Cast<AUTPlayerState>(UTGameState->PlayerArray[i]);
		if (PlayerState)
		{
			if (!PlayerState->bOnlySpectator)
			{
				DrawPlayer(Place, PlayerState, RenderDelta, XOffset, DrawOffset);
				DrawOffset += CellHeight;
				Place++;
				if (Place == 17)
				{
					XOffset = Size.X - ((Size.X * 0.5) - CenterBuffer);
					DrawOffset = YOffset;
				}
			}
			else
			{
				NumSpectators++;
			}
		}
	}
	
	if (UTGameState->PlayerArray.Num() <= 28 && NumSpectators > 0)
	{
		FText SpectatorCount = (NumSpectators == 1) 
			? NSLOCTEXT("UTScoreboard", "OneSpectator", "1 spectator is watching this match")
			: FText::Format(NSLOCTEXT("UTScoreboard","SpectatorFormat","{0} spectators are watching this match"), FText::AsNumber(NumSpectators));
		DrawText(SpectatorCount, 635, 765, UTHUDOwner->SmallFont, 1.0f, 1.0f, FLinearColor(0.75f, 0.75f, 0.75f, 1.0f), ETextHorzPos::Center, ETextVertPos::Bottom);
	}
}

void UUTScoreboard::DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset)
{
	if (PlayerState == NULL) return;	// Safeguard

	FLinearColor DrawColor = FLinearColor::White;
	float BarOpacity = 0.3;
	float Width = (Size.X * 0.5f) - CenterBuffer;
	bool bIsUnderCursor = false;

	// If we are interactive, store off the bounds of this cell for selection
	if (bIsInteractive)
	{
		FVector4 Bounds = FVector4(RenderPosition.X + (XOffset * RenderScale), RenderPosition.Y + (YOffset * RenderScale), 
										RenderPosition.X + ((XOffset + Width) * RenderScale), RenderPosition.Y + ((YOffset + CellHeight) * RenderScale));
		SelectionStack.Add(FSelectionObject(PlayerState, Bounds));
		bIsUnderCursor = (CursorPosition.X >= Bounds.X && CursorPosition.X <= Bounds.Z && CursorPosition.Y >= Bounds.Y && CursorPosition.Y <= Bounds.W);
	}

	FText Position = FText::Format(NSLOCTEXT("UTScoreboard", "PositionFormatText", "{0}."), FText::AsNumber(Index));
	FText PlayerName = FText::FromString(GetClampedName(PlayerState, UTHUDOwner->MediumFont, 1.f, 0.475f*Width));
	FText PlayerScore = FText::AsNumber(int32(PlayerState->Score));
	FText PlayerDeaths = FText::AsNumber(PlayerState->Deaths);

	int32 Ping = PlayerState->Ping * 4;
	if (UTHUDOwner->UTPlayerOwner->UTPlayerState == PlayerState)
	{
		Ping = PlayerState->ExactPing;
		DrawColor = FLinearColor(0.0f,0.92f,1.0f,1.0f);
		BarOpacity = 0.5;
	}

	else if (PlayerState->bIsFriend)
	{
		DrawColor = FLinearColor(FColor(254, 255, 174, 255));
	}

	FText PlayerPing;
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		AUTBot* Bot = Cast<AUTBot>(PlayerState->GetOwner());
		PlayerPing = Bot ? FText::AsNumber(Bot->Skill) : FText::FromString(TEXT("-"));
	}
	else
	{
		PlayerPing = FText::Format(NSLOCTEXT("UTScoreboard", "PingFormatText", "{0}ms"), FText::AsNumber(Ping));
	}

	// Draw the position
	
	// Draw the background border.
	FLinearColor BarColor = FLinearColor::Black;
	float FinalBarOpacity = BarOpacity;
	if (bIsUnderCursor) 
	{
		BarColor = FLinearColor(0.0,0.3,0.0,1.0);
		FinalBarOpacity = 0.75f;
	}
	if (PlayerState == SelectedPlayer) 
	{
		BarColor = FLinearColor(0.0,0.3,0.3,1.0);
		FinalBarOpacity = 0.75f;
	}

	if (PlayerState->KickPercent > 0)
	{
		FText Kick = FText::Format(NSLOCTEXT("Common","PercFormat","{0}%"),FText::AsNumber(PlayerState->KickPercent));
		DrawText(Kick, XOffset, YOffset + ColumnY, UTHUDOwner->TinyFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Left, ETextVertPos::Center);
	}

	DrawTexture(TextureAtlas, XOffset, YOffset, Width, 36, 149, 138, 32, 32, FinalBarOpacity, BarColor);	// NOTE: Once I make these interactable.. have a selection color too

	int32 FlagU = 0;
	int32 FlagV = 0;

	UTexture2D* NewFlagAtlas = UTHUDOwner->ResolveFlag(PlayerState->CountryFlag, FlagU, FlagV);
	DrawTexture(NewFlagAtlas, XOffset + (Width * FlagX), YOffset + 18, 36, 26, FlagU, FlagV, 36, 26, 1.0, FLinearColor::White, FVector2D(0.0f, 0.5f));	// Add a function to support additional flags

	// Draw the Text
	DrawText(Position, XOffset + (Width * FlagX - 5), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Right, ETextVertPos::Center);
	FVector2D NameSize = DrawText(PlayerName, XOffset + (Width * ColumnHeaderPlayerX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Left, ETextVertPos::Center);

	if (PlayerState->bIsFriend)
	{
		DrawTexture(TextureAtlas, XOffset + (Width * ColumnHeaderPlayerX) + NameSize.X + 5, YOffset + 18, 30, 24, 236, 136, 30, 24, 1.0, FLinearColor::White, FVector2D(0.0f, 0.5f));
	}

	if (UTGameState && UTGameState->HasMatchStarted())
	{
		DrawText(PlayerScore, XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
		DrawText(PlayerDeaths, XOffset + (Width * ColumnHeaderDeathsX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
	}
	else
	{
		PlayerState->UpdateReady();
		FText PlayerReady = PlayerState->bReadyToPlay ? NSLOCTEXT("UTScoreboard", "READY", "READY") : NSLOCTEXT("UTScoreboard", "NOTREADY", "");
		if (PlayerState->bPendingTeamSwitch)
		{
			PlayerReady = NSLOCTEXT("UTScoreboard", "TEAMSWITCH", "TEAM SWAP");
		}
		DrawText(PlayerReady, XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, PlayerState->ReadyColor, ETextHorzPos::Center, ETextVertPos::Center);
	}
	DrawText(PlayerPing, XOffset + (Width * ColumnHeaderPingX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);

	if (!PlayerState->bIsABot)
	{
		int32 Badge;
		int32 Level;

		UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(UTHUDOwner->UTPlayerOwner->Player);
		if (LP)
		{
			UUTLocalPlayer::GetBadgeFromELO(PlayerState->AverageRank, Badge, Level);
			Badge = FMath::Clamp<int32>(Badge,0, 3);
			Level = FMath::Clamp<int32>(Level,0, 8);
			DrawTexture(TextureAtlas, XOffset + (Width * ColumnMedalX), YOffset + 16, 32, 32, BadgeUVs[Badge].X, BadgeUVs[Badge].Y, 32, 32, 1.0, FLinearColor::White, FVector2D(0.5f, 0.5f));
			DrawTexture(TextureAtlas, XOffset + (Width * ColumnMedalX), YOffset + 16, 32, 32, BadgeNumberUVs[Level].X, BadgeNumberUVs[Level].Y, 32, 32, 1.0, FLinearColor::White, FVector2D(0.5f, 0.5f));
		}
	}
}

void UUTScoreboard::DrawServerPanel(float RenderDelta, float YOffset)
{
	if (UTGameState)
	{
		FText SpectatorMessage;
		bool bShortMessage = false;
		if (UTHUDOwner->SpectatorMessageWidget)
		{
			SpectatorMessage = UTHUDOwner->SpectatorMessageWidget->GetSpectatorMessageText(bShortMessage);
		}
		if (!SpectatorMessage.IsEmpty() && !bShortMessage && (UTGameState->PlayerArray.Num() < 26) && (UTHUDOwner->ScoreboardPage == 0))
		{
			// Only draw if there is room above spectator panel
			UTHUDOwner->SpectatorMessageWidget->PreDraw(RenderDelta, UTHUDOwner, Canvas, CanvasCenter);
			UTHUDOwner->SpectatorMessageWidget->DrawSimpleMessage(SpectatorMessage, RenderDelta, bShortMessage);
			return;
		}
		if (SpectatorMessage.IsEmpty() || bShortMessage)
		{
			SpectatorMessage = FText::FromString(UTGameState->ServerName);
		}

		// Draw the Background
		DrawTexture(TextureAtlas, 0, YOffset, 1269, 38, 4, 132, 30, 38, 1.0);
		DrawText(SpectatorMessage, 10, YOffset + 13, UTHUDOwner->SmallFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Left, ETextVertPos::Center);
		DrawText(FText::FromString(UTGameState->ServerDescription), 1259, YOffset + 13, UTHUDOwner->SmallFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Right, ETextVertPos::Center);
		if (NumPages > 1)
		{
			FText PageText = FText::Format(NSLOCTEXT("UTScoreboard", "Pages", "Arrow keys to switch page ({0} of {1})"), FText::AsNumber(UTHUDOwner->ScoreboardPage + 1), FText::AsNumber(NumPages + GetWorld()->GameState->PlayerArray.Num()));
			DrawText(PageText, Size.X * 0.5f, YOffset + 13, UTHUDOwner->SmallFont, 1.0f, 1.0f, FLinearColor::White, ETextHorzPos::Center, ETextVertPos::Center);
		}
	}
}

int32 UUTScoreboard::SelectionHitTest(FVector2D Position)
{
	if (bIsInteractive)
	{
		for (int32 i = 0; i < SelectionStack.Num(); i++)
		{
			if (Position.X >= SelectionStack[i].ScoreBounds.X && Position.X <= SelectionStack[i].ScoreBounds.Z &&
				  Position.Y >= SelectionStack[i].ScoreBounds.Y && Position.Y <= SelectionStack[i].ScoreBounds.W && SelectionStack[i].ScoreOwner.IsValid())
			{
				return i;
			}
		}
	}
	return -1;
}

void UUTScoreboard::TrackMouseMovement(FVector2D NewMousePosition)
{
	if (bIsInteractive)
	{
		CursorPosition = NewMousePosition;
	}
}

bool UUTScoreboard::AttemptSelection(FVector2D SelectionPosition)
{
	if (bIsInteractive)
	{
		int32 SelectionIndex = SelectionHitTest(SelectionPosition);
		if (SelectionIndex >=0 && SelectionIndex < SelectionStack.Num())
		{
			SelectedPlayer = SelectionStack[SelectionIndex].ScoreOwner;
			return true;
		}
	}
	return false;
}

void UUTScoreboard::ClearSelection()
{
	SelectedPlayer.Reset();
}

void UUTScoreboard::BecomeInteractive()
{
	bIsInteractive = true;
}

void UUTScoreboard::BecomeNonInteractive()
{
	bIsInteractive = false;
	ClearSelection();
}

void UUTScoreboard::DefaultSelection(AUTGameState* GS, uint8 TeamIndex)
{
	if (GS != NULL)
	{
		for (int32 i=0; i < GS->PlayerArray.Num(); i++)
		{
			AUTPlayerState* PS = Cast<AUTPlayerState>(GS->PlayerArray[i]);
			if (PS && !PS->bIsSpectator && !PS->bOnlySpectator && (TeamIndex == 255 || PS->GetTeamNum() == TeamIndex))
			{
				SelectedPlayer = PS;
				return;
			}
		}
	}
	SelectedPlayer.Reset();
}

void UUTScoreboard::SelectNext(int32 Offset, bool bDoNoWrap)
{
	AUTGameState* GS = UTHUDOwner->GetWorld()->GetGameState<AUTGameState>();
	if (GS == NULL) return;

	GS->SortPRIArray();
	int32 SelectedIndex = GS->PlayerArray.Find(SelectedPlayer.Get());
	
	if (SelectedIndex >= 0 && SelectedIndex < GS->PlayerArray.Num())
	{
		AUTPlayerState* Next = NULL;
		int32 Step = Offset > 0 ? 1 : -1;
		do 
		{
			SelectedIndex += Step;
			if (SelectedIndex < 0) 
			{
				if (bDoNoWrap) return;
				SelectedIndex = GS->PlayerArray.Num() -1;
			}
			if (SelectedIndex >= GS->PlayerArray.Num()) 
			{
				if (bDoNoWrap) return;
				SelectedIndex = 0;
			}

			Next = Cast<AUTPlayerState>(GS->PlayerArray[SelectedIndex]);
			if (Next && !Next->bOnlySpectator && !Next->bIsSpectator)
			{
				// Valid potential player.
				Offset -= Step;
				if (Offset == 0)
				{
					SelectedPlayer = Next;
					return;
				}
			}

		} while (Next != SelectedPlayer.Get());
	}
	else
	{
		DefaultSelection(GS);
	}

}

void UUTScoreboard::SelectionUp()
{
	SelectNext(-1);
}

void UUTScoreboard::SelectionDown()
{
	SelectNext(1);
}

void UUTScoreboard::SelectionLeft()
{
	SelectNext(-16,true);
}

void UUTScoreboard::SelectionRight()
{
	SelectNext(16,true);
}

void UUTScoreboard::SelectionClick()
{
	if (SelectedPlayer.IsValid())
	{
		UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(UTHUDOwner->UTPlayerOwner->Player);
		if (LP)
		{
			LP->ShowPlayerInfo(SelectedPlayer);
		}
	}
}

void UUTScoreboard::DrawStatsLine(FText StatsName, int32 StatValue, int32 ScoreValue, float DeltaTime, float XOffset, float& YPos, const FStatsFontInfo& StatsFontInfo, float ScoreWidth)
{
	Canvas->SetLinearDrawColor(FLinearColor::White);
	Canvas->DrawText(StatsFontInfo.TextFont, StatsName, XOffset, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);

	if (StatValue >= 0)
	{
		Canvas->SetLinearDrawColor((bHighlightStatsLineTopValue && (StatValue > ScoreValue)) ? FLinearColor::Yellow : FLinearColor::White);
		Canvas->DrawText(StatsFontInfo.TextFont, FString::Printf(TEXT(" %i"), StatValue), XOffset + ValueColumn*ScoreWidth, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	}
	if (ScoreValue >= 0)
	{
		Canvas->SetLinearDrawColor((bHighlightStatsLineTopValue && (ScoreValue > StatValue)) ? FLinearColor::Yellow : FLinearColor::White);
		Canvas->DrawText(StatsFontInfo.TextFont, FString::Printf(TEXT(" %i"), ScoreValue), XOffset + ScoreColumn*ScoreWidth, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	}
	YPos += StatsFontInfo.TextHeight;
}

void UUTScoreboard::DrawTextStatsLine(FText StatsName, FString StatValue, FString ScoreValue, float DeltaTime, float XOffset, float& YPos, const FStatsFontInfo& StatsFontInfo, float ScoreWidth, int32 HighlightIndex)
{
	Canvas->SetLinearDrawColor(FLinearColor::White);
	Canvas->DrawText(StatsFontInfo.TextFont, StatsName, XOffset, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);

	if (!StatValue.IsEmpty())
	{
		Canvas->SetLinearDrawColor((HighlightIndex & 1) ? FLinearColor::Yellow : FLinearColor::White);
		Canvas->DrawText(StatsFontInfo.TextFont, StatValue, XOffset + ValueColumn*ScoreWidth, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	}
	if (!ScoreValue.IsEmpty())
	{
		Canvas->SetLinearDrawColor((HighlightIndex & 2) ? FLinearColor::Yellow : FLinearColor::White);
		Canvas->DrawText(StatsFontInfo.TextFont, ScoreValue, XOffset + ScoreColumn*ScoreWidth, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	}
	YPos += StatsFontInfo.TextHeight;
}

void UUTScoreboard::DrawWeaponStatsLine(FText StatsName, int32 StatValue, int32 ScoreValue, float DeltaTime, float XOffset, float& YPos, const FStatsFontInfo& StatsFontInfo, float ScoreWidth, bool bIsBestWeapon)
{
	Canvas->SetLinearDrawColor(bIsBestWeapon ? FLinearColor::Yellow : FLinearColor::White);
	Canvas->DrawText(StatsFontInfo.TextFont, StatsName, XOffset, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);

	if (StatValue >= 0)
	{
		Canvas->SetLinearDrawColor((StatValue >= 15) ? FLinearColor::Yellow : FLinearColor::White);
		Canvas->DrawText(StatsFontInfo.TextFont, FString::Printf(TEXT(" %i"), StatValue), XOffset + ValueColumn*ScoreWidth, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	}
	if (ScoreValue >= 0)
	{
		Canvas->SetLinearDrawColor(FLinearColor::White);
		Canvas->DrawText(StatsFontInfo.TextFont, FString::Printf(TEXT(" %i"), ScoreValue), XOffset + ScoreColumn*ScoreWidth, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	}
	YPos += StatsFontInfo.TextHeight;
}

void UUTScoreboard::DrawWeaponStats(AUTPlayerState* PS, float DeltaTime, float& YPos, float XOffset, float ScoreWidth, float MaxHeight, const FStatsFontInfo& StatsFontInfo)
{
	DrawTextStatsLine(NSLOCTEXT("UTScoreboard", "WeaponColumnTitle", "Weapon"), "Kills With", "Deaths By", DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, 0);

	/** List of weapons to display stats for. */
	if (StatsWeapons.Num() == 0)
	{
		// add default weapons - needs to be automated
		StatsWeapons.AddUnique(AUTWeap_ImpactHammer::StaticClass()->GetDefaultObject<AUTWeapon>());
		StatsWeapons.AddUnique(AUTWeap_Enforcer::StaticClass()->GetDefaultObject<AUTWeapon>());

		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AUTPickupWeapon* Pickup = Cast<AUTPickupWeapon>(*It);
			if (Pickup && Pickup->GetInventoryType())
			{
				StatsWeapons.AddUnique(Pickup->GetInventoryType()->GetDefaultObject<AUTWeapon>());
			}
		}
	}

	float BestWeaponKills = (BestWeaponIndex == FMath::Clamp(BestWeaponIndex, 0, StatsWeapons.Num() - 1)) ? StatsWeapons[BestWeaponIndex]->GetWeaponKillStats(PS) : 0;
	for (int32 i = 0; i < StatsWeapons.Num(); i++)
	{
		int32 Kills = StatsWeapons[i]->GetWeaponKillStats(PS);
		DrawWeaponStatsLine(StatsWeapons[i]->DisplayName, Kills, StatsWeapons[i]->GetWeaponDeathStats(PS), DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, (i == BestWeaponIndex));
		if (Kills > BestWeaponKills)
		{
			BestWeaponKills = Kills;
			BestWeaponIndex = i;
		}
	}

	Canvas->DrawText(StatsFontInfo.TextFont, "----------------------------------------------------------------", XOffset, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	YPos += StatsFontInfo.TextHeight;

	DrawStatsLine(NSLOCTEXT("UTScoreboard", "ShockComboKills", "Shock Combo Kills"), PS->GetStatsValue(NAME_ShockComboKills), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "AmazingCombos", "Amazing Combos"), PS->GetStatsValue(NAME_AmazingCombos), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "HeadShots", "Sniper Headshots"), PS->GetStatsValue(NAME_SniperHeadshotKills), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "AirRox", "Air Rocket Kills"), PS->GetStatsValue(NAME_AirRox), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	// FIXMESTEVE track air rox and amazing combos

	Canvas->DrawText(StatsFontInfo.TextFont, "----------------------------------------------------------------", XOffset, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	YPos += StatsFontInfo.TextHeight;

	float BestComboRating = 0.01f*PS->GetStatsValue(NAME_BestShockCombo);
	DrawTextStatsLine(NSLOCTEXT("UTScoreboard", "ShockComboRating", "Best Shock Combo Rating"), FString::Printf(TEXT(" %4.1f"), BestComboRating), "", DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, (BestComboRating > 8.f));
}

void UUTScoreboard::DrawScoringStats(float DeltaTime, float& YPos)
{
	FVector2D SavedRenderPosition = RenderPosition;
	RenderPosition = FVector2D(0.f, 0.f);
	bScaleByDesignedResolution = false;
	float TopYPos = YPos;

	// draw left side
	float XOffset = Canvas->ClipX * 0.06f;
	float ScoreWidth = 0.5f * (Canvas->ClipX - 3.f*XOffset);
	float MaxHeight = FooterPosY + SavedRenderPosition.Y - YPos;
	float PageBottom = TopYPos + MaxHeight;

	FLinearColor PageColor = FLinearColor::Black;
	PageColor.A = 0.5f;
	DrawTexture(TextureAtlas, XOffset - 0.05f*ScoreWidth, YPos, 1.1f*ScoreWidth, MaxHeight, 149, 138, 32, 32, 0.5f, PageColor);
	DrawStatsLeft(DeltaTime, YPos, XOffset, ScoreWidth, PageBottom);

	// draw right side
	XOffset = ScoreWidth + 2.f*XOffset;
	YPos = TopYPos;
	DrawTexture(TextureAtlas, XOffset - 0.05f*ScoreWidth, YPos, 1.1f*ScoreWidth, MaxHeight, 149, 138, 32, 32, 0.5f, PageColor);
	DrawStatsRight(DeltaTime, YPos, XOffset, ScoreWidth, PageBottom);

	bScaleByDesignedResolution = true;
	RenderPosition = SavedRenderPosition;
}

void UUTScoreboard::DrawStatsLeft(float DeltaTime, float& YPos, float XOffset, float ScoreWidth, float PageBottom)
{

}

void UUTScoreboard::DrawStatsRight(float DeltaTime, float& YPos, float XOffset, float ScoreWidth, float PageBottom)
{
	DrawScoreBreakdown(DeltaTime, YPos, XOffset, ScoreWidth, PageBottom);
}

void UUTScoreboard::DrawScoreBreakdown(float DeltaTime, float& YPos, float XOffset, float ScoreWidth, float PageBottom)
{
	Canvas->SetLinearDrawColor(FLinearColor::White);
	FStatsFontInfo StatsFontInfo;
	StatsFontInfo.TextRenderInfo.bEnableShadow = true;
	StatsFontInfo.TextRenderInfo.bClipText = true;
	StatsFontInfo.TextFont = UTHUDOwner->SmallFont;
	bHighlightStatsLineTopValue = false;

	if (!UTPlayerOwner->CurrentlyViewedScorePS)
	{
		UTPlayerOwner->SetViewedScorePS(UTHUDOwner->GetScorerPlayerState(), UTPlayerOwner->CurrentlyViewedStatsTab);
	}
	AUTPlayerState* PS = UTPlayerOwner->CurrentlyViewedScorePS;
	if (!PS)
	{
		return;
	}

	FFormatNamedArguments Args;
	Args.Add(TEXT("PlayerName"), FText::FromString(PS->PlayerName));
	FText CombinedHeader = (UTHUDOwner && UTHUDOwner->UTPlayerOwner && UTHUDOwner->UTPlayerOwner->CurrentlyViewedStatsTab == 0)
		? FText::Format(NSLOCTEXT("UTScoreboard", "ScoringBreakDownHeader", "{PlayerName} Scoring Breakdown"), Args)
		: FText::Format(NSLOCTEXT("UTScoreboard", "WeaponStatsHeader", "{PlayerName} Weapon Stats"), Args);
	float XL, SmallYL;
	Canvas->TextSize(UTHUDOwner->SmallFont, "TEST", XL, SmallYL, RenderScale, RenderScale);
	StatsFontInfo.TextHeight = SmallYL;
	float MedYL;
	Canvas->TextSize(UTHUDOwner->MediumFont, CombinedHeader.ToString(), XL, MedYL, RenderScale, RenderScale);

	if (PS->Team)
	{
		// draw team icon
		float IconHeight = MedYL;
		int32 TeamIndex = PS->Team->TeamIndex;
		DrawTexture(UTHUDOwner->HUDAtlas, XOffset, YPos, IconHeight, IconHeight, UTHUDOwner->TeamIconUV[TeamIndex].X, UTHUDOwner->TeamIconUV[TeamIndex].Y, 72, 72, 1.f, PS->Team->TeamColor);
	}

	Canvas->DrawText(UTHUDOwner->MediumFont, CombinedHeader, XOffset + 0.5f*(ScoreWidth - XL), YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	YPos += MedYL;

	DrawPlayerStats(PS, DeltaTime, YPos, XOffset, ScoreWidth, PageBottom, StatsFontInfo);

	FString TabInstruction = (UTHUDOwner && UTHUDOwner->UTPlayerOwner && UTHUDOwner->UTPlayerOwner->CurrentlyViewedStatsTab == 0)
		? "Press Down Arrow to View Weapon Stats"
		: "Press Up Arrow to View Game Stats";

	Canvas->DrawText(UTHUDOwner->SmallFont, TabInstruction, XOffset, PageBottom - StatsFontInfo.TextHeight, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
}

void UUTScoreboard::DrawPlayerStats(AUTPlayerState* PS, float DeltaTime, float& YPos, float XOffset, float ScoreWidth, float PageBottom, const FStatsFontInfo& StatsFontInfo)
{
	if (UTHUDOwner && UTHUDOwner->UTPlayerOwner && UTHUDOwner->UTPlayerOwner->CurrentlyViewedStatsTab == 1)
	{
		DrawWeaponStats(PS, DeltaTime, YPos, XOffset, ScoreWidth, PageBottom, StatsFontInfo);
		return;
	}
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "Kills", "Kills"), PS->Kills, PS->Kills, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "Deaths", "Deaths"), PS->Deaths, -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "Suicides", "Suicides"), PS->GetStatsValue(NAME_Suicides), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawTextStatsLine(NSLOCTEXT("UTScoreboard", "KDRatio", "K/D Ratio"), FString::Printf(TEXT(" %6.2f"), ((PS->Deaths > 0) ? float(PS->Kills) / PS->Deaths : 0.f)), "", DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, 0);
	Canvas->DrawText(UTHUDOwner->SmallFont, "----------------------------------------------------------------", XOffset, YPos, RenderScale, RenderScale, StatsFontInfo.TextRenderInfo);
	YPos += StatsFontInfo.TextHeight;

	DrawStatsLine(NSLOCTEXT("UTScoreboard", "BeltPickups", "Shield Belt Pickups"), PS->GetStatsValue(NAME_ShieldBeltCount), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "VestPickups", "Armor Vest Pickups"), PS->GetStatsValue(NAME_ArmorVestCount), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "PadPickups", "Thigh Pad Pickups"), PS->GetStatsValue(NAME_ArmorPadsCount), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "HelmetPickups", "Helmet Pickups"), PS->GetStatsValue(NAME_HelmetCount), -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);

	int32 ClockVal = PS->GetStatsValue(NAME_UDamageTime);
	if (ClockVal > 0)
	{
		FText ClockString = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), ClockVal, false);
		DrawTextStatsLine(NSLOCTEXT("UTScoreboard", "UDamage", "UDamage Control"), ClockString.ToString(), "", DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, 0);
	}
	ClockVal = PS->GetStatsValue(NAME_BerserkTime);
	if (ClockVal > 0)
	{
		FText ClockString = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), ClockVal, false);
		DrawTextStatsLine(NSLOCTEXT("UTScoreboard", "Berserk", "Berserk Control"), ClockString.ToString(), "", DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, 0);
	}
	ClockVal = PS->GetStatsValue(NAME_InvisibilityTime);
	if (ClockVal > 0)
	{
		FText ClockString = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), ClockVal, false);
		DrawTextStatsLine(NSLOCTEXT("UTScoreboard", "Invisibility", "Invisibility Control"), ClockString.ToString(), "", DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth, 0);
	}

	int32 BootJumps = PS->GetStatsValue(NAME_BootJumps);
	if (BootJumps != 0)
	{
		DrawStatsLine(NSLOCTEXT("UTScoreboard", "JumpBootJumps", "JumpBoot Jumps"), BootJumps, -1, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
	}
	DrawStatsLine(NSLOCTEXT("UTScoreboard", "Scoring", "SCORE"), -1, PS->Score, DeltaTime, XOffset, YPos, StatsFontInfo, ScoreWidth);
}