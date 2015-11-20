// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#pragma once
#if ENABLE_VISUAL_LOG
#include "VisualLogger/VisualLogger.h"
#endif
#include "VisualLoggerExtension.generated.h"

class UWorld;
class AActor;

namespace EVisLogTags
{
	const FString TAG_EQS = TEXT("LogEQS");
}

#if ENABLE_VISUAL_LOG
struct FVisualLogDataBlock;
struct FLogEntryItem;
class UCanvas;

class FVisualLoggerExtension : public FVisualLogExtensionInterface
{
public:
	FVisualLoggerExtension();

	virtual void ResetData(IVisualLoggerEditorInterface* EdInterface) override;
	virtual void DrawData(IVisualLoggerEditorInterface* EdInterface, UCanvas* Canvas) override;
	virtual void OnItemsSelectionChanged(IVisualLoggerEditorInterface* EdInterface) override;
	virtual void OnLogLineSelectionChanged(IVisualLoggerEditorInterface* EdInterface, TSharedPtr<struct FLogEntryItem> SelectedItem, int64 UserData) override;

private:
	void DrawData(UWorld* InWorld, class UEQSRenderingComponent* EQSRenderingComponent, UCanvas* Canvas, AActor* HelperActor, const FName& TagName, const FVisualLogDataBlock& DataBlock, float Timestamp);
	void DisableEQSRendering(AActor* HelperActor);

protected:
	uint32 SelectedEQSId;
	float CurrentTimestamp;
	TArray<TWeakObjectPtr<class UEQSRenderingComponent> >	EQSRenderingComponents;
};
#endif //ENABLE_VISUAL_LOG

UCLASS(Abstract)
class AIMODULE_API UVisualLoggerExtension : public UObject
{
	GENERATED_BODY()
};
