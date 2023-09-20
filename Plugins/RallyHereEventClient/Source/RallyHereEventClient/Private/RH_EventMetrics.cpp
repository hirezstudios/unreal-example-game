#include "RallyHereEventClientModule.h"
#include "RH_EventClient.h"
#include "RH_EventMetrics.h"

#include <math.h>
#include <vector>
#include <random>
#include <algorithm>


//====================================================
// Helper function for testing
template <typename COUNTER_TYPE, typename VALUE_TYPE>
void RH_DumpStatistics(
    const FString& Title
    , TRH_HistogramMetrics<COUNTER_TYPE, VALUE_TYPE>& SmartHistogram
    , const std::vector<float>& Expecteds = { 0,0,0,0,0,0,0 }
    , const std::vector<float>& Percentiles = { 25, 50, 75, 90, 95, 97, 99 }
)
{
    UE_LOG(LogRallyHereEvent, Log, TEXT("%s"), *Title);


    for (auto Percentile : Percentiles)
    {
        float Value = SmartHistogram.GetQuantileValue(Percentile);
        COUNTER_TYPE CountBelow = 0;
        COUNTER_TYPE CountAbove = 0;
        SmartHistogram.GetCountAboveAndBelow(Value, CountBelow, CountAbove);

        UE_LOG(LogRallyHereEvent, Log, TEXT("Value: CountBelow/CountAbove %f: %d/%d")
            , Value
            , CountBelow
            , CountAbove
        );
    }

    UE_LOG(LogRallyHereEvent, Log,
        TEXT("%s\nBWPMean=%.2f, BWPStdDev=%.2f\nMean=%.2f, StdDev=%.2f\nMinimum=%.2f, Median=%.2f, Maximum=%.2f")
        , *Title
        , SmartHistogram.GetBWPMean(), SmartHistogram.GetBWPStandardDeviation()
        , SmartHistogram.GetMean(), SmartHistogram.GetStandardDeviation()
        , SmartHistogram.GetMinimum(), SmartHistogram.GetMedian(), SmartHistogram.GetMaximum()
    );
    for (size_t Index = 0; Index < Percentiles.size(); Index++)
    {
        float Percentile = Percentiles[Index];
        float Expected = Expecteds[Index];
        float Value = SmartHistogram.GetQuantileValue(Percentile);
        UE_LOG(LogRallyHereEvent, Log, TEXT("Percentile %.2f = %.2f [%.2f - delta: %g]")
            , Percentile
            , Value
            , Expected
            , (Value - Expected)
        );
    }
}

void RH_HistogramMetricsUnitTest()
{
#if !UE_BUILD_SHIPPING

    { //====================================================
        // sample instantiations looking for compile errors
        TRH_HistogramMetrics<unsigned char, int16> TestInt16(50, 0, 1000);
        TRH_HistogramMetrics<unsigned char, float> TestFloat(50, 0, 500);
        TRH_HistogramMetrics<unsigned char, double> TestDouble(50, 0, 500);
        TRH_HistogramMetrics<unsigned long long, float> TestDoubleDouble(1, 0, 1);

        // what happens in this case?
        TRH_HistogramMetrics<unsigned long long, int16> Test4(0, 0, 0);
        Test4.Add(-1);
        Test4.Add(0);
        Test4.Add(1);

    } //====================================================

    // use this site to check your numbers if there is a manageable amount :)
    // https://www.calculator.net/standard-deviation-calculator.html

    // test what happens if no data
    { //====================================================
        // you need to know your application to specify the types correctly
        TRH_HistogramMetrics<int32, float> FSHisto(50, 0, 500);

        RH_DumpStatistics("No data added", FSHisto);
        UE_LOG(LogRallyHereEvent, Log, TEXT("===================="));
    } //====================================================

    // test a simple set of data
    { //====================================================
        // you need to know your application to specify the types correctly
        TRH_HistogramMetrics<int32, float> FSHisto(6, -3, 3);

        FSHisto.Reset();
        std::vector<float> Test1{ -4, -3, -2, -1, 1, 2, 3, 4 };
        for (float Value : Test1)
        {
            FSHisto.Add(Value);
        }

        RH_DumpStatistics("Simple Data (percentiles are for continuous, real distributions so have little meaning here)"
            , FSHisto
            , {}
            , {}
        );
        UE_LOG(LogRallyHereEvent, Log, TEXT("===================="));
    } //====================================================

    // test a uniform distribution
    { //====================================================
        // you need to know your application to specify the types correctly
        TRH_HistogramMetrics<int32, float> FSHisto(100, 0, 1000);

        float MaxValue = 1000;

        // https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
        std::mt19937 Mersenne32;
        FSHisto.Reset();
        const size_t SampleCount = 1000000L;
        for (size_t Index = 0; Index < SampleCount; Index++)
        {
            // a nuniform distribution.
            float Sample = MaxValue * (Mersenne32() / (float)UINT_MAX);

            FSHisto.Add(Sample);
        }

        RH_DumpStatistics("Uniform distribution"
            , FSHisto
            , { 250, 500, 750, 900, 950, 970, 990 }
            , { 25,  50,  75,  90,  95,  97,  99 }
        );
        /* Expect values:
            Percentile 25.00 = 250.0
            Percentile 50.00 = 500.0
            Percentile 75.00 = 750.0
            Percentile 90.00 = 900.0
            Percentile 95.00 = 950.0
            Percentile 97.00 = 970.0
            Percentile 99.00 = 990.0
        */
        UE_LOG(LogRallyHereEvent, Log, TEXT("===================="));
    } //====================================================

    // test a normal distribution
    { //====================================================

        float Mu = 100;
        float Sigma = 25;

        /* works regardless of C++ version but is not as statistically sound
        const float M_PI = (float)3.14159265358979323846;
        auto TwoSamplesNormal = [&](float& S1, float& S2) {

            //https://www.baeldung.com/cs/uniform-to-normal-distribution
            // two uniform [0,1] values
            float U1 = (Mersenne32() / (float)UINT_MAX);
            float U2 = (Mersenne32() / (float)UINT_MAX);
            S1 = Mu + Sigma * std::sqrt(-2 * std::log(U1))*std::cos(U2 * 2 * M_PI);
            S2 = Mu + Sigma * std::sqrt(-2 * std::log(U1))*std::sin(U2 * 2 * M_PI);
        };
        */

        // requires C++11
        // https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
        std::random_device rd{};
        std::mt19937 Mersenne32{ rd() };
        std::normal_distribution<> NormalDistribution{ Mu, Sigma };

        auto TwoSamplesNormal = [&](float& S1, float& S2) {
            S1 = (float)NormalDistribution(Mersenne32);
            S2 = (float)NormalDistribution(Mersenne32);
        };

        // you need to know your application to specify the types correctly
        float MaxValue = Mu + 3 * Sigma;
        float MinValue = Mu - 3 * Sigma;
        TRH_HistogramMetrics<int32, float> FSHisto(100, MinValue, MaxValue);

        FSHisto.Reset();
        const size_t SampleCount = 50000L;
        for (size_t Index = 0; Index < SampleCount; Index++)
        {
            // generates two samples at the same time because of the simpler method; fine!
            float S1, S2;
            TwoSamplesNormal(S1, S2);
            FSHisto.Add(S1);
            FSHisto.Add(S2);
        }

        // cumulative normal values read from here: https://www.soa.org/globalassets/assets/Files/Edu/edu-cumulative-norm-table.pdf
        RH_DumpStatistics("Normal Distribution"
            , FSHisto
            , { Mu - 2 * Sigma, Mu - Sigma, Mu, Mu + Sigma, Mu + 2 * Sigma }
            , { 100.0f - 97.72f, 100.0f - 84.13f, 50.0f, 84.13f, 97.72f }
        );
        /* Expected output are sigma boundaries
            Percentile 2.28 = 50.0 // -2Sigma
            Percentile 15.87 = 75.0 // -Sigma
            Percentile 50.00 = 100.0 // Mu
            Percentile 84.13 = 125.0 // +Sigma
            Percentile 97.72 = 150.0 // +2Sigma
        */
        UE_LOG(LogRallyHereEvent, Log, TEXT("===================="));
    } //====================================================

    // test an oddball nonuniform
    if (false)
    { //====================================================
        // you need to know your application to specify the types correctly
        TRH_HistogramMetrics<int32, float> FSHisto(100, 0, 1000);

        float MaxValue = 1100;

        // https://en.cppreference.com/w/cpp/numeric/random/normal_distribution
        std::mt19937 Mersenne32;
        FSHisto.Reset();
        const size_t SampleCount = 1000000L;
        for (size_t Index = 0; Index < SampleCount; Index++)
        {
            // kludge a nonuniform distribution.
            float Range = MaxValue * Index / (float)SampleCount;
            float Sample = Range * (Mersenne32() / (float)UINT_MAX);

            FSHisto.Add(Sample);
        }

        RH_DumpStatistics("Oddball distribution", FSHisto);
        UE_LOG(LogRallyHereEvent, Log, TEXT("===================="));
    } //====================================================
#endif
}
