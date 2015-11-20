// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MovieSceneTrack.h"
#include "MovieSceneSection.h"
#include "ISequencerSection.h"


/**
 * A generic implementation for displaying simple property sections
 */
class FPropertySection
	: public ISequencerSection
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param The section's human readable display name.
	 */
	FPropertySection(UMovieSceneSection& InSectionObject, const FText& InDisplayName)
		: DisplayName(InDisplayName)
		, SectionObject(InSectionObject)
	{ }

public:

	// ISequencerSection interface

	virtual UMovieSceneSection* GetSectionObject() override
	{
		return &SectionObject;
	}

	virtual FText GetDisplayName() const override
	{
		return DisplayName;
	}
	
	virtual FText GetSectionTitle() const override
	{
		return FText::GetEmpty();
	}

	virtual void GenerateSectionLayout( class ISectionLayoutBuilder& LayoutBuilder ) const override { }

	virtual int32 OnPaintSection( const FGeometry& AllottedGeometry, const FSlateRect& SectionClippingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, bool bParentEnabled ) const override 
	{
		const ESlateDrawEffect::Type DrawEffects = bParentEnabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
		
		// Add a box for the section
		FSlateDrawElement::MakeBox( 
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(),
			FEditorStyle::GetBrush("Sequencer.GenericSection.Background"),
			SectionClippingRect,
			DrawEffects
		);

		return LayerId;
	}

	void SetIntermediateValueForTrack( UMovieSceneTrack* Track, FPropertyChangedParams PropertyChangedParams )
	{
		if ( SectionObject.GetOuter() == Track )
		{
			SetIntermediateValue(PropertyChangedParams);
		}
	}

	/** Sets the intermediate value for this section interface.  Intermediate values are used to display property values which
		have changed, but have not been keyed yet. */
	virtual void SetIntermediateValue(FPropertyChangedParams PropertyChangedParams) = 0;

	/** Clears any previously set intermediate values. */
	virtual void ClearIntermediateValue() = 0;

protected:

	/** Display name of the section */
	FText DisplayName;

	/** The section we are visualizing */
	UMovieSceneSection& SectionObject;
};

