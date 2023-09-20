#include "RallyHereStart.h"
#include "RH_Common.h"
#include "Shared/Widgets/RHSafeZone.h"
#include "Components/SafeZoneSlot.h"

//$$ BME: BEGIN - MaxAspectRatio for CameraComponent
static int32 GSafeZoneDrawBorders = 1;
static FAutoConsoleVariableRef CVarSafeZoneDrawBorders(
	TEXT("rh.SafeZone.DrawBorders"),
	GSafeZoneDrawBorders,
	TEXT(" 0: Do not draw black borders unless URHSafeZone::bForceDrawBorders is true \n")
	TEXT(" 1: Draw black borders around URHSafeZone widgets (default)"),
	FConsoleVariableDelegate::CreateLambda([](IConsoleVariable* InConsoleVariable)
	{
		for (TObjectIterator<URHSafeZone> SafeZoneIt; SafeZoneIt; ++SafeZoneIt)
		{
			URHSafeZone* RHSafeZone = *SafeZoneIt;
			if (RHSafeZone && !RHSafeZone->IsTemplate())
			{
				RHSafeZone->SynchronizeProperties();
			}
		}
	}),
	ECVF_Default
);
//$$ BME: END - MaxAspectRatio for CameraComponent

class SRHSafeZone : public SSafeZone
{
public:
	FDelegateHandle OnSafeFrameChangedHandle;

	const FSlateBrush* BorderImage = FCoreStyle::Get().GetBrush("BlackBrush");
	bool bBorderLeft = false;
	bool bBorderRight = false;
	bool bBorderTop = false;
	bool bBorderBottom = false;

	virtual ~SRHSafeZone() override
	{
		FCoreDelegates::OnSafeFrameChangedEvent.Remove(OnSafeFrameChangedHandle);
	}

	void Construct(const FArguments& InArgs)
	{
		SSafeZone::Construct(InArgs);
		OnSafeFrameChangedHandle = FCoreDelegates::OnSafeFrameChangedEvent.AddSP(this, &SRHSafeZone::OnSafeFrameChanged);
	}

	void SetBorderSides(bool bInBorderLeft, bool bInBorderRight, bool bInBorderTop, bool bInBorderBottom, bool bForceDrawBorders)
	{
		if (GSafeZoneDrawBorders || bForceDrawBorders)
		{
			bBorderLeft = bInBorderLeft;
			bBorderRight = bInBorderRight;
			bBorderTop = bInBorderTop;
			bBorderBottom = bInBorderBottom;
		}
		else
		{
			bBorderLeft = false;
			bBorderRight = false;
			bBorderTop = false;
			bBorderBottom = false;
		}

		Invalidate(EInvalidateWidgetReason::Paint);
	}

	void OnSafeFrameChanged()
	{
		Invalidate(EInvalidateWidgetReason::Layout);
		Invalidate(EInvalidateWidgetReason::Prepass);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		LayerId =  SSafeZone::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

		const FMargin SlotPadding = GetSafeMargin(AllottedGeometry.Scale);

		if (bBorderLeft)
		{
			const FVector2D LocalOffset(FVector2D::ZeroVector);
			const FVector2D LocalSize(SlotPadding.Left, AllottedGeometry.GetLocalSize().Y);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId++,
#if RH_FROM_ENGINE_VERSION(5,2)
				AllottedGeometry.ToPaintGeometry(LocalSize, FSlateLayoutTransform(1.f, TransformPoint(1.f, UE::Slate::CastToVector2f(LocalOffset)))),
#else
				AllottedGeometry.ToPaintGeometry(LocalOffset, LocalSize),
#endif
				BorderImage,
				ESlateDrawEffect::None,
				FLinearColor::Black
			);
		}
		if (bBorderRight)
		{
			const FVector2D LocalOffset(AllottedGeometry.GetLocalSize().X - SlotPadding.Right, 0.0f);
			const FVector2D LocalSize(SlotPadding.Right, AllottedGeometry.GetLocalSize().Y);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId++,
#if RH_FROM_ENGINE_VERSION(5,2)
				AllottedGeometry.ToPaintGeometry(LocalSize, FSlateLayoutTransform(1.f, TransformPoint(1.f, UE::Slate::CastToVector2f(LocalOffset)))),
#else
				AllottedGeometry.ToPaintGeometry(LocalOffset, LocalSize),
#endif
				BorderImage,
				ESlateDrawEffect::None,
				FLinearColor::Black
			);
		}
		if (bBorderTop)
		{
			const FVector2D LocalOffset(FVector2D::ZeroVector);
			const FVector2D LocalSize(AllottedGeometry.GetLocalSize().X, SlotPadding.Top);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId++,
#if RH_FROM_ENGINE_VERSION(5,2)
				AllottedGeometry.ToPaintGeometry(LocalSize, FSlateLayoutTransform(1.f, TransformPoint(1.f, UE::Slate::CastToVector2f(LocalOffset)))),
#else
				AllottedGeometry.ToPaintGeometry(LocalOffset, LocalSize),
#endif
				BorderImage,
				ESlateDrawEffect::None,
				FLinearColor::Black
			);
		}
		if (bBorderBottom)
		{
			const FVector2D LocalOffset(0.0f, AllottedGeometry.GetLocalSize().Y - SlotPadding.Bottom);
			const FVector2D LocalSize(AllottedGeometry.GetLocalSize().X, SlotPadding.Bottom);
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId++,
#if RH_FROM_ENGINE_VERSION(5,2)
				AllottedGeometry.ToPaintGeometry(LocalSize, FSlateLayoutTransform(1.f, TransformPoint(1.f, UE::Slate::CastToVector2f(LocalOffset)))),
#else
				AllottedGeometry.ToPaintGeometry(LocalOffset, LocalSize),
#endif
				BorderImage,
				ESlateDrawEffect::None,
				FLinearColor::Black
			);
		}
		
		return LayerId;
	}
};

URHSafeZone::URHSafeZone()
	: Super()
	, bBorderLeft(true)
	, bBorderRight(true)
	, bBorderTop(true)
	, bBorderBottom(true)
{
}

void URHSafeZone::SetBorderSides(bool bInBorderLeft, bool bInBorderRight, bool bInBorderTop, bool bInBorderBottom)
{
	bBorderLeft = bInBorderLeft;
	bBorderRight = bInBorderRight;
	bBorderTop = bInBorderTop;
	bBorderBottom = bInBorderBottom;

	if (MySafeZone.IsValid() && GetChildrenCount() > 0)
	{
		static_cast<SRHSafeZone*>(MySafeZone.Get())->SetBorderSides(bBorderLeft, bBorderRight, bInBorderTop, bBorderBottom, bForceDrawBorders);
	}
}

void URHSafeZone::SynchronizeProperties()
{
	Super::SynchronizeProperties();

	if (MySafeZone.IsValid())
	{
		static_cast<SRHSafeZone*>(MySafeZone.Get())->SetBorderSides(bBorderLeft, bBorderRight, bBorderTop, bBorderBottom, bForceDrawBorders);
	}
}

TSharedRef<SWidget> URHSafeZone::RebuildWidget()
{
#if RH_FROM_ENGINE_VERSION(5,2)
	USafeZoneSlot* SafeSlot = nullptr;
	if (GetChildrenCount() > 0)
	{
		SafeSlot = Cast<USafeZoneSlot>(GetContentSlot());;
	}


	TSharedPtr<SRHSafeZone> NewSafeZone = SNew(SRHSafeZone)
		.IsTitleSafe(SafeSlot ? SafeSlot->IsTitleSafe() : false)
		.SafeAreaScale(SafeSlot ? SafeSlot->GetSafeAreaScale() : FMargin(1, 1, 1, 1))
		.HAlign(SafeSlot ? SafeSlot->GetHorizontalAlignment() : HAlign_Fill)
		.VAlign(SafeSlot ? SafeSlot->GetVerticalAlignment() : VAlign_Fill)
		.Padding(SafeSlot ? SafeSlot->GetPadding() : FMargin())
		.PadLeft(GetPadLeft())
		.PadRight(GetPadRight())
		.PadTop(GetPadTop())
		.PadBottom(GetPadBottom())
#if WITH_EDITOR
		.OverrideScreenSize(DesignerSize)
		.OverrideDpiScale(DesignerDpi)
#endif
		[
			GetChildAt(0) ? GetChildAt(0)->TakeWidget() : SNullWidget::NullWidget
		];

	NewSafeZone->SetBorderSides(bBorderLeft, bBorderRight, bBorderTop, bBorderBottom, bForceDrawBorders);

	MySafeZone = NewSafeZone;

	return MySafeZone.ToSharedRef();
#else
	USafeZoneSlot* SafeSlot = Slots.Num() > 0 ? Cast<USafeZoneSlot>(Slots[0]) : nullptr;

	
	TSharedPtr<SRHSafeZone> NewSafeZone = SNew(SRHSafeZone)
		.IsTitleSafe(SafeSlot ? SafeSlot->bIsTitleSafe : false)
		.SafeAreaScale(SafeSlot ? SafeSlot->SafeAreaScale : FMargin(1, 1, 1, 1))
		.HAlign(SafeSlot ? SafeSlot->HAlign.GetValue() : HAlign_Fill)
		.VAlign(SafeSlot ? SafeSlot->VAlign.GetValue() : VAlign_Fill)
		.Padding(SafeSlot ? SafeSlot->Padding : FMargin())
		.PadLeft(PadLeft)
		.PadRight(PadRight)
		.PadTop(PadTop)
		.PadBottom(PadBottom)
#if WITH_EDITOR
		.OverrideScreenSize(DesignerSize)
		.OverrideDpiScale(DesignerDpi)
#endif
		[
			GetChildAt(0) ? GetChildAt(0)->TakeWidget() : SNullWidget::NullWidget
		];

	NewSafeZone->SetBorderSides(bBorderLeft, bBorderRight, bBorderTop, bBorderBottom, bForceDrawBorders);

	MySafeZone = NewSafeZone;

	return MySafeZone.ToSharedRef();
#endif

}

class SRHUnsafeZone : public SSafeZone
{
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override
	{
		const EVisibility& MyCurrentVisibility = this->GetVisibility();
		if (ArrangedChildren.Accepts(MyCurrentVisibility))
		{
			FMargin SlotPadding = GetSafeMargin(AllottedGeometry.Scale);
			SlotPadding = SlotPadding * -1.0f; // Invert the safe zone transform
			AlignmentArrangeResult XAlignmentResult = AlignChild<Orient_Horizontal>(AllottedGeometry.GetLocalSize().X, ChildSlot, SlotPadding);
			AlignmentArrangeResult YAlignmentResult = AlignChild<Orient_Vertical>(AllottedGeometry.GetLocalSize().Y, ChildSlot, SlotPadding);

			ArrangedChildren.AddWidget(
				AllottedGeometry.MakeChild(
					ChildSlot.GetWidget(),
					FVector2D(XAlignmentResult.Offset, YAlignmentResult.Offset),
					FVector2D(XAlignmentResult.Size, YAlignmentResult.Size)
				)
			);
		}
	}
};

TSharedRef<SWidget> URHUnsafeZone::RebuildWidget()
{
#if RH_FROM_ENGINE_VERSION(5,2)
	USafeZoneSlot* SafeSlot = nullptr;
	if (GetChildrenCount() > 0)
	{
		SafeSlot = Cast<USafeZoneSlot>(GetContentSlot());;
	}

	MySafeZone = SNew(SRHUnsafeZone)
		.IsTitleSafe(SafeSlot ? SafeSlot->IsTitleSafe() : false)
		.SafeAreaScale(SafeSlot ? SafeSlot->GetSafeAreaScale() : FMargin(1, 1, 1, 1))
		.HAlign(SafeSlot ? SafeSlot->GetHorizontalAlignment() : HAlign_Fill)
		.VAlign(SafeSlot ? SafeSlot->GetVerticalAlignment() : VAlign_Fill)
		.Padding(SafeSlot ? SafeSlot->GetPadding() : FMargin())
		.PadLeft(GetPadLeft())
		.PadRight(GetPadRight())
		.PadTop(GetPadTop())
		.PadBottom(GetPadBottom())
#if WITH_EDITOR
		.OverrideScreenSize(DesignerSize)
		.OverrideDpiScale(DesignerDpi)
#endif
		[
			GetChildAt(0) ? GetChildAt(0)->TakeWidget() : SNullWidget::NullWidget
		];

	return MySafeZone.ToSharedRef();
#else
	USafeZoneSlot* SafeSlot = Slots.Num() > 0 ? Cast<USafeZoneSlot>(Slots[0]) : nullptr;

	MySafeZone = SNew(SRHUnsafeZone)
		.IsTitleSafe(SafeSlot ? SafeSlot->bIsTitleSafe : false)
		.SafeAreaScale(SafeSlot ? SafeSlot->SafeAreaScale : FMargin(1, 1, 1, 1))
		.HAlign(SafeSlot ? SafeSlot->HAlign.GetValue() : HAlign_Fill)
		.VAlign(SafeSlot ? SafeSlot->VAlign.GetValue() : VAlign_Fill)
		.Padding(SafeSlot ? SafeSlot->Padding : FMargin())
		.PadLeft(PadLeft)
		.PadRight(PadRight)
		.PadTop(PadTop)
		.PadBottom(PadBottom)
#if WITH_EDITOR
		.OverrideScreenSize(DesignerSize)
		.OverrideDpiScale(DesignerDpi)
#endif
		[
			GetChildAt(0) ? GetChildAt(0)->TakeWidget() : SNullWidget::NullWidget
		];

	return MySafeZone.ToSharedRef();
#endif
}
