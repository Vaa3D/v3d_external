#include "nanvst.h"

extern "C" {
#include "nvstusb.h"
#include "libusb.h"
}

#include <iostream>
#include <cassert>
#include <sstream>

using namespace std;

/* cpu clock */
#define NVSTUSB_CLOCK           48000000LL

/* T0 runs at 4 MHz */
#define NVSTUSB_T0_CLOCK        (NVSTUSB_CLOCK/12LL)
#define NVSTUSB_T0_COUNT(us)    (-(us)*(NVSTUSB_T0_CLOCK/1000000)+1)
#define NVSTUSB_T0_US(count)    (-(count-1)/(NVSTUSB_T0_CLOCK/1000000))

/* T2 runs at 12 MHz */
#define NVSTUSB_T2_CLOCK        (NVSTUSB_CLOCK/ 4LL)
#define NVSTUSB_T2_COUNT(us)    (-(us)*(NVSTUSB_T2_CLOCK/1000000)+1)
#define NVSTUSB_T2_US(count)    (-(count-1)/(NVSTUSB_T2_CLOCK/1000000))

#define NVSTUSB_CMD_WRITE       (0x01)  /* write data */
#define NVSTUSB_CMD_READ        (0x02)  /* read data */
#define NVSTUSB_CMD_CLEAR       (0x40)  /* set data to 0 */

#define NVSTUSB_CMD_SET_EYE     (0xAA)  /* set current eye */
#define NVSTUSB_CMD_CALL_X0199  (0xBE)  /* call routine at 0x0199 */

/* state of the controller */
struct nvstusb_context {
  /* currently selected refresh rate */
  float rate;

  /* currently active eye */
  enum nvstusb_eye eye;

  /* device handle */
  struct nvstusb_usb_device *device;

  /* Toggled state */
  int toggled3D;

  /* Vblank method */
  int vblank_method;

  /* Invert eyes command status */
  int invert_eyes;

  /* Stereo Thread handler */
  pthread_t s_thread;

  /* Stereo thread state */
  char b_thread_running;
};

/* set controller refresh rate (should be monitor refresh rate) */
void nvstusb_set_rate2(
    nvstusb_context *ctx,
    float rate,
    float phase_offset = 4774.25)
{
    assert(ctx != 0);
    assert(ctx->device != 0);
    assert(rate > 60);

    /* send some magic data to device, this function is mainly black magic */

    /* some timing voodoo */
    int32_t frameTime   = (1000000.0/rate);     /* 8.33333 ms if 120 Hz */
    int32_t activeTime  = 2080;                 /* 2.08000 ms time each eye is on*/

    int32_t w = NVSTUSB_T2_COUNT(4568.50);      /* 4.56800 ms */
    int32_t x = NVSTUSB_T0_COUNT(phase_offset);      /* 4.77425 ms */
    int32_t y = NVSTUSB_T0_COUNT(activeTime);
    int32_t z = NVSTUSB_T2_COUNT(frameTime);

    uint8_t cmdTimings[] = {
    NVSTUSB_CMD_WRITE,      /* write data */
    0x00,                   /* to address 0x2007 (0x2007+0x00) = ?? */
    0x18, 0x00,             /* 24 bytes follow */

    /* original: e1 29 ff ff (-54815; -55835) */
    w, w>>8, w>>16, w>>24,    /* 2007: ?? some timer 2 counter, 1020 is subtracted from this
                               *       loaded at startup with:
                               *       0x44 0xEC 0xFE 0xFF (-70588(-1020)) */
    /* original: 68 b5 ff ff (-19096), 4.774 ms */
    x, x>>8, x>>16, x>>24,    /* 200b: ?? counter saved at long at address 0x4f
                               *       increased at timer 0 interrupt if bit 20h.1
                               *       is cleared, on overflow
                               *       to 0 the code at 0x03c8 is executed.
                               *       timer 0 will be started with this value
                               *       by timer2 */

    /* original: 81 df ff ff (-8319), 2.08 ms */
    y, y>>8, y>>16, y>>24,    /* 200f: ?? counter saved at long at address 0x4f, 784 is added to this
                               *       if PD1 is set, delay until turning eye off? */

    /* wave forms to send via IR: */
    0x30,                     /* 2013: 110000 PD1=0, PD2=0: left eye off  */
    0x28,                     /* 2014: 101000 PD1=1, PD2=0: left eye on   */
    0x24,                     /* 2015: 100100 PD1=0, PD2=1: right eye off */
    0x22,                     /* 2016: 100010 PD1=1, PD2=1: right eye on  */

    /* ?? used when frameState is != 2, for toggling bits in Port B,
     * values seem to have no influence on the glasses or infrared signals */
    0x0a,                     /* 2017: 1010 */
    0x08,                     /* 2018: 1000 */
    0x05,                     /* 2019: 0101 */
    0x04,                     /* 201a: 0100 */

    z, z>>8, z>>16, z>>24     /* 201b: timer 2 reload value */
    };
    nvstusb_usb_write_bulk(ctx->device, 2, cmdTimings, sizeof(cmdTimings));

    uint8_t cmd0x1c[] = {
    NVSTUSB_CMD_WRITE,      /* write data */
    0x1c,                   /* to address 0x2023 (0x2007+0x1c) = ?? */
    0x02, 0x00,             /* 2 bytes follow */

    0x02, 0x00              /* ?? seems to be the start value of some
                               counter. runs up to 6, some things happen
                               when it is lower, that will stop if when
                               it reaches 6. could be the index to 6 byte values
                               at 0x17ce that are loaded into TH0*/
    };
    nvstusb_usb_write_bulk(ctx->device, 2, cmd0x1c, sizeof(cmd0x1c));

    /* wait at most 2 seconds before going into idle */
    uint16_t timeout = rate * 4;

    uint8_t cmdTimeout[] = {
    NVSTUSB_CMD_WRITE,      /* write data */
    0x1e,                   /* to address 0x2025 (0x2007+0x1e) = timeout */
    0x02, 0x00,             /* 2 bytes follow */

    timeout, timeout>>8     /* idle timeout (number of frames) */
    };
    nvstusb_usb_write_bulk(ctx->device, 2, cmdTimeout, sizeof(cmdTimeout));

    uint8_t cmd0x1b[] = {
    NVSTUSB_CMD_WRITE,      /* write data */
    0x1b,                   /* to address 0x2022 (0x2007+0x1b) = ?? */
    0x01, 0x00,             /* 1 byte follows */

    0x07                    /* ?? compared with byte at 0x29 in TD_Poll()
                               bit 0-1: index to a table of 4 bytes at 0x17d4 (0x00,0x08,0x04,0x0C),
                               PB1 is set in TD_Poll() if this index is 0, cleared otherwise
                               bit 2:   set bool21_4, start timer 1, enable ext. int. 5
                               bit 3:   PC1 is set to the inverted value of this bit in TD_Poll()
                               bit 4-5: index to a table of 4 bytes at 0x2a
                               bit 6:   restart t0 on some conditions in TD_Poll()
                             */
    };
    nvstusb_usb_write_bulk(ctx->device, 2, cmd0x1b, sizeof(cmd0x1b));

    ctx->rate = rate;
}

bool init_nvstusb(float phase_offset)
{
    // float phase_offset = 4774.25; // microseconds
    nvstusb_context* ctx = nvstusb_init(NULL);
    nvstusb_set_rate2(ctx, 120, phase_offset);
    nvstusb_deinit(ctx);
    return true;
}
