// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Math/NumericLimits.h"
#include "GenericPlatform/GenericPlatformMath.h"
#include "PlayerExp_StatAccumulator.generated.h"

class FJsonValue;

USTRUCT()
struct FPlayerExp_StatAccumulator 
{
    GENERATED_BODY()

protected:

    // the state information we use to do our work
    double MinRange;
    double MaxRange;
    int32 BinCount;
    double BinWidth;

    double MinimumValue;
    double MaximumValue;

    TArray<int32> Histogram;
    int32 UnderflowCount;
    int32 OverflowCount;
    int32 ValueCount;

    // sum(x-u)^2 = sum(x^2 - 2x*mu + mu^2)/N = sum(x^2)/N - 2*(sum(x)/N)*sum(mu)/N + sum(mu^2)/N = sum(x^2)/N - mu^2
    double SumXSquared;	// running value
    double SumX;		// running value

    void Reset()
    {
        ValueCount = 0;
        UnderflowCount = 0;
        OverflowCount = 0;
        Histogram.Init(0, BinCount);

        // use templated type values
        MaximumValue = -BIG_NUMBER; // TNumericLimits<double>::Lowest(); //  std::numeric_limits<double>::lowest();
        MinimumValue = BIG_NUMBER; // TNumericLimits<double>::Max(); // std::numeric_limits<double>::max();

        // Algebraic
        SumX = SumXSquared = 0;
    }

    void Resize(int32 _BinCount, double _MinRange, double _MaxRange)
    {
        BinCount = _BinCount;
        MinRange = _MinRange;
        MaxRange = _MaxRange;
        Histogram.Reserve(_BinCount);
        BinWidth = _MaxRange - _MinRange;
        BinWidth /= _BinCount ? _BinCount : 1;
        Reset();
    }

    // find the data Value at the cumulative percentile requested
    double GetQuantileValue(float Percentile)
    {
        // sum over counters until you get to TargetSampleCount;
        double TargetSampleCount = (Percentile / 100.0)*ValueCount;

        if (TargetSampleCount < UnderflowCount)
        {
            return MinRange;
        }

        int32 Accumulator = UnderflowCount;
        for (size_t BIndex = 0; BIndex < Histogram.Num(); BIndex++)
        {
            int32 CountInThisBin = Histogram[BIndex];
            if (TargetSampleCount <= (Accumulator + CountInThisBin))
            {
                // Get the interpolated value assuming that sample distribution is uniform between bin boundary values
                int32 Divisor = (CountInThisBin ? CountInThisBin : 1);
                double InterpolatedBIndex = BIndex + (TargetSampleCount - Accumulator) / Divisor;
                double InterpolatedValue = BinWidth * InterpolatedBIndex;
                return (MinRange + InterpolatedValue);
            }
            // otherwise keep integrating
            Accumulator += CountInThisBin;
        }

        // we ran off the end of the histogram
        return MaxRange;
    }

protected:
    // the bits that get turned into JSON

	UPROPERTY()
	float Minimum;
	UPROPERTY()
	float Maximum;
    UPROPERTY()
    double Mean;
    UPROPERTY()
    double StandardDeviation;
    UPROPERTY()
    double Variance;
    UPROPERTY()
    double Quantile25;
    UPROPERTY()
    double Quantile50;
    UPROPERTY()
    double Quantile75;
    UPROPERTY()
    double Quantile90;
    UPROPERTY()
    double Quantile95;
    UPROPERTY()
    double Quantile97;
    UPROPERTY()
    double Quantile99;
    UPROPERTY()
    int32 SampleUnderflowCount;
    UPROPERTY()
    int32 SampleOverflowCount;
    UPROPERTY()
    int32 SampleCount;

public:
    FPlayerExp_StatAccumulator()
        : Minimum(0.0f)
        , Maximum(0.0f)
        , Mean(0.0)
        , StandardDeviation(0.0)
        , Variance(0.0)
        , Quantile25(0.0)
        , Quantile50(0.0)
        , Quantile75(0.0)
        , Quantile90(0.0)
        , Quantile95(0.0)
        , Quantile97(0.0)
        , Quantile99(0.0)
        , SampleUnderflowCount(0)
        , SampleOverflowCount(0)
        , SampleCount(0)
    {
        Resize(100 /*BinCount*/, 0/*MinRange*/, 500/*MaxRange*/);
        Reset();
    }

    FORCEINLINE void CaptureStatsForReport()
    {
        Minimum = GetMinimum();
        Maximum = GetMaximum();
        Mean = GetMean();
        StandardDeviation = GetStandardDeviation();
        Variance = GetVariance();
        Quantile25 = GetQuantileValue(25);
        Quantile50 = GetQuantileValue(50);
        Quantile75 = GetQuantileValue(75);
        Quantile90 = GetQuantileValue(90);
        Quantile95 = GetQuantileValue(95);
        Quantile97 = GetQuantileValue(97);
        Quantile99 = GetQuantileValue(99);
        SampleUnderflowCount = GetUnderflowCount();
        SampleOverflowCount = GetOverflowCount();
        SampleCount = GetSampleCount();
    }

    FORCEINLINE double GetMinimum() const { return MinimumValue; }
    FORCEINLINE double GetMaximum() const { return MaximumValue; }
    FORCEINLINE double GetMean() const { return (SumX / (ValueCount ? ValueCount : 1)); }
    FORCEINLINE double GetVariance() const  { return FMath::Max<double>(0,(SumXSquared / (ValueCount ? ValueCount : 1)) - (GetMean() * GetMean())); }
    FORCEINLINE double GetStandardDeviation() const { return (double)FMath::Sqrt(GetVariance()); }

    FORCEINLINE int32 GetValueCount() const { return ValueCount; }
    FORCEINLINE int32 GetUnderflowCount() const { return UnderflowCount; }
    FORCEINLINE int32 GetOverflowCount() const { return OverflowCount; }
    FORCEINLINE int32 GetTotalSamples() const { return GetValueCount() + GetUnderflowCount() + GetOverflowCount(); }
    FORCEINLINE int32 GetSampleCount() const { return GetValueCount(); }

    FORCEINLINE void ResetStatistics() { Reset(); }

    FORCEINLINE void AddSample(double Value)
    {
        if (Value < MinRange)
        {
            UnderflowCount++;
        }
        else if (Value >= MaxRange)
        {
            OverflowCount++;
        }
        else
        {
            ValueCount++;

            SumXSquared += Value * Value;
            SumX += Value;

            size_t Index = FMath::Min<size_t>(Histogram.Num() - 1, size_t((Value - MinRange) / BinWidth));
            Histogram[Index]++;
        }

        if (Value < MinimumValue)
        {
            MinimumValue = Value;
        }

        if (Value > MaximumValue)
        {
            MaximumValue = Value;
        }
    }


    FORCEINLINE friend FPlayerExp_StatAccumulator operator+(const FPlayerExp_StatAccumulator& A, const FPlayerExp_StatAccumulator& B)
    {
        if (A.GetTotalSamples() <= 0)
        {
            return B;
        }

        if (B.GetTotalSamples() <= 0)
        {
            return A;
        }

        check(A.BinCount == B.BinCount);
        check(A.MinRange == B.MinRange);
        check(A.MaxRange == B.MaxRange);

        FPlayerExp_StatAccumulator Combined;

        Combined.MinimumValue = FMath::Min(A.MinimumValue, B.MinimumValue);
        Combined.MaximumValue = FMath::Max(A.MaximumValue, B.MaximumValue);
        Combined.ValueCount = A.ValueCount + B.ValueCount;
        Combined.UnderflowCount = A.UnderflowCount + B.UnderflowCount;
        Combined.OverflowCount = A.OverflowCount + B.OverflowCount;

        Combined.SumX = A.SumX + B.SumX;
        Combined.SumXSquared = A.SumXSquared + B.SumXSquared;

        for (size_t Index = 0; Index < A.Histogram.Num(); Index++)
        {
            Combined.Histogram[Index] = A.Histogram[Index] + B.Histogram[Index];
        }

        return Combined;
    }

    FORCEINLINE FPlayerExp_StatAccumulator& operator+=(const FPlayerExp_StatAccumulator& Other)
    {
        if (Other.GetTotalSamples() <= 0)
        {
            return *this;
        }

        if (GetTotalSamples() <= 0)
        {
            *this = Other;
            return *this;
        }

        FPlayerExp_StatAccumulator Combined = *this + Other;
        *this = Combined;
        return *this;
    }

    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
    {
        // read or write
        Ar << MinimumValue;
        Ar << MaximumValue;
        Ar << UnderflowCount;
        Ar << OverflowCount;
        Ar << ValueCount;
        Ar << SumXSquared;
        Ar << SumX;

        typedef uint8 TSerializedIndex;
        // cause a ruckus if someone makes a histogram too large
        check(Histogram.Num() < (sizeof(TSerializedIndex)<<8));

#if 0
        // unoptimized for testing
        for (TSerializedIndex Index = 0; Index < Histogram.Num(); Index++)
        {
            Ar << Histogram[Index];
        }
#else 
        // reduced message size
        TSerializedIndex ModifiedBinCount = 0;
        if (Ar.IsSaving())
        {
            for (TSerializedIndex Index = 0; Index < Histogram.Num(); Index++)
            {
                if (Histogram[Index] > 0)
                {
                    ModifiedBinCount++;
                }
            }

            Ar << ModifiedBinCount;
            // write each nonzero (index,count) pair
            for (TSerializedIndex Index = 0; Index < Histogram.Num(); Index++)
            {
                if (Histogram[Index] > 0)
                {
                    Ar << Index;
                    Ar << Histogram[Index];
                }
            }
        }

        if (Ar.IsLoading())
        {
            Ar << ModifiedBinCount;
            // read each nonzero (index,count) pair
            TSerializedIndex Index;
            for (TSerializedIndex Count = 0; Count < ModifiedBinCount; Count++)
            {
                Ar << Index;
                Ar << Histogram[Index];
            }
        }
#endif

        bOutSuccess = true;
        return true;
    }
};

template<>
struct TStructOpsTypeTraits<FPlayerExp_StatAccumulator> : public TStructOpsTypeTraitsBase2<FPlayerExp_StatAccumulator>
{
    enum
    {
        WithNetSerializer = true,
    };
};