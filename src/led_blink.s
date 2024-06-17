#The program should first set the GPIO pin to output
#In a loop:
    #Set the pin to output a high signal via the GPSET registers.
    #Wait for a fixed amount of time.
    #Set the pin to output a low signal via the GPCLR clear registers.
    #Wait again for some time.

ldr x0, =0x3F200000 #gpio address
ldr x3, =0x3F20001C #gpset0 address
ldr x5, =0x3f200028 #gpclear0 address

ldr w1, x0          #section sets pin 9 to output
movz w2, #0b111, lsl #27       #clears bits 27-29             
bic w1, w1, w2  
movz w2, #0b001, lsl #27      #sets bits 27-29 to 001
orr w1, w1, w2
str w1, [x0]

loop:               #sets up a label to loop to
ldr w1, x3          #sets it to output a high signal
movz w2, #1, lsl #9    
orr w1, w1, w2
str w1, [x3]

movz x4, #1000000000 #makes it wait for a second      
delay_loop:
subs x4, x4, #1         
bne delay_loop 

ldr w1, x5          #sets it to output a low signal
movz w2, #1, lsl #9       
orr w1, w1, w2
str w1, [x5]

movz x4, #1000000000 #makes it wait for a second      
delay_loop2:
subs x4, x4, #1         
bne delay_loop2

b loop

and x0, x0, x0
