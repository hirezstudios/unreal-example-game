// Copyright 2022-2023 Rally Here Interactive, Inc. All Rights Reserved.

#pragma once
#include <vector>

/*===============================================================
    https://www.johndcook.com/blog/standard_deviation/
    "This better way of computing variance goes back to a 1962
    paper by B. P. Welford and is presented in Donald Knuth’s Art
    of Computer Programming, Vol 2, page 232, 3rd edition"

    Initialize M1 = x1 and S1 = 0.

    For subsequent x‘s, use the recurrence formulas

    Mk = Mk-1+ (xk – Mk-1)/k
    Sk = Sk-1 + (xk – Mk-1)*(xk – Mk)

    For 2 <= k <= n, the k^th estimate of the variance is s^2 = Sk/(k – 1).

===============================================================*/



void RH_HistogramMetricsUnitTest();

// COUNTER_TYPE must be at least int32 or uint32 for framerates
template <typename COUNTER_TYPE, typename VALUE_TYPE>
class TRH_HistogramMetrics
{
protected:

    // Bins are [lower, upper), including the final bin; Value == MaxRange lands in CountAbove
    double MinRange;
    double MaxRange;
    int32 BinCount;
    double BinWidth;

    const int32 kInitialDiscard;
    int32 InitialDiscard;

    COUNTER_TYPE UnderflowCount;
    std::vector<COUNTER_TYPE> Histogram;
    COUNTER_TYPE OverflowCount;

    double Mininum;
    double Median;		// save as annotation
    double Maximum;

    COUNTER_TYPE ValueCount;

    // sum(x-u)^2 = sum(x^2 - 2x*mu + mu^2)/N = sum(x^2)/N - 2*(sum(x)/N)*sum(mu)/N + sum(mu^2)/N = sum(x^2)/N - mu^2
    double SumXSquared;	// running value
    double SumX;		// running value
    double Mean;		// save as annotation
    double StandardDeviation;		// save as annotation
    double Variance;	// save as annotation

    double BPWVarianceSum; // running value
    double BPWMean;		// always current
    double BPWStandardDeviation;	// save as annotation
    double BPWVariance; // save as annotation

public:

    VALUE_TYPE GetValueCount()
    {
        return ValueCount;
    }
    VALUE_TYPE GetMean()
    {
        int32 Divisor = (ValueCount ? ValueCount : 1);
        Mean = SumX / Divisor;
        return (VALUE_TYPE)Mean;
    }
    VALUE_TYPE GetVariance()
    {
        int32 Divisor = (ValueCount ? ValueCount : 1);
        Variance = (SumXSquared / Divisor) - (GetMean() * GetMean());
        return (VALUE_TYPE)Variance;
    }
    VALUE_TYPE GetStandardDeviation()
    {
        return (VALUE_TYPE)(StandardDeviation = std::sqrt(GetVariance()));
    }


    VALUE_TYPE GetBWPMean()
    {
        return (VALUE_TYPE)BPWMean;
    }
    VALUE_TYPE GetBPWVariance(bool IsDistributionSample = false)
    {
        int32 BWPDivisor = (ValueCount > 1 ? ValueCount : 1);
        if (IsDistributionSample && ValueCount > 1)
        {
            BWPDivisor -= 1;
        }
        BPWVariance = BPWVarianceSum / BWPDivisor;
        return (VALUE_TYPE)BPWVariance;
    }
    VALUE_TYPE GetBWPStandardDeviation(bool IsDistributionSample = false)
    {
        return (VALUE_TYPE)(BPWStandardDeviation = std::sqrt(GetBPWVariance(IsDistributionSample)));
    }

    VALUE_TYPE GetMinimum() { return (VALUE_TYPE)Mininum; }
    VALUE_TYPE GetMaximum() { return (VALUE_TYPE)Maximum; }
    VALUE_TYPE GetMedian() { return (VALUE_TYPE)(Median = GetQuantileValue(50)); }

    COUNTER_TYPE GetUnderflowCount() { return UnderflowCount; }
    COUNTER_TYPE GetOverflowCount() { return OverflowCount; }
    COUNTER_TYPE GetBinCount(int32 Index)
    {
        if (Index < 0)
        {
            return UnderflowCount;
        }
        if (Index >= Histogram.size())
        {
            return OverflowCount;
        }
        return Histogram[Index];
    }

    void Reset()
    {
        InitialDiscard = kInitialDiscard;
        ValueCount = 0;
        UnderflowCount = 0;
        OverflowCount = 0;
        Histogram.assign(BinCount, 0);

        // use C++ templateed type values
        Maximum = std::numeric_limits<VALUE_TYPE>::lowest();
        Mininum = std::numeric_limits<VALUE_TYPE>::max();
        Median = 0;

        // iterative
        BPWVarianceSum = BPWMean = BPWVariance = BPWStandardDeviation = 0;

        // Algegbraic
        SumX = SumXSquared = Mean = Variance = StandardDeviation = 0;
    }

    void Resize(int32 _BinCount, double _MinRange, double _MaxRange)
    {
        BinCount = _BinCount;
        MinRange = _MinRange;
        MaxRange = _MaxRange;
        Histogram.reserve(_BinCount);
        BinWidth = _MaxRange - _MinRange;
        BinWidth /= _BinCount ? _BinCount : 1;
        Reset();
    }

    TRH_HistogramMetrics(int32 _BinCount = 1, double _MinRange = 0, double _MaxRange = 1, int32 _InitialDiscard = 0)
        : kInitialDiscard(_InitialDiscard)
    {
        Resize(_BinCount, _MinRange, _MaxRange);
        Reset();
    }

    void Add(VALUE_TYPE Value)
    {
        if (InitialDiscard > 0)
        {
            --InitialDiscard;
            return;
        }

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

            // BWP Estimates from https://www.johndcook.com/blog/standard_deviation/ --
            if (ValueCount == 1)
            {
                BPWMean = Value;
                BPWVarianceSum = 0; // additional reset
                BPWStandardDeviation = 0; // additional reset
            }
            else
            {
                double OldBPWMean = BPWMean;
                double OldBPWVarianceSum = BPWVarianceSum;

                BPWMean = OldBPWMean + (Value - OldBPWMean) / ValueCount;
                BPWVarianceSum = OldBPWVarianceSum + (Value - OldBPWMean) * (Value - BPWMean);
            }

            SumXSquared += Value * Value;
            SumX += Value;

            size_t Index = std::min<size_t>(Histogram.size() - 1, size_t((Value - MinRange) / BinWidth));
            Histogram[Index]++;
        }

        if (Value < Mininum)
        {
            Mininum = Value;
        }

        if (Value > Maximum)
        {
            Maximum = Value;
        }
    }

    // find the data Value at the cumulative percentile requested
    VALUE_TYPE GetQuantileValue(float Percentile)
    {
        // sum over counters until you get to TargetSampleCount;
        double TargetSampleCount = (Percentile / 100.0)*ValueCount;

        if (TargetSampleCount < UnderflowCount)
        {
            return (VALUE_TYPE)MinRange;
        }

        int32 Accumulator = UnderflowCount;
        for (size_t BIndex = 0; BIndex < Histogram.size(); BIndex++)
        {
            COUNTER_TYPE CountInThisBin = Histogram[BIndex];
            if (TargetSampleCount <= (Accumulator + CountInThisBin))
            {
                // Get the interpolated value assuming that sample distribution is uniform between bin boundary values
                int32 Divisor = (CountInThisBin ? CountInThisBin : 1);
                double InterpolatedBIndex = BIndex + (TargetSampleCount - Accumulator) / Divisor;
                double InterpolatedValue = BinWidth * InterpolatedBIndex;
                return (VALUE_TYPE)(MinRange + InterpolatedValue);
            }
            // otherwise keep integrating
            Accumulator += CountInThisBin;
        }

        // we ran off the end of the histogram
        return (VALUE_TYPE)MaxRange;
    }

    COUNTER_TYPE GetCountAbove(VALUE_TYPE Value)
    {
        COUNTER_TYPE CountBelow = 0;
        COUNTER_TYPE CountAbove = 0;
        GetCountAboveAndBelow(Value, CountBelow, CountAbove);
        return CountAbove;
    }

    COUNTER_TYPE GetCountBelow(VALUE_TYPE Value)
    {
        COUNTER_TYPE CountBelow = 0;
        COUNTER_TYPE CountAbove = 0;
        GetCountAboveAndBelow(Value, CountBelow, CountAbove);
        return CountBelow;
    }

    void GetCountAboveAndBelow(VALUE_TYPE Value, /*out*/ COUNTER_TYPE& CountBelow, /*out*/ COUNTER_TYPE& CountAbove)
    {
        // we need the inverse of VALUE_TYPE GetQuantileValue(float Percentile)

        if (Value < Mininum)
        {
            CountBelow = 0;
            CountAbove = ValueCount + UnderflowCount + OverflowCount;
            return;
        }

        if (Value > Maximum)
        {
            CountBelow = ValueCount + UnderflowCount + OverflowCount;
            CountAbove = 0;
            return;
        }

        if (Value < MinRange) // but > Minimum
        {
            CountBelow = UnderflowCount;
            CountAbove = ValueCount + OverflowCount;
            return;
        }

        if (Value > MaxRange) // but < Maximum
        {
            CountBelow = ValueCount + UnderflowCount;
            CountAbove = OverflowCount;
            return;
        }

        // the value must be >= MinRange and <= MaxRange
        double InterpolatedBinIndex = (Value - MinRange) / BinWidth;
        size_t LastFullBinIndex = std::min<size_t>(Histogram.size() - 1, size_t(InterpolatedBinIndex));
        COUNTER_TYPE CountBelowTheValue = UnderflowCount;
        for (size_t Index = 0; Index < LastFullBinIndex; Index++)
        {
            CountBelowTheValue += Histogram[Index];
        }

        if (InterpolatedBinIndex < (Histogram.size() - 1))
        {
            COUNTER_TYPE CountInBin = Histogram[LastFullBinIndex + 1];
            double ShareOfBin = (InterpolatedBinIndex - LastFullBinIndex) / BinWidth;
            CountBelowTheValue += COUNTER_TYPE(CountInBin * ShareOfBin);
        }

        CountBelow = CountBelowTheValue;
        CountAbove = ValueCount + OverflowCount - CountBelowTheValue;
    }
};
