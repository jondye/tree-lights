from blinkstick import blinkstick
from time import sleep
from itertools import cycle

#               F R O N T                                         B A C K
#
#                . .                                                              . .
#             .       . .                                                    . .       .
#       . . .              . . . . . .                          . . . . . .         8    . . .
#     .         7                      .                      .                                .
#    .                  21               .                  .                             6     .
#    .                           19      .                  .                                   .
#  .                                     .                  .     18                              .
# .   4       9                    17   .                    .                                  5  .
# .                        20            . . .          . . .                 10        3          .
# .                                            .      .               16                           .
# .                                     15     .      .                                            .
# .             11         14                   .    .                                             .
# .    2                                         .  .                                              .
# .          1                   12        13    .  .                                              .
# .                                              .  .                                              .
# .                     0                        .  .                                              .
# .    ....     .           . . . . . . . . .   .    .   . . . . . . . . .           .     ....    .
# .   .    .    . . .     .                   .        .                   .     . . .    .    .   .
# .   .    .    .     . .                                                    . .     .    .    .   .
# .   .    .    .                                                                    .    .    .   .

colours = [
    (255, 0, 0),
    (0, 255, 0),
    (0, 0, 255),
    (255, 255, 0),
    (255, 0, 255),
    (0, 255, 255),
    (255, 255, 255),
]


def en(leds):
    bstick = blinkstick.BlinkStickPro(r_led_count=leds, max_rgb_value=255)
    bstick.connect()
    try:
        while True:
            led = input('which led? ')
            bstick.off()
            bstick.set_color(channel=0, index=led, r=255, g=255, b=255)
            bstick.send_data_all()
    finally:
        bstick.off()


def test(leds):
    bstick = blinkstick.BlinkStickPro(r_led_count=leds, max_rgb_value=128)
    bstick.connect()
    colour = cycle(colours)
    for i in range(leds):
        r, g, b = next(colour)
        bstick.set_color(channel=0, index=i, r=r, g=g, b=b)
    try:
        bstick.send_data_all()
        sleep(30)
    finally:
        bstick.off()
