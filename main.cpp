/*-
 * $Copyright$
-*/

#include <stm32f4/PwrViaSTM32F4.hpp>
#include <stm32f4/FlashViaSTM32F4.hpp>
#include <stm32f4/RccViaSTM32F4.hpp>
#include <stm32f4/ScbViaSTM32F4.hpp>
#include <stm32f4/NvicViaSTM32F4.hpp>

#include <gpio/GpioAccess.hpp>
#include <gpio/GpioEngine.hpp>
#include <gpio/GpioPin.hpp>

#include <uart/UartAccess.hpp>
#include <uart/UartDevice.hpp>

#include <tasks/Heartbeat.hpp>

#include <usb/UsbCoreViaSTM32F4.hpp>
#include <usb/UsbDeviceViaSTM32F4.hpp>
#include <usb/InEndpointViaSTM32F4.hpp>
#include <usb/OutEndpointViaSTM32F4.hpp>

#include <usb/UsbTypes.hpp>

#include <usb/UsbDevice.hpp>
#include <usb/UsbInEndpoint.hpp>
#include <usb/UsbOutEndpoint.hpp>
#include <usb/UsbControlPipe.hpp>
#include <usb/UsbConfiguration.hpp>
#include <usb/UsbInterface.hpp>

#include <usb/UsbApplication.hpp>

#include <version.h>

/*******************************************************************************
 *
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

/*******************************************************************************
 * Information on some Segments, defined via Linker Script
 ******************************************************************************/
extern char stext, etext;
extern char sdata, edata;
extern char sbss, ebss;
extern char bstack, estack;

/*******************************************************************************
 * USB Descriptors (defined in UsbDescriptors.cpp)
 ******************************************************************************/
extern const ::usb::UsbDeviceDescriptor_t usbDeviceDescriptor;
extern const ::usb::UsbStringDescriptors_t usbStringDescriptors;
extern const ::usb::UsbConfigurationDescriptor_t usbConfigurationDescriptor;

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */


/*******************************************************************************
 * System Devices
 ******************************************************************************/
static devices::RccViaSTM32F4::PllConfiguration pllCfg(336, 8, devices::RccViaSTM32F4::e_PllP_Div2, 7,
    devices::RccViaSTM32F4::e_APBPrescaler_Div4,
    devices::RccViaSTM32F4::e_APBPrescaler_Div2,
    devices::RccViaSTM32F4::e_AHBPrescaler_None,
    devices::RccViaSTM32F4::e_PllSourceHSE,
    devices::RccViaSTM32F4::e_SysclkPLL,
    16000000,
    8000000);

static devices::PwrViaSTM32F4           pwr(PWR);
static devices::FlashViaSTM32F4         flash(FLASH);
static devices::RccViaSTM32F4           rcc(RCC, pllCfg, flash, pwr);
static devices::ScbViaSTM32F4           scb(SCB);
static devices::NvicViaSTM32F4          nvic(NVIC, scb);

/*******************************************************************************
 * GPIO Engine Handlers
 ******************************************************************************/
static gpio::GpioAccessViaSTM32F4_GpioA gpio_A(rcc);
static gpio::GpioEngine                 gpio_engine_A(&gpio_A);

static gpio::GpioAccessViaSTM32F4_GpioC gpio_C(rcc);
static gpio::GpioEngine                 gpio_engine_C(&gpio_C);

static gpio::GpioAccessViaSTM32F4_GpioD gpio_D(rcc);
static gpio::GpioEngine                 gpio_engine_D(&gpio_D);

/*******************************************************************************
 * LEDs
 ******************************************************************************/
static gpio::PinT<decltype(gpio_engine_D)>  g_led_gn(&gpio_engine_D, 12);
static gpio::PinT<decltype(gpio_engine_D)>  g_led_or(&gpio_engine_D, 13);
static gpio::PinT<decltype(gpio_engine_D)>  g_led_rd(&gpio_engine_D, 14);
static gpio::PinT<decltype(gpio_engine_D)>  g_led_bl(&gpio_engine_D, 15);

/*******************************************************************************
 * UART
 ******************************************************************************/
static gpio::PinT<decltype(gpio_engine_C)>  uart_tx(&gpio_engine_C, 6);
static gpio::PinT<decltype(gpio_engine_C)>  uart_rx(&gpio_engine_C, 7);
#if defined(STM32F4_DISCOVERY)
static uart::UartAccessSTM32F4_Uart6    uart_access(rcc, uart_rx, uart_tx);
#endif /* defined(STM32F4_DISCOVERY) */
uart::UartDevice                        g_uart(&uart_access);

/*******************************************************************************
 * USB Device
 ******************************************************************************/
static gpio::PinT<decltype(gpio_engine_A)>  usb_pin_dm(&gpio_engine_A, 11);
static gpio::PinT<decltype(gpio_engine_A)>  usb_pin_dp(&gpio_engine_A, 12);
static gpio::PinT<decltype(gpio_engine_A)>  usb_pin_vbus(&gpio_engine_A, 9);
static gpio::PinT<decltype(gpio_engine_A)>  usb_pin_id(&gpio_engine_A, 10);

static usb::stm32f4::UsbFullSpeedCore                               usbCore(nvic, rcc, usb_pin_dm, usb_pin_dp, usb_pin_vbus, usb_pin_id, /* p_rxFifoSzInWords = */ 256);
static usb::stm32f4::UsbDeviceViaSTM32F4                            usbHwDevice(usbCore);
static usb::stm32f4::InEndpointViaSTM32F4                           defaultHwInEndpoint(usbHwDevice, /* p_fifoSzInWords = */ 0x20, 0);
static usb::stm32f4::CtrlInEndpointViaSTM32F4                       defaultHwCtrlInEndpoint(defaultHwInEndpoint);
static usb::stm32f4::OutEndpointViaSTM32F4                          defaultHwOutEndpoint(usbHwDevice, 0);

static usb::stm32f4::InEndpointViaSTM32F4                           inHwEndpoint(usbHwDevice, /* p_fifoSzInWords = */ 128, 1);
static usb::stm32f4::BulkInEndpointViaSTM32F4                       bulkInHwEndp(inHwEndpoint);
usb::UsbBulkInEndpoint                                              bulkInEndpoint(bulkInHwEndp);

#if defined(USB_APPLICATION_LOOPBACK)
/* TODO I've found that increasing the buffer size beyond 951 Bytes will make things stop working. */
static usb::UsbBulkOutLoopbackApplicationT<512>                     bulkOutApplication(bulkInEndpoint);
#elif defined(USB_APPLICATION_UART)
static usb::UsbUartApplicationT<decltype(uart_access)>              bulkOutApplication(uart_access);
#else
#warning No USB Application defined.
#endif

#if defined(USB_APPLICATION_LOOPBACK) || defined(USB_APPLICATION_UART)
static usb::stm32f4::OutEndpointViaSTM32F4                                  outHwEndpoint(usbHwDevice, 1);
static usb::UsbBulkOutEndpointT<usb::stm32f4::BulkOutEndpointViaSTM32F4>    bulkOutEndpoint(bulkOutApplication);
static usb::stm32f4::BulkOutEndpointViaSTM32F4                              bulkOutHwEndp(outHwEndpoint, bulkOutEndpoint);
#endif /* defined(USB_APPLICATION_LOOPBACK) || defined(USB_APPLICATION_UART) */

#if defined(USB_INTERFACE_VCP)
static usb::UsbVcpInterface                                         usbInterface(bulkOutEndpoint, bulkInEndpoint);
#elif defined(USB_INTERFACE_VENDOR)
static usb::UsbVendorInterface                                      usbInterface(bulkOutEndpoint, bulkInEndpoint);
#else
#error No USB Interface defined.
#endif

static usb::UsbConfigurationT<decltype(usbConfigurationDescriptor)> usbConfiguration(usbInterface, usbConfigurationDescriptor);

static usb::UsbDevice                                                       genericUsbDevice(usbHwDevice, usbDeviceDescriptor, usbStringDescriptors, { &usbConfiguration });

static usb::UsbCtrlInEndpoint                                               ctrlInEndp(defaultHwCtrlInEndpoint);
static usb::UsbControlPipe                                                  defaultCtrlPipe(genericUsbDevice, ctrlInEndp);

static usb::UsbCtrlOutEndpointT<usb::stm32f4::CtrlOutEndpointViaSTM32F4>    ctrlOutEndp(defaultCtrlPipe);
static usb::stm32f4::CtrlOutEndpointViaSTM32F4                              defaultCtrlOutEndpoint(defaultHwOutEndpoint, ctrlOutEndp);

/*******************************************************************************
 * Tasks
 ******************************************************************************/
static tasks::HeartbeatT<decltype(g_uart), decltype(g_led_gn)>      heartbeat_gn("heartbt", g_uart, g_led_gn, 3, 1000);

/*******************************************************************************
 *
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* defined(__cplusplus) */

[[noreturn]]
void
main(void) {
    g_led_gn.enable(gpio::GpioAccessViaSTM32F4::e_Output, gpio::GpioAccessViaSTM32F4::e_None, gpio::GpioAccessViaSTM32F4::e_Gpio);
    g_led_or.enable(gpio::GpioAccessViaSTM32F4::e_Output, gpio::GpioAccessViaSTM32F4::e_None, gpio::GpioAccessViaSTM32F4::e_Gpio);
    g_led_rd.enable(gpio::GpioAccessViaSTM32F4::e_Output, gpio::GpioAccessViaSTM32F4::e_None, gpio::GpioAccessViaSTM32F4::e_Gpio);
    g_led_bl.enable(gpio::GpioAccessViaSTM32F4::e_Output, gpio::GpioAccessViaSTM32F4::e_None, gpio::GpioAccessViaSTM32F4::e_Gpio);

    g_uart.printf("Copyright (c) 2013-2017, 2020 Philip Schulz <phs@phisch.org>\r\n");
    g_uart.printf("All rights reserved.\r\n");
    g_uart.printf("\r\n");
    g_uart.printf("SW Version: %s\r\n", gSwVersionId);
    g_uart.printf("SW Build Timestamp: %s\r\n", gSwBuildTime);
    g_uart.printf("\r\n");
    g_uart.printf("Fixed Data: [0x0%x - 0x0%x]\t(%d Bytes total, %d Bytes used)\r\n",
      &gFixedDataBegin, &gFixedDataEnd, &gFixedDataEnd - &gFixedDataBegin, &gFixedDataUsed- &gFixedDataBegin);
    g_uart.printf("      Code: [0x0%x - 0x0%x]\t(%d Bytes)\r\n", &stext, &etext, &etext - &stext);
    g_uart.printf("      Data: [0x%x - 0x%x]\t(%d Bytes)\r\n", &sdata, &edata, &edata - &sdata);
    g_uart.printf("       BSS: [0x%x - 0x%x]\t(%d Bytes)\r\n", &sbss, &ebss, &ebss - &sbss);
    g_uart.printf(" Total RAM: [0x%x - 0x%x]\t(%d Bytes)\r\n", &sdata, &ebss, &ebss - &sdata);
    g_uart.printf("     Stack: [0x%x - 0x%x]\t(%d Bytes)\r\n", &bstack, &estack, &estack - &bstack);
    g_uart.printf("\r\n");

    unsigned sysclk = rcc.getSysclkSpeedInHz() / 1000;
    unsigned ahb    = rcc.getAhbSpeedInHz() / 1000;
    unsigned apb1   = rcc.getApb1SpeedInHz() / 1000;
    unsigned apb2   = rcc.getApb2SpeedInHz() / 1000;

    g_uart.printf("CPU running @ %d kHz\r\n", sysclk);
    g_uart.printf("        AHB @ %d kHz\r\n", ahb);
    g_uart.printf("       APB1 @ %d kHz\r\n", apb1);
    g_uart.printf("       APB2 @ %d kHz\r\n", apb2);
    g_uart.printf("\r\n");

    /* Inform FreeRTOS about clock speed */
    if (SysTick_Config(rcc.getSysclkSpeedInHz() / configTICK_RATE_HZ)) {
        g_uart.printf("FATAL: Capture Error!\r\n");
        goto bad;
    }

    usbHwDevice.start();

    g_uart.printf("Starting FreeRTOS Scheduler...\r\n");
    vTaskStartScheduler();

    usbHwDevice.stop();

bad:
    g_led_rd.set(gpio::Pin::On);
    g_uart.printf("FATAL ERROR!\r\n");
    while (1) ;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined(__cplusplus) */

/*******************************************************************************
 *
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

void
led2_off(void) {
    g_led_or.set(gpio::GpioPin::Off);
}

void
led3_off(void) {
    g_led_bl.set(gpio::GpioPin::Off);
}

[[noreturn]]
void
halt(const char * const p_file, const unsigned p_line) {
    g_led_rd.enable(gpio::GpioAccessViaSTM32F4::e_Output, gpio::GpioAccessViaSTM32F4::e_None, gpio::GpioAccessViaSTM32F4::e_Gpio);
    g_led_rd.set(gpio::Pin::On);

    g_uart.printf("%s(): %s : %d\r\n", __func__, p_file, p_line);

    while (1) { };
}

[[noreturn]]
void
assert_failed(uint8_t *p_file, uint32_t p_line) {
    __disable_irq();

    halt(reinterpret_cast<char *>(p_file), p_line);
}

int
usleep(unsigned p_usec) {
    SysTick_Type *sysTick = reinterpret_cast<SysTick_Type *>(SysTick_BASE);

    /*
     * Halt SysTick, if already running. Also, store current SysTick status.
     */
    bool enabled = (sysTick->CTRL & SysTick_CTRL_ENABLE_Msk) != 0;
    if (enabled) {
        sysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    }

    unsigned safeCtrl = sysTick->CTRL;
    unsigned safeLoad = sysTick->LOAD;
    unsigned safeVal  = sysTick->VAL;

    /*
     * Configure SysTick for 1ms Overflow, then wait for required number of
     * milliseconds.
     */
    const unsigned ticksPerMs = rcc.getSysclkSpeedInHz() / 1000;
    assert((ticksPerMs & 0x00FFFFFF) == ticksPerMs); 
    unsigned waitMs = p_usec / 1000;

    sysTick->CTRL &= ~SysTick_CTRL_TICKINT_Msk;
    sysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;

    sysTick->LOAD = ticksPerMs;
    sysTick->VAL = ticksPerMs;
    sysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while (waitMs > 0) {
        while (!(sysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) ;
        waitMs--;
    }
    sysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    /*
     * Configure SysTick for 1us Overflow, then wait for required number of
     * microseconds.
     */
    const unsigned ticksPerUs = rcc.getSysclkSpeedInHz() / (1000 * 1000);
    assert((ticksPerUs & 0x00FFFFFF) == ticksPerUs);
    unsigned waitUs = p_usec & 1024; // Assumes 1ms = 1024us. Close enough.

    sysTick->LOAD = ticksPerUs;
    sysTick->VAL = ticksPerUs;
    sysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while (waitUs > 0) {
        while (!(sysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)) ;
        waitUs--;
    }
    sysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;

    /*
     * Restore SysTick status.
     */
    sysTick->VAL  = safeVal;
    sysTick->LOAD = safeLoad;
    sysTick->CTRL = safeCtrl;
    if (enabled) {
        sysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    }
    
    return 0;
}

extern const uintptr_t __stack_chk_guard = 0xa5a5a5a5;

[[noreturn]]
void
__stack_chk_fail( ) {
    __disable_irq();

    halt("Stack Check failed.", 0);
}

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined (__cplusplus) */

/*******************************************************************************
 *
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* defined (__cplusplus) */

void
OTG_FS_WKUP_IRQHandler(void) {
    while (1) ;
}

void
OTG_FS_IRQHandler(void) {
    usbCore.handleIrq();
}

void
OTG_HS_EP1_OUT_IRQHandler(void) {
    while (1) ;
}

void
OTG_HS_EP1_IN_IRQHandler(void) {
    while (1) ;
}

void
OTG_HS_WKUP_IRQHandler(void) {
    while (1) ;
}

void
OTG_HS_IRQHandler(void) {
    while (1) ;
}

#if defined(__cplusplus)
} /* extern "C" */
#endif /* defined (__cplusplus) */

