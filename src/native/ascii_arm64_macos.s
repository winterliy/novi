.text
.p2align 2
.globl _novi_ascii_prefix
_novi_ascii_prefix:
    mov x2, #0
1:
    cmp x2, x1
    b.hs 2f
    ldrb w3, [x0, x2]
    tbnz w3, #7, 2f
    add x2, x2, #1
    b 1b
2:
    mov x0, x2
    ret
