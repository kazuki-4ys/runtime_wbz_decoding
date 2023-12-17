ENTRY(__entry)
SECTIONS
{
        .text 0x80004000 : {
                *(.text.__entry);
                *(.text*);
                *(.text.__end);
        }
}
