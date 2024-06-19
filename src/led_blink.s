.global _start
_start:

ldr x0, =0x3F200000 
ldr x3, =0x3F20001C 
ldr x5, =0x3f200028 

ldr w1, x0          
movz w2, #0b11100000000000, lsl #16                  
bic w1, w1, w2  
movz w2, #0b00100000000000, lsl #16      
orr w1, w1, w2
str w1, [x0]

loop:               
ldr w1, x3          
movz w2, #0x0
movk w2, #0x4000, lsl #16    
orr w1, w1, w2
str w1, [x3]

movz x4, #0xFFFF
movk x4, #0xFFFF, lsl #16     
delay_loop:
subs x4, x4, #1         
bne delay_loop 

ldr w1, x5          
movz w2, #0x0 
movk w2, #0x4000, lsl #16       
orr w1, w1, w2
str w1, [x5]

movz x4, #0xFFFF  
movk x4, #0xFFFF, lsl #16    
delay_loop2:
subs x4, x4, #1         
bne delay_loop2

b loop
