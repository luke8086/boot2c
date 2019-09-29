SECTIONS
{    
    .text : 
    {
        *(.start);
        *(.text);
    }
    .data :
    {
        *(.data);
        *(.rodata*);
        *(.bss);
    }    
    /DISCARD/ :
    {
        *(.comment);
    }
}
