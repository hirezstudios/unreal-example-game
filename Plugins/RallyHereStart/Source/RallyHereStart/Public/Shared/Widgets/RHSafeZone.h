// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once

#include "Components/SafeZone.h"
#include "Engine/StreamableManager.h"
#include "RHSafeZone.generated.h"

/*
* Extends USafeZone with some visual options, such as rendering borders around the safe zone
* area
*/
UCLASS()
class RALLYHERESTART_API URHSafeZone : public USafeZone
{
    GENERATED_BODY()

public:
	URHSafeZone();

	/** If this safe zone should pad for the left side of the screen's safe zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone")
	bool bBorderLeft;

	/** If this safe zone should pad for the right side of the screen's safe zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone")
	bool bBorderRight;

	/** If this safe zone should pad for the top side of the screen's safe zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone")
	bool bBorderTop;

	/** If this safe zone should pad for the bottom side of the screen's safe zone */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone")
	bool bBorderBottom;

	/** If true, this widget will draw borders even on platforms that default to no borders: rh.SafeZone.DrawBorders=0 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SafeZone")
	bool bForceDrawBorders;

	UFUNCTION(BlueprintCallable, Category = "SafeZone")
	void SetBorderSides(bool bInBorderLeft, bool bInBorderRight, bool bInBorderTop, bool bInBorderBottom);

	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	// End of UWidget interface
};

/*
* Behaves the opposite of how the "SafeZone" widget works, by applying the reverse padding for
* child widgets. 
* For example, useful for scaling up background images deep in a widget hierarchy that should not
* be affected by the safe zone widget
*/
UCLASS()
class RALLYHERESTART_API URHUnsafeZone : public USafeZone
{
	GENERATED_BODY()

public:
	// UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// End of UWidget interface
};
