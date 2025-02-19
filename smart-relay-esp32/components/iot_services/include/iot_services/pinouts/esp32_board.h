#pragma once

#define __MAKE_PULSE_ENTRY(p) { p, 0U }


#define __ADC0          0U
#define __ADC1          1U
#define __ADC2          2U
#define __ADC3          3U

#define __ADC_CH0       0U
#define __ADC_CH1       1U
#define __ADC_CH2       2U
#define __ADC_CH3       3U

#define __ADC_ATTEN0    0U
#define __ADC_ATTEN1    1U
#define __ADC_ATTEN2    2U
#define __ADC_ATTEN3    3U
#define __ADC_ATTEN4    4U
#define __ADC_ATTEN5    5U



#if defined(CONFIG_DEVELOPMENT_BOARD)
    #include "dev_board.h"
    // Note: it is possible to define a new board type using the file
    // dev_board.h or it is possible to add multiple choices into the
    // "Board type definition" menu inside the Kconfig file

#elif defined(CONFIG_SMART_RELAY_WIFI_R1)
    #if CONFIG_EDGE_COM_HREV <= 1
        #include "rev_0_1/smart_relay_wifi_r1_rev_0_1.h"
    #else
        //Note: you can also use hardware revision to load a different configuration
        static_assert(false, "No configuration available for this revision");  
    #endif
#else
    static_assert(false, "No configuration available for this board");  
#endif