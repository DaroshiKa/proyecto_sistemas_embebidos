#pragma once

namespace Utils
{
    class ComplementaryFilter
    {
    public:
        explicit ComplementaryFilter(
            float alpha = 0.98f
        );

        // Reinicia el estado del filtro (útil tras calibración)
        void reset(
            float pitch = 0.0f,
            float roll = 0.0f,
            float yaw = 0.0f
        );

        // Coeficiente de fusión (0.95..0.99 típico)
        void setAlpha(
            float alpha
        );

        // Actualiza con muestras crudas y devuelve la orientación fusionada.
        // ax,ay,az en g; gx,gy,gz en °/s; dt en segundos.
        void update(
            float ax,
            float ay,
            float az,
            float gx,
            float gy,
            float gz,
            float dtSeconds
        );

        float pitch() const { return pitch_; }

        float roll() const { return roll_; }

        float yaw() const { return yaw_; }

    private:
        float alpha_ { 0.98f };
        float pitch_ { 0.0f };
        float roll_  { 0.0f };
        float yaw_   { 0.0f };
    };
}