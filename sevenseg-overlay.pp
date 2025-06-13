# 0 "sevenseg-overlay.dts"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "sevenseg-overlay.dts"
/dts-v1/;
/plugin/;

# 1 "/usr/src/linux-headers-6.12.25+rpt-common-rpi/include/dt-bindings/gpio/gpio.h" 1
# 5 "sevenseg-overlay.dts" 2

/ {
    compatible = "brcm,bcm2835";

    fragment@0 {
        target-path = "/";

        __overlay__ {
            sevenseg: sevenseg@0 {

                seg-gpios = <
                    &gpio 5 0
                    &gpio 6 0
                    &gpio 4 0
                    &gpio 17 0
                    &gpio 27 0
                    &gpio 22 0
                    &gpio 10 0
                    &gpio 9 0
                >;
            };
        };
    };
};
