#pragma once

namespace Utils
{
    // HPF de primer orden tipo "DC blocker":
    //   y[n] = alpha * (y[n-1] + x[n] - x[n-1])
    class DCRemoveFilter
    {
    public:
        explicit DCRemoveFilter(float alpha = 0.995f);

        void  setAlpha(float alpha);
        void  reset();
        float process(float x);

    private:
        float alpha_ { 0.995f };
        float prevX_ { 0.0f };
        float prevY_ { 0.0f };
    };
}