#include "communication/UARTConsole.hpp"

#include <cstdio>
#include <cstring>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace Communication
{
    UARTConsole::UARTConsole(
        HAL::UARTHal& uartHal,
        uart_port_t port
    )
        : uart_(uartHal),
          port_(port)
    {
    }

    bool UARTConsole::bind(int baudRate, gpio_num_t tx, gpio_num_t rx)
    {
        return uart_.initialize(port_, baudRate, tx, rx);
    }

    void UARTConsole::write(const char* s)
    {
        if (s == nullptr) return;
        uart_.write(
            port_,
            reinterpret_cast<const uint8_t*>(s),
            strlen(s)
        );
    }

    void UARTConsole::writeLine(const char* s)
    {
        if (s != nullptr) write(s);
        write("\r\n");
    }

    void UARTConsole::printf(const char* fmt, ...)
    {
        char buffer[256];
        va_list ap;
        va_start(ap, fmt);
        const int n = vsnprintf(buffer, sizeof(buffer), fmt, ap);
        va_end(ap);

        if (n > 0)
        {
            uart_.write(
                port_,
                reinterpret_cast<const uint8_t*>(buffer),
                static_cast<size_t>(n)
            );
        }
    }

    bool UARTConsole::readLine(
        char* outLine,
        size_t outLineSize,
        uint32_t timeoutMs
    )
    {
        if (outLine == nullptr || outLineSize == 0) return false;

        uint8_t buf[32];

        const int got = uart_.read(
            port_,
            buf,
            sizeof(buf),
            pdMS_TO_TICKS(timeoutMs)
        );

        if (got <= 0) return false;

        for (int i = 0; i < got; ++i)
        {
            const char c = static_cast<char>(buf[i]);

            // Echo
            uart_.write(port_, reinterpret_cast<const uint8_t*>(&c), 1);

            if (c == '\r' || c == '\n')
            {
                // Fin de línea
                if (c == '\r')
                {
                    const char lf = '\n';
                    uart_.write(port_, reinterpret_cast<const uint8_t*>(&lf), 1);
                }

                lineBuf_[lineIdx_] = '\0';

                // Copiar a outLine
                size_t toCopy = lineIdx_;
                if (toCopy >= outLineSize) toCopy = outLineSize - 1;
                memcpy(outLine, lineBuf_, toCopy);
                outLine[toCopy] = '\0';

                lineIdx_ = 0;
                return true;
            }
            else if (c == 0x08 || c == 0x7F)
            {
                // Backspace / DEL
                if (lineIdx_ > 0)
                {
                    --lineIdx_;
                    write("\b \b");
                }
            }
            else if (lineIdx_ < LINE_BUFFER_SIZE - 1 &&
                     c >= 0x20 && c < 0x7F)
            {
                lineBuf_[lineIdx_++] = c;
            }
        }

        return false;
    }
}