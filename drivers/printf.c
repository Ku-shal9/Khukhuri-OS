#include "printf.h"
#include "screen.h"

static void print_uint(unsigned int value, unsigned int base, int upper) {
    char buf[32];
    int i = 0;
    const char* digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (value == 0) {
        putchar('0');
        return;
    }
    while (value > 0 && i < 31) {
        buf[i++] = digits[value % base];
        value /= base;
    }
    while (i > 0) {
        putchar(buf[--i]);
    }
}

static void print_int(int value, int base, int upper) {
    if (value < 0 && base == 10) {
        putchar('-');
        print_uint(0U - (unsigned int)value, (unsigned int)base, upper);
    } else {
        print_uint((unsigned int)value, (unsigned int)base, upper);
    }
}

void vprintf(const char* fmt, va_list ap) {
    for (int i = 0; fmt[i] != '\0'; i++) {
        if (fmt[i] != '%' || fmt[i + 1] == '\0') {
            putchar(fmt[i]);
            continue;
        }
        i++;
        switch (fmt[i]) {
            case '%':
                putchar('%');
                break;
            case 'c':
                putchar((char)va_arg(ap, int));
                break;
            case 's': {
                const char* s = va_arg(ap, const char*);
                if (s == 0) {
                    print("(null)");
                } else {
                    print(s);
                }
                break;
            }
            case 'd':
            case 'i':
                print_int(va_arg(ap, int), 10, 0);
                break;
            case 'u':
                print_uint(va_arg(ap, unsigned int), 10, 0);
                break;
            case 'x':
                print_uint(va_arg(ap, unsigned int), 16, 0);
                break;
            case 'X':
                print_uint(va_arg(ap, unsigned int), 16, 1);
                break;
            case 'p': {
                unsigned int p = (unsigned int)va_arg(ap, void*);
                print("0x");
                print_uint(p, 16, 0);
                break;
            }
            default:
                putchar('%');
                putchar(fmt[i]);
                break;
        }
    }
}

void printf(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}
