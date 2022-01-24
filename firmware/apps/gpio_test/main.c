
#include "board.h"
#include "utils.h"
#include "osmocom/core/timer.h"

extern void gpio_test_init(void);

/* returns '1' in case we should break any endless loop */
static void check_exec_dbg_cmd(void)
{
	int ch;

	if (!UART_IsRxReady())
		return;

	ch = UART_GetChar();

	board_exec_dbg_cmd(ch);
}


extern int main(void)
{
	led_init();
	led_blink(LED_RED, BLINK_ALWAYS_ON);
	led_blink(LED_GREEN, BLINK_ALWAYS_ON);

	/* Enable watchdog for 2000 ms, with no window */
	WDT_Enable(WDT, WDT_MR_WDRSTEN | WDT_MR_WDDBGHLT | WDT_MR_WDIDLEHLT |
		   (WDT_GetPeriod(2000) << 16) | WDT_GetPeriod(2000));

	PIO_InitializeInterrupts(0);


	printf("\r\n\r\n"
		"=============================================================================\r\n"
		"GPIO Test firmware " GIT_VERSION " (C) 2019 Sysmocom GmbH\r\n"
		"=============================================================================\r\n");

	board_main_top();

	TRACE_INFO("starting gpio test...\r\n");
	gpio_test_init();

	TRACE_INFO("entering main loop...\r\n");
	while (1) {
		WDT_Restart(WDT);

		check_exec_dbg_cmd();
		osmo_timers_prepare();
		osmo_timers_update();
	}

}
