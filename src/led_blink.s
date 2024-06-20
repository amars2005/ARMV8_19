movz x0, #0x3f20, lsl #16
movz w1, #0x8000
str w1, [x0]

add x3, x0, #0x001c
add x5, x0, #0x0028 

movz w1, #0x20

movz w6, #0

high:
str w1, [x3]
movz w6, #1
b set_delay

low:
str w1, [x5]
movz w6, #0

set_delay:
movz w4, #0x50, lsl #16

delay_loop:
subs w4, w4, #1         
b.ne delay_loop 

cmp w6, #0
b.ne low
b high
