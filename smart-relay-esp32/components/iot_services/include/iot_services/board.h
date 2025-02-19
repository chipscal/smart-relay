#pragma once

namespace clab::iot_services
{
    /// @brief Saves board state (rtc, i/o, ecc...) and calls abort(). 
    void board_clean_restart();
}